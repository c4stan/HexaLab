#include <culling_plane_view.h>

namespace HexaLab {
    void CullingPlaneView::set_mesh(js_ptr mesh_ptr) {
        this->mesh = (Mesh*)mesh_ptr;

        singularity_model.clear();
        for (size_t i = 0; i < mesh->edges.size(); ++i) {
            MeshNavigator nav = mesh->navigate(mesh->edges[i]);
            if (nav.edge().face_count == 4) continue;
            if (nav.edge().surface) continue;
            for (int j = 0; j < 2; ++j) {
                singularity_model.wireframe_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                nav = nav.flip_vert();
            }
            int d = (4 - nav.edge().face_count);
            Vector3f color;
            switch (d) {
            case -1:
                color = Vector3f(1, 0, 0);
                break;
            case 1:
                color = Vector3f(0, 1, 0);
                break;
            default:
                color = Vector3f(0, 0, 1);
            }
            singularity_model.wireframe_vert_color.push_back(color);
            singularity_model.wireframe_vert_color.push_back(color);
        }
    }

    bool CullingPlaneView::plane_cull_test(Face& face) {
        MeshNavigator nav = mesh->navigate(face);
        for (int v = 0; v < 4; ++v) {
            if (plane.signedDistance(nav.vert().position) < 0) {
                return true;
                break;
            }
            nav = nav.rotate_on_face();
        }
        return false;
    }

    void CullingPlaneView::add_straight_face(Dart& dart, float normal_sign) {
        MeshNavigator nav = mesh->navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                straight_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                add_straight_wireframe(nav.dart());
                nav = nav.rotate_on_face();
            }
            straight_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);

            Vector3f normal = nav.face().normal * normal_sign;
            straight_model.surface_vert_norm.push_back(normal);
            straight_model.surface_vert_norm.push_back(normal);
            straight_model.surface_vert_norm.push_back(normal);

            Vector3f color = Vector3f(1 - nav.hexa().scaled_jacobian, 0, nav.hexa().scaled_jacobian);
            straight_model.surface_vert_color.push_back(color);
            straight_model.surface_vert_color.push_back(color);
            straight_model.surface_vert_color.push_back(color);
        }
    }

    void CullingPlaneView::add_straight_wireframe(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);
        if (nav.edge().mark != mesh->mark) {
            nav.edge().mark = mesh->mark;
            MeshNavigator edge_nav = nav;
            for (int v = 0; v < 2; ++v) {
                straight_model.wireframe_vert_pos.push_back(mesh->verts[edge_nav.dart().vert].position);
                edge_nav = edge_nav.flip_vert();
            }
        }
    }

    void CullingPlaneView::add_hidden_face(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                hidden_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                add_hidden_wireframe(nav.dart());
                nav = nav.rotate_on_face();
            }
            hidden_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);

            Vector3f normal = nav.face().normal;
            hidden_model.surface_vert_norm.push_back(normal);
            hidden_model.surface_vert_norm.push_back(normal);
            hidden_model.surface_vert_norm.push_back(normal);
        }
    }

    void CullingPlaneView::add_hidden_wireframe(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);
        //if (edge_marks[nav.dart().edge] != mark) {
            nav.edge().mark = mesh->mark;
            MeshNavigator edge_nav = nav;
            for (int v = 0; v < 2; ++v) {
                hidden_model.wireframe_vert_pos.push_back(mesh->verts[edge_nav.dart().vert].position);
                edge_nav = edge_nav.flip_vert();
            }
        //}
    }

    void CullingPlaneView::update() {
        if (mesh == nullptr) {
            return;
        }

        ++mesh->mark;
        
        straight_model.clear();
        hidden_model.clear();
        int qwe = 0;
        // plane culling
        auto t_culling = sample_time();
        for (unsigned int i = 0; i < mesh->hexas.size(); ++i) {
            Hexa& hexa = mesh->hexas[i];

            // front face plane cull check
            MeshNavigator nav = mesh->navigate(hexa);
            if (plane_cull_test(nav.face())) { ++qwe;  continue; }
            nav = nav.rotate_on_hexa().rotate_on_hexa();
            if (plane_cull_test(nav.face())) { ++qwe;  continue; }

            // mark the hexa as visible
            nav.hexa().mark = mesh->mark;
        }
        auto dt_culling = milli_from_sample(t_culling);
        HL_LOG("[%s] Culling took %dms.\n", name, dt_culling);
         
        // face drawing
        auto t_building = sample_time();
        for (size_t i = 0; i < mesh->faces.size(); ++i) {
            MeshNavigator nav = mesh->navigate(mesh->faces[i]);
            // hexa a visible, hexa b not existing or not visible
            if (nav.hexa().mark == mesh->mark && (nav.dart().hexa_neighbor == -1 || nav.flip_hexa().hexa().mark != mesh->mark)) {
                add_straight_face(nav.dart(), 1);
                // hexa a invisible, hexa b existing and visible
            } else if (nav.hexa().mark != mesh->mark && nav.dart().hexa_neighbor != -1 && nav.flip_hexa().hexa().mark == mesh->mark) {
                nav = nav.flip_hexa().flip_edge();
                add_straight_face(nav.dart(), -1);
                // face was culled by the plane, is surface
            } else if (nav.hexa().mark != mesh->mark && nav.dart().hexa_neighbor == -1) {
                add_hidden_face(nav.dart());
            }
        }
        auto dt_building = milli_from_sample(t_building);
        HL_LOG("[Culling Plane View] Face pass took %dms.\n", dt_building);
    }
}
