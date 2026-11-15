#ifndef DRONE_SIMULATOR_INCLUDE_JPEG_RENDERER_H_
#define DRONE_SIMULATOR_INCLUDE_JPEG_RENDERER_H_

#include <vector>

#include <opencv2/core.hpp>

#include "renderer.h"
#include "types.h"
#include "world.h"

namespace drone_simulator::render
{
    class JpegRenderer
    {
    public:
        // Renders and JPEG-encodes the frame. GetJpeg() is empty if encoding failed.
        const cv::Mat &Render(const world::WorldState &state);
        const std::vector<u8> &GetJpeg() const;

    private:
        Renderer renderer_;
        std::vector<u8> jpeg_;
    };

}

#endif
