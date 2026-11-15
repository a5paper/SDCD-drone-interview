#ifndef DRONE_SIMULATOR_INCLUDE_RENDERER_H_
#define DRONE_SIMULATOR_INCLUDE_RENDERER_H_

#include <opencv2/core.hpp>

#include "types.h"
#include "world.h"

namespace drone_simulator::render
{
    class Renderer
    {
    public:
        Renderer();
        const cv::Mat &Render(const world::WorldState &state);

    private:
        cv::Point2d WorldToScreen_(f64 world_x, f64 world_y,
                                 const world::WorldState &state) const;

        cv::Mat frame_;
    };

}

#endif
