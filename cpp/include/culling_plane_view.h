#ifndef _HL_CULLING_PLANE_VIEW_H_
#define _HL_CULLING_PLANE_VIEW_H_

#include <mesh.h>
#include <model.h>
#include <iview.h>

#include <string>
#include <vector>
#include <eigen/dense>

namespace HexaLab {
    class CullingPlaneView : public IView {
    private:
        Model straight_model;
        Model hidden_model;
        Model singularity_model;
        Hyperplane<float, 3> plane;

    public:
        CullingPlaneView(Mesh& mesh);

        void set_plane_position(float x, float y, float z) {
            plane = Hyperplane<float, 3>(plane.normal(), Vector3f(x, y, z));
        }
        void set_plane_normal(float nx, float ny, float nz) {
            Vector3f normal(nx, ny, nz);
            normal.normalize();
            plane = Hyperplane<float, 3>(normal, mesh.aabb.center() + normal * (mesh.aabb.diagonal().norm() * (get_plane_offset() - 0.5)));
        }
        void set_plane_offset(float offset) { // offset in [0,1]
             plane = Hyperplane<float, 3>(plane.normal(), mesh.aabb.center() + plane.normal() * (mesh.aabb.diagonal().norm() * (offset - 0.5)));
        }
        Vector3f get_plane_position() {
            return mesh.aabb.center() + plane.normal() * (mesh.aabb.diagonal().norm() * (get_plane_offset() - 0.5));
        }
        Vector3f get_plane_normal() {
            return plane.normal();
        }
        float get_plane_offset() {   // return the offset from the center expressed in [0,1] range (0.5 is the center)
            return -plane.signedDistance(mesh.aabb.center()) / mesh.aabb.diagonal().norm() + 0.5;
        }
        float get_plane_world_offset() {
            return plane.signedDistance(mesh.aabb.center());
        }

        void update();

        Model* get_straight_model() { return &straight_model; }
        Model* get_hidden_model() { return &hidden_model; }
        Model* get_singularity_model() { return &singularity_model; }
    
    private:
        bool plane_cull_test(Face& face);
        // the following 4 should probably be part of the Model class api, but that would require to move marks inside the MeshNavigator, which kinda makes sense, but requires some work.
        void add_straight_face(Dart& dart, float normal_sign);
        void add_straight_wireframe(Dart& dart);
        void add_hidden_face(Dart& dart);
        void add_hidden_wireframe(Dart& dart);
    };
}

#endif