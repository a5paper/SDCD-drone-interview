#ifndef DRONE_SIMULATOR_RENDER_RENDERER_H_
#define DRONE_SIMULATOR_RENDER_RENDERER_H_

#include <opencv2/core.hpp>

#include "sim/world.h"

namespace drone_simulator::render {

// Draws a top-down frame centred on the drone, one layer at a time: grass,
// buildings, roads, cars, drone. World X points up the frame, world Y right.
class Renderer {
 public:
  Renderer();

  const cv::Mat& Render(const world::WorldState& state);

 private:
  // The world area the frame shows, in metres.
  struct View {
    double forward_min = 0.0;
    double forward_max = 0.0;
    double right_min = 0.0;
    double right_max = 0.0;
  };

  void DrawBuildings();
  void DrawBlockBuildings(int block_forward, int block_right);
  void DrawRoads();
  void DrawCars(const world::CarPool& pool, bool along_x);
  void DrawDrone(double drone_x, double drone_y);

  cv::Point2d WorldToScreen(double world_x, double world_y) const;
  // Fills a world-space rectangle. Skips it when it is outside the view.
  void DrawWorldRect(double forward_min, double forward_max, double right_min,
                     double right_max, const cv::Scalar& color);

  cv::Mat frame_;
  View view_;
};

}

#endif
