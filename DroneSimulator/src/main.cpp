#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "config/config.h"
#include "config/network_config.h"
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
    std::cerr << "Failed to open " << socket_name
              << " socket: " << std::strerror(errno) << std::endl;
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

void ShowPreview(const std::vector<uint8_t>& jpeg) {
  if (!config::kShowPreview || jpeg.empty()) {
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

  network::VideoStreamer streamer(
      network::MakeFecStrategy(settings.fec_scheme, settings.fec_group_size));
  if (!ReportOpen(streamer.Open(settings.gcs_address, settings.video_port),
                  "video")) {
    return EXIT_FAILURE;
  }
  std::cout << "Streaming video to " << settings.gcs_address << ":"
            << settings.video_port << std::endl;

  network::MessageSender telemetry_sender;
  if (!ReportOpen(
          telemetry_sender.Open(settings.gcs_address, settings.telemetry_port),
          "telemetry")) {
    return EXIT_FAILURE;
  }
  std::cout << "Sending telemetry to " << settings.gcs_address << ":"
            << settings.telemetry_port << std::endl;

  Simulator simulator;
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

    if (tick % config::kTicksPerFrame == 0) {
      renderer.Render(simulator.GetWorld().GetState());
      streamer.Send(renderer.GetJpeg(), next_frame_id++,
                    simulator.GetWorld().GetState().elapsed_time_s);
      ShowPreview(renderer.GetJpeg());
    }

    if (tick % config::kTicksPerTelemetry == 0) {
      telemetry_sender.Send(MakeTelemetry(simulator, next_telemetry_sequence++,
                                          next_frame_id - 1));
    }

    if (status_timer.Elapsed(delta_time_s)) {
      PrintStatus(simulator, renderer, streamer, telemetry_sender,
                  clock.GetUptime());
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
