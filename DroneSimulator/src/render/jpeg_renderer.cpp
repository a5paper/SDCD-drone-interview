#include "render/jpeg_renderer.h"

#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "config/config.h"

namespace drone_simulator::render {

const cv::Mat& JpegRenderer::Render(const world::WorldState& state) {
  const cv::Mat& frame = renderer_.Render(state);
  if (!cv::imencode(".jpg", frame, jpeg_,
                    {cv::IMWRITE_JPEG_QUALITY, config::kJpegQuality})) {
    jpeg_.clear();
  }
  return frame;
}

const std::vector<uint8_t>& JpegRenderer::GetJpeg() const { return jpeg_; }

}
