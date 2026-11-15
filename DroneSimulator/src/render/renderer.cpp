#include "render/renderer.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <random>

#include <opencv2/imgproc.hpp>

#include "core/config.h"

namespace drone_simulator::render {

Renderer::Renderer()
    : frame_(config::kFrameHeightPx, config::kFrameWidthPx, CV_8UC3) {}

const cv::Mat& Renderer::Render(const world::WorldState& state) {
  const cv::Scalar grass_color(40, 90, 40);
  const cv::Scalar road_color(110, 110, 110);
  const cv::Scalar drone_color(0, 200, 255);
  const cv::Scalar drone_center_color(0, 0, 0);
  const uint8_t building_min_shade = 150;
  const uint8_t building_max_shade = 210;
  // Red, blue, white, black.
  const std::array<cv::Scalar, config::kCarColorCount> car_colors = {
      cv::Scalar(40, 40, 220), cv::Scalar(220, 90, 30),
      cv::Scalar(245, 245, 245), cv::Scalar(20, 20, 20)};

  const double x = state.drone_position_x;
  const double y = state.drone_position_y;

  const double half_forward_m = frame_.rows / 2.0 / config::kPixelsPerMeter;
  const double half_right_m = frame_.cols / 2.0 / config::kPixelsPerMeter;
  const double forward_min = x - half_forward_m;
  const double forward_max = x + half_forward_m;
  const double right_min = y - half_right_m;
  const double right_max = y + half_right_m;

  frame_.setTo(grass_color);

  constexpr double kBlock = config::kBlockSizeM;
  constexpr double kHalfRoad = config::kRoadWidthM / 2.0;
  constexpr double kLotMin = kHalfRoad + config::kBuildingMarginM;
  constexpr double kLotSize = kBlock - 2 * kLotMin;

  // Each block's buildings come from a generator seeded by the block's grid
  // position, so they are the same on every frame without being stored.
  for (int f = static_cast<int>(std::floor(forward_min / kBlock));
       f * kBlock <= forward_max; ++f) {
    for (int r = static_cast<int>(std::floor(right_min / kBlock));
         r * kBlock <= right_max; ++r) {
      std::seed_seq seed{static_cast<uint32_t>(f), static_cast<uint32_t>(r),
                         config::kRandomSeed};
      std::minstd_rand random(seed);
      std::uniform_real_distribution<double> size(config::kBuildingMinSizeM,
                                                  config::kBuildingMaxSizeM);
      std::uniform_real_distribution<double> place(0.0, 1.0);
      std::uniform_int_distribution<int> shade(building_min_shade,
                                               building_max_shade);
      for (int i = 0; i < config::kBuildingsPerBlock; ++i) {
        const double size_f = size(random);
        const double size_r = size(random);
        const double min_f =
            f * kBlock + kLotMin + place(random) * (kLotSize - size_f);
        const double min_r =
            r * kBlock + kLotMin + place(random) * (kLotSize - size_r);
        const auto gray = static_cast<uint8_t>(shade(random));
        DrawWorldRect(min_f, min_f + size_f, min_r, min_r + size_r,
                      cv::Scalar(gray, gray, gray), state);
      }
    }
  }

  for (double k = std::floor(right_min / kBlock);
       k <= std::ceil(right_max / kBlock); ++k) {
    DrawWorldRect(forward_min, forward_max, k * kBlock - kHalfRoad,
                  k * kBlock + kHalfRoad, road_color, state);
  }

  for (double k = std::floor(forward_min / kBlock);
       k <= std::ceil(forward_max / kBlock); ++k) {
    DrawWorldRect(k * kBlock - kHalfRoad, k * kBlock + kHalfRoad, right_min,
                  right_max, road_color, state);
  }

  for (const world::Car& car : state.cars) {
    const bool along_x = car.velocity_x != 0.0;
    const double half_f =
        (along_x ? config::kCarLengthM : config::kCarWidthM) / 2.0;
    const double half_r =
        (along_x ? config::kCarWidthM : config::kCarLengthM) / 2.0;
    DrawWorldRect(car.position_x - half_f, car.position_x + half_f,
                  car.position_y - half_r, car.position_y + half_r,
                  car_colors[car.color_index], state);
  }

  const double radius = config::kDroneSizeM / 2.0;
  DrawWorldRect(x - radius, x + radius, y - radius, y + radius, drone_color,
                state);
  cv::circle(frame_, WorldToScreen(x, y, state), config::kDroneCenterRadiusPx,
             drone_center_color, cv::FILLED);

  return frame_;
}

cv::Point2d Renderer::WorldToScreen(double world_x, double world_y,
                                    const world::WorldState& state) const {
  const double screen_x =
      frame_.cols / 2.0 +
      (world_y - state.drone_position_y) * config::kPixelsPerMeter;
  const double screen_y =
      frame_.rows / 2.0 -
      (world_x - state.drone_position_x) * config::kPixelsPerMeter;

  return {screen_x, screen_y};
}

void Renderer::DrawWorldRect(double forward_min, double forward_max,
                             double right_min, double right_max,
                             const cv::Scalar& color,
                             const world::WorldState& state) {
  cv::rectangle(frame_, WorldToScreen(forward_max, right_min, state),
                WorldToScreen(forward_min, right_max, state), color,
                cv::FILLED);
}

}
