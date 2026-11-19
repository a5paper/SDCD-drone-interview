#include <cstdint>
#include <cstdlib>
#include <format>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "config/config.h"
#include "config/network_config.h"
#include "gps/simulated_gps_receiver.h"
#include "network/command_receiver.h"
#include "network/message_sender.h"
#include "network/video_streamer.h"
#include "protocols/telemetry.pb.h"
#include "render/jpeg_renderer.h"
#include "sim/simulator.h"
#include "timing/loop_clock.h"

namespace drone_simulator {
namespace {

bool ReportOpen(bool opened, std::string_view socket_name) {
  if (!opened) {
    // Read the error before any output: console writes can reset it.
    const int error = network::LastSocketError();
    std::cerr << "Failed to open " << socket_name
              << " socket: " << network::ErrorText(error) << std::endl;
  }
  return opened;
}

drone_protocol::Telemetry MakeTelemetry(const Simulator& simulator,
                                        uint64_t sequence, uint64_t frame_id) {
  const drone::DroneState& drone_state = simulator.GetDrone().GetState();

  drone_protocol::Telemetry telemetry;
  telemetry.set_sequence(sequence);
  telemetry.set_timestamp_s(simulator.GetWorld().GetState().elapsed_time_s);
  telemetry.set_frame_id(frame_id);
  telemetry.set_speed_mps(drone_state.speed_mps);
  telemetry.set_battery_percentage(drone_state.battery_percentage);
  *telemetry.mutable_last_command() = drone_state.last_command;
  drone_protocol::GpsStatus& gps_status = *telemetry.mutable_gps_status();
  if (drone_state.gps_fix) {
    gps_status.mutable_fix()->set_latitude_deg(
        drone_state.gps_fix->latitude_deg);
    gps_status.mutable_fix()->set_longitude_deg(
        drone_state.gps_fix->longitude_deg);
  } else {
    gps_status.mutable_no_fix();
  }
  return telemetry;
}

void PrintStatus(const Simulator& simulator,
                 const render::JpegRenderer& renderer,
                 const network::VideoStreamer& streamer,
                 const network::MessageSender& telemetry_sender,
                 timing::LoopClock::Clock::duration uptime) {
  const world::WorldState& world_state = simulator.GetWorld().GetState();
  const double jpeg_kib = renderer.GetJpeg().size() / config::kBytesPerKiB;

  std::cout << "Position X: " << world_state.drone_position_x << " m\n"
            << "Position Y: " << world_state.drone_position_y << " m\n"
            << "Battery: " << simulator.GetDrone().GetState().battery_percentage
            << " %\n"
            << "JPEG: " << jpeg_kib << " KiB (" << jpeg_kib * config::kVideoFps
            << " KiB/s)\n"
            << "Video send failures: " << streamer.GetFailedSends() << "\n"
            << "Telemetry send failures: " << telemetry_sender.GetFailedSends()
            << "\n"
            << "Time On: " << std::format("{:%T}", uptime) << "\n\n"
            << std::flush;
}

void ShowPreview(const std::vector<uint8_t>& jpeg, bool enabled) {
  if (!enabled || jpeg.empty()) {
    return;
  }

  cv::imshow("Drone camera", cv::imdecode(jpeg, cv::IMREAD_COLOR));
  cv::waitKey(config::kPreviewWaitMs);
}

int Run(const config::NetworkConfig& settings) {
  network::CommandReceiver receiver;
  if (!ReportOpen(receiver.Open(settings.command_port), "command")) {
    return EXIT_FAILURE;
  }
  std::cout << "Listening for commands on UDP " << settings.command_port
            << std::endl;

  std::unique_ptr<network::FecStrategy> fec =
      network::MakeFecStrategy(settings.fec_scheme, settings.fec_group_size);
  network::VideoStreamer streamer(std::move(fec));
  const bool video_opened =
      streamer.Open(settings.gcs_address, settings.video_port);
  if (!ReportOpen(video_opened, "video")) {
    return EXIT_FAILURE;
  }
  std::cout << "Streaming video to " << settings.gcs_address << ":"
            << settings.video_port << std::endl;

  network::MessageSender telemetry_sender;
  const bool telemetry_opened =
      telemetry_sender.Open(settings.gcs_address, settings.telemetry_port);
  if (!ReportOpen(telemetry_opened, "telemetry")) {
    return EXIT_FAILURE;
  }
  std::cout << "Sending telemetry to " << settings.gcs_address << ":"
            << settings.telemetry_port << std::endl;

  world::World world;
  gps::SimulatedGpsReceiver gps_receiver(world);
  drone::Drone drone(gps_receiver);
  if (settings.command_timeout_s) {
    drone.SetCommandTimeout(*settings.command_timeout_s);
  }
  Simulator simulator(world, drone);
  render::JpegRenderer renderer;
  uint64_t next_frame_id = 0;
  uint64_t next_telemetry_sequence = 0;

  std::cout << std::fixed << std::setprecision(config::kStatusPrintPrecision);
  std::cout << "Drone Simulator starting..." << std::endl;

  timing::LoopClock clock(config::kTickPeriodS);
  timing::IntervalTimer status_timer(config::kStatusPrintIntervalS);

  for (int64_t tick = 0;; ++tick) {
    const double delta_time_s = clock.Tick();

    if (const auto command = receiver.Poll()) {
      simulator.HandleCommand(*command);
    }

    simulator.Update(delta_time_s);
    const world::WorldState& world_state = simulator.GetWorld().GetState();

    if (tick % config::kTicksPerFrame == 0) {
      renderer.Render(world_state);
      const std::vector<uint8_t>& jpeg = renderer.GetJpeg();
      streamer.Send(jpeg, next_frame_id++, world_state.elapsed_time_s);
      ShowPreview(jpeg, settings.preview);
    }

    if (tick % config::kTicksPerTelemetry == 0) {
      const drone_protocol::Telemetry telemetry = MakeTelemetry(
          simulator, next_telemetry_sequence++, next_frame_id - 1);
      telemetry_sender.Send(telemetry);
    }

    if (status_timer.Elapsed(delta_time_s)) {
      const timing::LoopClock::Clock::duration uptime = clock.GetUptime();
      PrintStatus(simulator, renderer, streamer, telemetry_sender, uptime);
    }

    clock.SleepUntilNextTick();
  }
}

}
}

int main(int argc, char** argv) {
  std::string config_path = DRONE_SIMULATOR_DEFAULT_CONFIG;
  if (argc == 3 && std::string_view(argv[1]) == "--config") {
    config_path = argv[2];
  } else if (argc != 1) {
    std::cerr << "Usage: " << argv[0] << " [--config PATH]" << std::endl;
    return EXIT_FAILURE;
  }

  const std::optional<drone_simulator::config::NetworkConfig> settings =
      drone_simulator::config::LoadNetworkConfig(config_path);
  if (!settings) {
    return EXIT_FAILURE;
  }
  return drone_simulator::Run(*settings);
}
