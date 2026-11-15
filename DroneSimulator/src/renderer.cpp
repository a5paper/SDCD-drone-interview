#include "renderer.h"

#include <cmath>

#include <opencv2/imgproc.hpp>

#include "config.h"

namespace drone_simulator::render
{

    const cv::Scalar kGrassColor(40, 90, 40);
    const cv::Scalar kRoadColor(110, 110, 110);
    const cv::Scalar kBuildingColor(140, 140, 140);
    const cv::Scalar kDroneColor(0, 200, 255);
    const cv::Scalar kDroneCenterColor(0, 0, 0);

    Renderer::Renderer()
    {
        frame_ = cv::Mat(config::kFrameHeightPx, config::kFrameWidthPx, CV_8UC3);
    }

    const cv::Mat &Renderer::Render(const world::WorldState &state)
    {
        const f64 x = state.drone_position_x;
        const f64 y = state.drone_position_y;

        const f64 half_forward_m = frame_.rows / 2.0 / config::kPixelsPerMeter;
        const f64 half_right_m = frame_.cols / 2.0 / config::kPixelsPerMeter;
        const f64 forward_min = x - half_forward_m;
        const f64 forward_max = x + half_forward_m;
        const f64 right_min = y - half_right_m;
        const f64 right_max = y + half_right_m;

        frame_.setTo(kGrassColor);

        constexpr f64 kBlock = config::kBlockSizeM;
        constexpr f64 kHalfRoad = config::kRoadWidthM / 2.0;

        for (f64 k = std::floor(right_min / kBlock); k <= std::ceil(right_max / kBlock); ++k)
        {
            DrawWorldRect_(forward_min, forward_max, k * kBlock - kHalfRoad, k * kBlock + kHalfRoad,
                           kRoadColor, state);
        }

        for (f64 k = std::floor(forward_min / kBlock); k <= std::ceil(forward_max / kBlock); ++k)
        {
            DrawWorldRect_(k * kBlock - kHalfRoad, k * kBlock + kHalfRoad, right_min, right_max,
                           kRoadColor, state);
        }

        const f64 radius = config::kDroneSizeM / 2.0;
        DrawWorldRect_(x - radius, x + radius, y - radius, y + radius, kDroneColor, state);
        cv::circle(frame_, WorldToScreen_(x, y, state), 3, kDroneCenterColor, cv::FILLED);

        return frame_;
    }

    cv::Point2d Renderer::WorldToScreen_(f64 world_x, f64 world_y,
                                         const world::WorldState &state) const
    {
        auto screen_x =
            frame_.cols / 2 + (world_y - state.drone_position_y) * config::kPixelsPerMeter;
        auto screen_y =
            frame_.rows / 2 - (world_x - state.drone_position_x) * config::kPixelsPerMeter;

        return {screen_x, screen_y};
    }

    void Renderer::DrawWorldRect_(f64 forward_min, f64 forward_max, f64 right_min, f64 right_max,
                                  const cv::Scalar &color, const world::WorldState &state)
    {
        cv::rectangle(frame_, WorldToScreen_(forward_max, right_min, state),
                      WorldToScreen_(forward_min, right_max, state), color, cv::FILLED);
    }

} // namespace drone_simulator::render
