#include "visualizer.h"

namespace HexaLab {
    bool Visualizer::import_mesh(std::string path) {
        
        HL_LOG("Loading %s...\n", path.c_str());
        vector<Vector3f> verts;
        vector<Index> indices;
		if (!Loader::load(path, verts, indices)) {
			return false;
		}
		
		HL_LOG("Processing...\n");
		Builder::build(mesh, verts, indices);
		
        if (!Builder::validate(mesh)) {
            return false;
        }

        update_vbuffer();
        update_view();

		return true;
    }

    void Visualizer::update_vbuffer() {
        this->vbuffer.clear();
        this->mesh_aabb = AlignedBox3f();
        
        auto& verts = this->mesh.get_verts();
        size_t k = verts.size();
        for (unsigned int i = 0; i < verts.size(); ++i) {
            this->vbuffer.push_back(verts[i].position);
            this->mesh_aabb.extend(verts[i].position);
        }
    }

    void Visualizer::update_view() {
        this->ibuffer.clear();
        this->normals.clear();

        vector<Index> visible_faces;

        auto& hexas = this->mesh.get_hexas();
        for (unsigned int i = 0; i < hexas.size(); ++i) {
            Hexa& hexa = mesh.get_hexa(i);
            
            // PLANE CULL CHECK
            bool culled = 0;
            MeshNavigator nav; 
            const Vert* begin;
            
            // front face
            nav = mesh.navigate(hexa);
            begin = &nav.vert();
            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.rotate_on_face();
            } while (nav.vert() != *begin);
            if (culled) {
                hexa.is_visible = false;
                continue;
            }
            
            // back face
            nav.rotate_on_hexa().rotate_on_hexa();
            begin = &nav.vert();
            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.rotate_on_face();
            } while (nav.vert() != *begin);
            if (culled) {
                hexa.is_visible = false;
                continue;
            }

            // PROXIMITY CHECK
            nav = mesh.navigate(hexa);
            const Face* front_face = &nav.face();
            do {
                if (nav.dart().hexa_neighbor == -1) {
                    // DRAW
                    // split the face in 2 triangles and add their indices to the ibuffer
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    // normal
                    //mesh.navigate(nav.face()).hexa() == nav.hexa() ? normals.push_back(nav.face().normal) : normals.push_back(nav.face().normal * -1);
                    normals.push_back(nav.face().normal);
                }
                nav.next_hexa_face();

                nav.flip_edge();
                if (nav.dart().hexa_neighbor == -1) {
                    // DRAW
                    // split the face in 2 triangles and add their indices to the ibuffer
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    nav.rotate_on_face();
                    ibuffer.push_back(nav.dart().vert);
                    // normal
                    //mesh.navigate(nav.face()).hexa() == nav.hexa() ? normals.push_back(nav.face().normal) : normals.push_back(nav.face().normal * -1);
                    normals.push_back(nav.face().normal);
                }
                nav.flip_edge();
                nav.next_hexa_face();
            } while (nav.face() != *front_face);
        }
    }
}