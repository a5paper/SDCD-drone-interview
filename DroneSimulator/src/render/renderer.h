#ifndef DRONE_SIMULATOR_RENDER_RENDERER_H_
#define DRONE_SIMULATOR_RENDER_RENDERER_H_

#include <opencv2/core.hpp>

#include "core/types.h"
#include "sim/world.h"

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

        void DrawWorldRect_(f64 forward_min, f64 forward_max, f64 right_min, f64 right_max,
                            const cv::Scalar &color, const world::WorldState &state);

        cv::Mat frame_;
    };

}

#endif
