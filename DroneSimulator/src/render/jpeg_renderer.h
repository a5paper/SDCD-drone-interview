#ifndef DRONE_SIMULATOR_RENDER_JPEG_RENDERER_H_
#define DRONE_SIMULATOR_RENDER_JPEG_RENDERER_H_

#include <cstdint>
#include <vector>

#include <opencv2/core.hpp>

#include "render/renderer.h"
#include "sim/world.h"

namespace drone_simulator::render {

class JpegRenderer {
 public:
  // Renders and JPEG-encodes the frame. GetJpeg() is empty if encoding failed.
  const cv::Mat& Render(const world::WorldState& state);
  const std::vector<uint8_t>& GetJpeg() const;

 private:
  Renderer renderer_;
  std::vector<uint8_t> jpeg_;
};

}

#endif
