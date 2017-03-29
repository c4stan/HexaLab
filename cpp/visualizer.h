#ifndef _HL_VISUALIZER_H_
#define _HL_VISUALIZER_H_

#include "common.h"
#include "mesh.h"
#include <eigen/dense>
#include <vector>

namespace HexaLab {
    class Visualizer {
        using Vert = Eigen::Vector3f;

        std::vector<Vert> vbuffer;
        std::vector<Index> ibuffer;
        Eigen::Hyperplane<float, 3> plane;
        Mesh* mesh = nullptr;

    public:
        void set_mesh(Mesh& mesh) { this->mesh = &mesh; }
        void set_culling_plane(const Eigen::Hyperplane<float, 3>& plane) { this->plane = plane; }
        void set_culling_plane(Vert normal, Vert position) { this->plane = Eigen::Hyperplane<float, 3>(normal, position); }
        void set_culling_plane(Vert normal, float d) { this->plane = Eigen::Hyperplane<float, 3>(normal, d); }
        void set_culling_plane(float nx, float ny, float nz, float d) { this->plane = Eigen::Hyperplane<float, 3>(Vert(nx, ny, nz), d); }

        void update_vbuffer();
        void update_ibuffer();
    
        js_ptr get_vbuffer() { return (js_ptr)this->vbuffer.data(); }
        int get_vbuffer_size() { return this->vbuffer.size(); }
        js_ptr get_ibuffer() { return (js_ptr)this->ibuffer.data(); }
        int get_ibuffer_size() { return this->ibuffer.size(); }
    };
}

#endif