#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include "core/config.h"
#include "network/command_receiver.h"
#include "network/video_streamer.h"
#include "render/jpeg_renderer.h"
#include "sim/simulator.h"

namespace drone_simulator {
namespace {

using Clock = std::chrono::steady_clock;
using Seconds = std::chrono::duration<double>;

void PrintStatus(const Simulator& simulator,
                 const render::JpegRenderer& renderer,
                 const network::VideoStreamer& streamer,
                 Clock::duration uptime) {
  const world::WorldState& world = simulator.GetWorld().GetState();
  const double jpeg_kib = renderer.GetJpeg().size() / config::kBytesPerKiB;

  std::cout << "Position X: " << world.drone_position_x << " m\n"
            << "Position Y: " << world.drone_position_y << " m\n"
            << "Battery: " << simulator.GetDrone().GetState().battery_percentage
            << " %\n"
            << "JPEG: " << jpeg_kib << " KiB (" << jpeg_kib * config::kVideoFps
            << " KiB/s)\n"
            << "Video send failures: " << streamer.GetFailedSends() << "\n"
            << "Time On: " << std::format("{:%T}", uptime) << "\n\n";
}

void ShowPreview(const std::vector<uint8_t>& jpeg) {
  if (!config::kShowPreview || jpeg.empty()) {
    return;
  }

  cv::imshow("Drone camera", cv::imdecode(jpeg, cv::IMREAD_COLOR));
  cv::waitKey(config::kPreviewWaitMs);
}

}

int Run() {
  network::CommandReceiver receiver;
  if (!receiver.Open(config::kCommandPort)) {
    std::cerr << "Failed to open command socket: " << std::strerror(errno)
              << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Listening for commands on UDP " << config::kCommandPort
            << std::endl;

  network::VideoStreamer streamer;
  if (!streamer.Open(config::kGcsAddress, config::kVideoPort)) {
    std::cerr << "Failed to open video socket: " << std::strerror(errno)
              << std::endl;
    return EXIT_FAILURE;
  }
  std::cout << "Streaming video to " << config::kGcsAddress << ":"
            << config::kVideoPort << std::endl;

  Simulator simulator;
  render::JpegRenderer renderer;
  uint64_t frame_id = 0;

  std::cout << std::fixed << std::setprecision(config::kStatusPrintPrecision);
  std::cout << "Drone Simulator starting..." << std::endl;

  const auto tick_duration = std::chrono::duration_cast<Clock::duration>(
      Seconds(config::kTickPeriodS));
  const Clock::time_point start_time = Clock::now();
  Clock::time_point previous_time = start_time;
  double status_print_timer = 0.0;

  for (int64_t tick = 0;; ++tick) {
    const Clock::time_point current_time = Clock::now();
    const double delta_time = Seconds(current_time - previous_time).count();
    previous_time = current_time;

    if (const auto command = receiver.Poll()) {
      simulator.HandleCommand(*command);
    }

    simulator.Update(delta_time);

    if (tick % config::kTicksPerFrame == 0) {
      renderer.Render(simulator.GetWorld().GetState());
      streamer.Send(renderer.GetJpeg(), frame_id++);
      ShowPreview(renderer.GetJpeg());
    }

    status_print_timer += delta_time;
    if (status_print_timer >= config::kStatusPrintIntervalS) {
      status_print_timer = 0.0;
      PrintStatus(simulator, renderer, streamer, current_time - start_time);
    }

    std::this_thread::sleep_until(current_time + tick_duration);
  }
}

}

int main() { return drone_simulator::Run(); }
