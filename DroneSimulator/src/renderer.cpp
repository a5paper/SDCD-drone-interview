#include "renderer.h"

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
        frame_.setTo(kGrassColor);

        const f64 radius = config::kDroneSizeM / 2.0;
        const f64 x = state.drone_position_x;
        const f64 y = state.drone_position_y;

        // Top-left = forward-left of the drone, bottom-right = behind-right.
        cv::rectangle(frame_, WorldToScreen_(x + radius, y - radius, state),
                      WorldToScreen_(x - radius, y + radius, state), kDroneColor, cv::FILLED);
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

} // namespace drone_simulator::render
