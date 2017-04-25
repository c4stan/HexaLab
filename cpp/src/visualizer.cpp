#include <visualizer.h>

namespace HexaLab {
    
    bool Visualizer::import_mesh(std::string path) {

        mesh.hexas.clear();
        mesh.faces.clear();
        mesh.edges.clear();
        mesh.verts.clear();
        mesh.darts.clear();
        
        HL_LOG("Loading %s...\n", path.c_str());
        vector<Vector3f> verts;
        vector<Index> indices;
		if (!Loader::load(path, verts, indices)) {
			return false;
		}
		
		HL_LOG("Building...\n");
		Builder::build(mesh, verts, indices);

        HL_LOG("Validating...\n");
        if (!Builder::validate(mesh)) {
            return false;
        }

        HL_LOG("Preparing the view...\n");
        update_static_buffers();
        update_dynamic_buffers();

		return true;
    }

    void Visualizer::update_static_buffers() {
        auto t0 = sample_time();
        vert_pos.clear();
        mesh_aabb = AlignedBox3f();
        
        for (unsigned int i = 0; i < mesh.verts.size(); ++i) {
            vert_pos.push_back(mesh.verts[i].position);
            mesh_aabb.extend(mesh.verts[i].position);
        }
        auto dt = milli_from_sample(t0);
        HL_LOG("[Visualizer] Vbuffer building took %dms.\n", dt);
    
        bad_edge_pos.clear();
        bad_edge_color.clear();

        for (size_t i = 0; i < mesh.edges.size(); ++i) {
            MeshNavigator nav = mesh.navigate(mesh.edges[i]);
            if (nav.edge().face_count == 4) continue;
            if (nav.edge().surface) continue;
            for (int j = 0; j < 2; ++j) {
                bad_edge_pos.push_back(vert_pos[nav.dart().vert]);
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
            bad_edge_color.push_back(color);
            bad_edge_color.push_back(color);
        }

        for (size_t i = 0; i < mesh.hexas.size(); ++i) {
            if (mesh.hexas[i].scaled_jacobian > 0.5) continue;

        }
    }

    void Visualizer::add_visible_edge(Dart& dart) {
        MeshNavigator nav = mesh.navigate(dart);
        if (nav.edge().mark != mark) {
            nav.edge().mark = mark;
            MeshNavigator edge_nav = nav;
            for (int v = 0; v < 2; ++v) {
                visible_edge_idx.push_back(edge_nav.dart().vert);
                edge_nav = edge_nav.flip_vert();
            }
        }
    }

    void Visualizer::add_culled_edge(Dart& dart) {
        MeshNavigator nav = mesh.navigate(dart);
        if (nav.edge().mark != mark) {
            nav.edge().mark = mark;
            MeshNavigator edge_nav = nav;
            for (int v = 0; v < 2; ++v) {
                culled_edge_idx.push_back(edge_nav.dart().vert);
                edge_nav = edge_nav.flip_vert();
            }
        }
    }

    void Visualizer::add_visible_face(Dart& dart, float normal_sign) {
        MeshNavigator nav = mesh.navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                visible_face_pos.push_back(vert_pos[nav.dart().vert]);
                add_visible_edge(nav.dart());
                nav = nav.rotate_on_face();
            }
            visible_face_pos.push_back(vert_pos[nav.dart().vert]);

            Vector3f normal = nav.face().normal * normal_sign;
            visible_face_norm.push_back(normal);
            visible_face_norm.push_back(normal);
            visible_face_norm.push_back(normal);

            Vector3f color = Vector3f(1 - nav.hexa().scaled_jacobian, 0, nav.hexa().scaled_jacobian);
            visible_face_color.push_back(color);
            visible_face_color.push_back(color);
            visible_face_color.push_back(color);
        }
    }

    void Visualizer::add_culled_face(Dart& dart) {
        MeshNavigator nav = mesh.navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                culled_face_pos.push_back(vert_pos[nav.dart().vert]);
                add_culled_edge(nav.dart());
                nav = nav.rotate_on_face();
            }
            culled_face_pos.push_back(vert_pos[nav.dart().vert]);

            Vector3f normal = nav.face().normal;
            culled_face_norm.push_back(normal);
            culled_face_norm.push_back(normal);
            culled_face_norm.push_back(normal);
        }
    }

    bool Visualizer::plane_cull_test(Face& face) {
        MeshNavigator nav = mesh.navigate(face);
        for (int v = 0; v < 4; ++v) {
            if (plane.signedDistance(nav.vert().position) < 0) {
                return true;
                break;
            }
            nav = nav.rotate_on_face();
        }
        return false;
    }

    void Visualizer::update_dynamic_buffers() {
        auto t0 = sample_time();

        ++mark;
        
        visible_face_pos.clear();
        visible_face_norm.clear();
        visible_face_color.clear();
        culled_face_pos.clear();
        culled_face_norm.clear();
        visible_edge_idx.clear();
        culled_edge_idx.clear();

        // culling prepass
        auto t_prepass = sample_time();
        for (unsigned int i = 0; i < mesh.hexas.size(); ++i) {
            Hexa& hexa = mesh.hexas[i];

            // front face plane cull check
            MeshNavigator nav = mesh.navigate(hexa);
            if (plane_cull_test(nav.face())) continue;
            nav = nav.rotate_on_hexa().rotate_on_hexa();
            if (plane_cull_test(nav.face())) continue;

            // mark the hexa as visible
            hexa.mark = mark;
        }
        auto dt_prepass = milli_from_sample(t_prepass);
        HL_LOG("[Visualizer] Plane culling took %dms.\n", dt_prepass);

        // face pass
        auto t_facepass = sample_time();
        for (size_t i = 0; i < mesh.faces.size(); ++i) {
            MeshNavigator nav = mesh.navigate(mesh.faces[i]);
            // hexa a visible, hexa b not existing or not visible
            if (nav.hexa().mark == mark && (nav.dart().hexa_neighbor == -1 || nav.flip_hexa().hexa().mark != mark)) {
                add_visible_face(nav.dart(), 1);
            // hexa a invisible, hexa b existing and visible
            } else if (nav.hexa().mark != mark && nav.dart().hexa_neighbor != -1 && nav.flip_hexa().hexa().mark == mark) {
                nav = nav.flip_hexa().flip_edge();
                add_visible_face(nav.dart(), -1);
                // face was culled by the plane, is surface
            } else if (nav.hexa().mark != mark && nav.dart().hexa_neighbor == -1) {
                add_culled_face(nav.dart());
            }
        }
        auto dt_facepass = milli_from_sample(t_facepass);
        HL_LOG("[Visualizer] Face pass took %dms.\n", dt_facepass);

        auto dt = milli_from_sample(t0);
        HL_LOG("[Visualizer] View building took %dms in total.\n", dt);
    }
}