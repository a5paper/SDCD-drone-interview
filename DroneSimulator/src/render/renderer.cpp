#include "render/renderer.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <random>

#include <opencv2/imgproc.hpp>

#include "config/config.h"
#include "sim/road_grid.h"

namespace drone_simulator::render {
namespace {

constexpr double kHalfRoad = config::kRoadWidthM / 2.0;

}

Renderer::Renderer()
    : frame_(config::kFrameHeightPx, config::kFrameWidthPx, CV_8UC3) {}

const cv::Mat& Renderer::Render(const world::WorldState& state) {
  const cv::Scalar grass_color(40, 90, 40);

  const double x = state.drone_position_x;
  const double y = state.drone_position_y;
  const double half_forward = frame_.rows / 2.0 / config::kPixelsPerMeter;
  const double half_right = frame_.cols / 2.0 / config::kPixelsPerMeter;
  view_ = {x - half_forward, x + half_forward, y - half_right, y + half_right};

  frame_.setTo(grass_color);
  DrawBuildings();
  DrawRoads();
  DrawCars(state.cars_along_x, /*along_x=*/true);
  DrawCars(state.cars_along_y, /*along_x=*/false);
  DrawDrone(x, y);
  return frame_;
}

void Renderer::DrawBuildings() {
  for (int f = world::RoadAtOrBelow(view_.forward_min);
       world::RoadCenter(f) <= view_.forward_max; ++f) {
    for (int r = world::RoadAtOrBelow(view_.right_min);
         world::RoadCenter(r) <= view_.right_max; ++r) {
      DrawBlockBuildings(f, r);
    }
  }
}

// A block's buildings come from a generator seeded by the block's position,
// so they are the same on every frame without being stored.
void Renderer::DrawBlockBuildings(int block_forward, int block_right) {
  const uint8_t min_shade = 150;
  const uint8_t max_shade = 210;
  constexpr double kLotMin = kHalfRoad + config::kBuildingMarginM;
  constexpr double kLotSize = config::kBlockSizeM - 2 * kLotMin;

  std::seed_seq seed{static_cast<uint32_t>(block_forward),
                     static_cast<uint32_t>(block_right), config::kRandomSeed};
  std::minstd_rand random(seed);
  std::uniform_real_distribution<double> size(config::kBuildingMinSizeM,
                                              config::kBuildingMaxSizeM);
  std::uniform_real_distribution<double> place(0.0, 1.0);
  std::uniform_int_distribution<int> shade(min_shade, max_shade);

  for (int i = 0; i < config::kBuildingsPerBlock; ++i) {
    const double size_forward = size(random);
    const double size_right = size(random);
    const double forward = world::RoadCenter(block_forward) + kLotMin +
                           place(random) * (kLotSize - size_forward);
    const double right = world::RoadCenter(block_right) + kLotMin +
                         place(random) * (kLotSize - size_right);
    const auto gray = static_cast<uint8_t>(shade(random));
    const cv::Scalar color(gray, gray, gray);
    DrawWorldRect(forward, forward + size_forward, right, right + size_right,
                  color);
  }
}

void Renderer::DrawRoads() {
  const cv::Scalar road_color(110, 110, 110);

  for (int k = world::RoadAtOrBelow(view_.right_min);
       world::RoadCenter(k) - kHalfRoad <= view_.right_max; ++k) {
    const double center = world::RoadCenter(k);
    DrawWorldRect(view_.forward_min, view_.forward_max, center - kHalfRoad,
                  center + kHalfRoad, road_color);
  }
  for (int k = world::RoadAtOrBelow(view_.forward_min);
       world::RoadCenter(k) - kHalfRoad <= view_.forward_max; ++k) {
    const double center = world::RoadCenter(k);
    DrawWorldRect(center - kHalfRoad, center + kHalfRoad, view_.right_min,
                  view_.right_max, road_color);
  }
}

// `along_x` tells which way the pool's roads run, so which car side is long.
void Renderer::DrawCars(const world::CarPool& pool, bool along_x) {
  // Red, blue, white, black.
  const std::array<cv::Scalar, config::kCarColorCount> colors = {
      cv::Scalar(40, 40, 220), cv::Scalar(220, 90, 30),
      cv::Scalar(245, 245, 245), cv::Scalar(20, 20, 20)};
  const double half_length = config::kCarLengthM / 2.0;
  const double half_width = config::kCarWidthM / 2.0;
  const double half_forward = along_x ? half_length : half_width;
  const double half_right = along_x ? half_width : half_length;

  for (size_t i = 0; i < pool.along.size(); ++i) {
    const double forward = along_x ? pool.along[i] : pool.across[i];
    const double right = along_x ? pool.across[i] : pool.along[i];
    DrawWorldRect(forward - half_forward, forward + half_forward,
                  right - half_right, right + half_right,
                  colors[pool.color[i]]);
  }
}

void Renderer::DrawDrone(double drone_x, double drone_y) {
  const cv::Scalar body_color(0, 200, 255);
  const cv::Scalar center_color(0, 0, 0);
  const double radius = config::kDroneSizeM / 2.0;

  DrawWorldRect(drone_x - radius, drone_x + radius, drone_y - radius,
                drone_y + radius, body_color);
  const cv::Point2d center = WorldToScreen(drone_x, drone_y);
  cv::circle(frame_, center, config::kDroneCenterRadiusPx, center_color,
             cv::FILLED);
}

cv::Point2d Renderer::WorldToScreen(double world_x, double world_y) const {
  return {(world_y - view_.right_min) * config::kPixelsPerMeter,
          (view_.forward_max - world_x) * config::kPixelsPerMeter};
}

void Renderer::DrawWorldRect(double forward_min, double forward_max,
                             double right_min, double right_max,
                             const cv::Scalar& color) {
  if (forward_max < view_.forward_min || forward_min > view_.forward_max ||
      right_max < view_.right_min || right_min > view_.right_max) {
    return;
  }
  const cv::Point2d top_left = WorldToScreen(forward_max, right_min);
  const cv::Point2d bottom_right = WorldToScreen(forward_min, right_max);
  cv::rectangle(frame_, top_left, bottom_right, color, cv::FILLED);
}

}
