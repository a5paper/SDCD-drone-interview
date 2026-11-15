#include "render/jpeg_renderer.h"

#include <opencv2/imgcodecs.hpp>

#include "core/config.h"

namespace drone_simulator::render
{

    const cv::Mat &JpegRenderer::Render(const world::WorldState &state)
    {
        const cv::Mat &frame = renderer_.Render(state);
        if (!cv::imencode(".jpg", frame, jpeg_, {cv::IMWRITE_JPEG_QUALITY, config::kJpegQuality}))
        {
            jpeg_.clear();
        }
        return frame;
    }

    const std::vector<u8> &JpegRenderer::GetJpeg() const
    {
        return jpeg_;
    }

}
