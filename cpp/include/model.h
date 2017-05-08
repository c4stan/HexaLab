#ifndef _HL_MODEL_H_
#define _HL_MODEL_H_

#include <common.h>
#include <mesh.h>

#include <vector>
#include <eigen/dense>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;
    
    struct Model {
        vector<Vector3f> surface_vert_pos;
        vector<Vector3f> surface_vert_norm;
        vector<Vector3f> surface_vert_color;
        vector<Vector3f> wireframe_vert_pos;
        vector<Vector3f> wireframe_vert_color;
        bool dirty_flag = false;
        int qwe = 5;

        void clear() {
            surface_vert_pos.clear();
            surface_vert_norm.clear();
            surface_vert_color.clear();
            wireframe_vert_pos.clear();
            wireframe_vert_color.clear();
            dirty_flag = false;
        }

        uintptr_t get_surface_vert_pos_addr() { return (uintptr_t)surface_vert_pos.data(); }
        uint32_t get_surface_vert_pos_count() { return surface_vert_pos.size(); }
        uintptr_t get_surface_vert_norm_addr() { return (uintptr_t)surface_vert_norm.data(); }
        uint32_t get_surface_vert_norm_count() { return surface_vert_norm.size(); }
        uintptr_t get_surface_vert_color_addr() { return (uintptr_t)surface_vert_color.data(); }
        uint32_t get_surface_vert_color_count() { return surface_vert_color.size(); }

        uintptr_t get_wireframe_vert_pos_addr() { return (uintptr_t)wireframe_vert_pos.data(); }
        uint32_t get_wireframe_vert_pos_count() { return wireframe_vert_pos.size(); }
        uintptr_t get_wireframe_vert_color_addr() { return (uintptr_t)wireframe_vert_color.data(); }
        uint32_t get_wireframe_vert_color_count() { return wireframe_vert_color.size(); }
    };
}

#endif