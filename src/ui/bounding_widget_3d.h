#ifndef BOUNDING_WIDGET_3D_H
#define BOUNDING_WIDGET_3D_H

#include "state.h"
#include "volume_rendering_2.h"
#include "rendering_2d.h"

namespace igl { namespace opengl { namespace glfw { class Viewer; }}}

class Bounding_Widget_3d {

public:
    Bounding_Widget_3d(State& state);

    void initialize(igl::opengl::glfw::Viewer* viewer);
    bool pre_draw(float current_cut_index);
    bool post_draw(const glm::vec4& viewport, BoundingCage::KeyFrameIterator current_kf);

    vr::VolumeRenderer volume_renderer;
    Renderer2d renderer_2d;

private:
    void update_volume_geometry(const Eigen::MatrixXd& cage_V, const Eigen::MatrixXi& cage_F);
    void update_2d_geometry(BoundingCage::KeyFrameIterator current_kf);

    int cage_polyline_id;
    int current_kf_polyline_id;
    int skeleton_polyline_id;

    State& _state;
    igl::opengl::glfw::Viewer* _viewer;

    glm::vec4 _last_viewport;

    struct {
        GLuint framebuffer[2];
        GLuint texture[2];
    } _gl_state;
};

#endif // BOUNDING_WIDGET_3D_H