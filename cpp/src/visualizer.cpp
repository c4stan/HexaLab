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
        update_vbuffer();
        update_view();

		return true;
    }

    void Visualizer::update_vbuffer() {
        auto t0 = sample_time();
        this->vbuffer.clear();
        this->mesh_aabb = AlignedBox3f();
        
        for (unsigned int i = 0; i < mesh.verts.size(); ++i) {
            this->vbuffer.push_back(mesh.verts[i].position);
            this->mesh_aabb.extend(mesh.verts[i].position);
        }
        auto dt = milli_from_sample(t0);
        HL_LOG("[Visualizer] Vbuffer building took %dms.\n", dt);
    }

    void Visualizer::update_view() {
        auto t0 = sample_time();

        ++mark;
        
        faces.clear();
        culled_faces.clear();
        edges.clear();
        verts.clear();

        // culling prepass
        auto t_prepass = sample_time();
        for (unsigned int i = 0; i < mesh.hexas.size(); ++i) {
            Hexa& hexa = mesh.hexas[i];

            bool culled = 0;

            // front face plane cull check
            MeshNavigator nav = mesh.navigate(hexa);
            for (int v = 0; v < 4; ++v) {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav = nav.rotate_on_face();
            }
            if (culled) continue;

            // back face plane cull check
            nav = nav.rotate_on_hexa().rotate_on_hexa();
            for (int v = 0; v < 4; ++v) {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav = nav.rotate_on_face();
            }
            if (culled) continue;

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
                nav.face().mark = mark;
                ViewFace face;
                for (int v = 0; v < 4; ++v) {
                    face.indices[v] = nav.dart().vert;
                    
                    if (nav.edge().mark != mark) {
                        nav.edge().mark = mark;
                        MeshNavigator edge_nav = nav;
                        ViewEdge edge;
                        for (int v = 0; v < 2; ++v) {
                            edge.indices[v] = edge_nav.dart().vert;
                            edge_nav = edge_nav.flip_vert();
                        }
                        edges.push_back(edge);
                    }
                    
                    nav = nav.rotate_on_face();
                }
                face.normal = nav.face().normal;
                faces.push_back(face);
            // hexa a invisible, hexa b existing and visible
            } else if (nav.hexa().mark != mark && nav.dart().hexa_neighbor != -1 && nav.flip_hexa().hexa().mark == mark) {
                nav.face().mark = mark;
                nav = nav.flip_hexa().flip_edge();
                ViewFace face;
                for (int v = 0; v < 4; ++v) {
                    face.indices[v] = nav.dart().vert;
                    
                    if (nav.edge().mark != mark) {
                        nav.edge().mark = mark;
                        MeshNavigator edge_nav = nav;
                        ViewEdge edge;
                        for (int v = 0; v < 2; ++v) {
                            edge.indices[v] = edge_nav.dart().vert;
                            edge_nav = edge_nav.flip_vert();
                        }
                        edges.push_back(edge);
                    }
                    
                    nav = nav.rotate_on_face();
                }
                face.normal = nav.face().normal * -1;
                faces.push_back(face);
            // face was culled by the plane, is surface
            } else if(nav.hexa().mark != mark && nav.dart().hexa_neighbor == -1) {
                ViewFace face;
                for (int v = 0; v < 4; ++v) {
                    face.indices[v] = nav.dart().vert;
                    nav = nav.rotate_on_face();
                }
                face.normal = nav.face().normal;
                culled_faces.push_back(face);
            }
        }
        auto dt_facepass = milli_from_sample(t_facepass);
        HL_LOG("[Visualizer] Face pass took %dms.\n", dt_facepass);

        auto dt = milli_from_sample(t0);
        HL_LOG("[Visualizer] View building took %dms in total.\n", dt);
    }
}