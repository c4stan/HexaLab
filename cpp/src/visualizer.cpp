#include <visualizer.h>

namespace HexaLab {
    bool Visualizer::import_mesh(std::string path) {
        
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
        this->vbuffer.clear();
        this->mesh_aabb = AlignedBox3f();
        
        for (unsigned int i = 0; i < mesh.verts.size(); ++i) {
            this->vbuffer.push_back(mesh.verts[i].position);
            this->mesh_aabb.extend(mesh.verts[i].position);
        }
    }

    void Visualizer::update_view() {
        this->faces.clear();
        this->edges.clear();
        this->verts.clear();

        // culling prepass
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

        // face pass
        for (size_t i = 0; i < mesh.faces.size(); ++i) {
            MeshNavigator nav = mesh.navigate(mesh.faces[i]);

            // hexa a visible, hexa b not existing or not visible
            if (nav.hexa().mark == mark && (nav.dart().hexa_neighbor == -1 || nav.flip_hexa().hexa().mark != mark)) {
                nav.face().mark = mark;
                ViewFace face;
                for (int v = 0; v < 4; ++v) {
                    face.indices[v] = nav.dart().vert;
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
                    nav = nav.rotate_on_face();
                }
                face.normal = nav.face().normal * -1;
                faces.push_back(face);
            }
        }
        
        // edge pass
        for (size_t i = 0; i < mesh.edges.size(); ++i) {
            MeshNavigator nav = mesh.navigate(mesh.edges[i]);

            const Face& origin = nav.face();

            do {
                if (nav.face().mark == mark) {
                    ViewEdge edge;
                    for (int v = 0; v < 2; ++v) {
                        edge.indices[v] = nav.dart().vert;
                        nav = nav.flip_vert();
                    }
                    edges.push_back(edge);
                    break;
                }
                if (nav.dart().hexa_neighbor == -1) break;
                nav = nav.rotate_on_edge();
            } while (nav.face() != origin);
        }
        
        // vert pass

        // hexa pass

        ++mark;
    }
}