#ifndef DRONE_SIMULATOR_RENDER_RENDERER_H_
#define DRONE_SIMULATOR_RENDER_RENDERER_H_

#include <opencv2/core.hpp>

#include "sim/world.h"

namespace drone_simulator::render {

class Renderer {
 public:
  Renderer();

  const cv::Mat& Render(const world::WorldState& state);

 private:
  cv::Point2d WorldToScreen(double world_x, double world_y,
                            const world::WorldState& state) const;

  void DrawWorldRect(double forward_min, double forward_max, double right_min,
                     double right_max, const cv::Scalar& color,
                     const world::WorldState& state);

  cv::Mat frame_;
};

}

#endif
