#include <cerrno>
#include <chrono>
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
#include "core/types.h"
#include "network/command_receiver.h"
#include "render/jpeg_renderer.h"
#include "sim/simulator.h"

namespace
{
    using namespace drone_simulator;
    using Clock = std::chrono::steady_clock;

    void PrintStatus(const Simulator &simulator, const render::JpegRenderer &renderer,
                     Clock::duration uptime)
    {
        const f64 jpeg_kib = renderer.GetJpeg().size() / config::kBytesPerKiB;

        std::cout << "Position X: " << simulator.world.state.drone_position_x << " m\n"
                  << "Position Y: " << simulator.world.state.drone_position_y << " m\n"
                  << "Battery: " << simulator.drone.GetState().battery_percentage << " %\n"
                  << "JPEG: " << jpeg_kib << " KiB (" << jpeg_kib * config::kVideoFps
                  << " KiB/s)\n"
                  << "Time On: " << std::format("{:%T}", uptime) << "\n\n";
    }

    void ShowPreview(const std::vector<u8> &jpeg)
    {
        if (!config::kShowPreview || jpeg.empty())
        {
            return;
        }

        cv::imshow("Drone camera", cv::imdecode(jpeg, cv::IMREAD_COLOR));
        cv::waitKey(config::kPreviewWaitMs);
    }
}

int main()
{
    using namespace std::chrono;

    network::CommandReceiver receiver;
    if (!receiver.Open(config::kCommandPort))
    {
        std::cerr << "Failed to open command socket: " << std::strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Listening for commands on UDP " << config::kCommandPort << std::endl;

    Simulator simulator;
    render::JpegRenderer renderer;

    std::cout << std::fixed << std::setprecision(config::kStatusPrintPrecision);
    std::cout << "Drone Simulator starting..." << std::endl;

    const auto tick_duration = duration_cast<Clock::duration>(duration<f64>(config::kTickPeriodS));
    const auto start_time = Clock::now();
    auto previous_time = start_time;
    f64 status_print_timer = 0.0;

    for (u64 tick = 0;; ++tick)
    {
        const auto current_time = Clock::now();
        const f64 delta_time = duration<f64>(current_time - previous_time).count();
        previous_time = current_time;

        if (const auto command = receiver.Poll())
        {
            simulator.drone.HandleCommand(*command);
        }

        simulator.Update(delta_time);

        if (tick % config::kTicksPerFrame == 0)
        {
            renderer.Render(simulator.world.state);
            ShowPreview(renderer.GetJpeg());
        }

        status_print_timer += delta_time;
        if (status_print_timer >= config::kStatusPrintIntervalS)
        {
            status_print_timer = 0.0;
            PrintStatus(simulator, renderer, current_time - start_time);
        }

        std::this_thread::sleep_until(current_time + tick_duration);
    }
}
