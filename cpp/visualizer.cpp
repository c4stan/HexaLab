#include "visualizer.h"

namespace HexaLab {
    Result Visualizer::import_mesh(std::string path) {
        
        HL_LOG("Loading %s...\n", path.c_str());
		auto data = Loader::load(path);
		if (!data.is_good()) {
			return Result::Error;
		}
		
		HL_LOG("Processing...\n");
		mesh = Builder::build(data);
		
		if (Builder::validate(mesh) != Result::Success) {
            return Result::Error;
        }

        update_vbuffer();
        update_ibuffer();

		return Result::Success;
    }

    void Visualizer::update_vbuffer() {
        this->vbuffer.clear();
        this->mesh_aabb = AlignedBox3f();
        
        auto& verts = this->mesh.get_verts();
        int k = verts.size();
        for (unsigned int i = 0; i < verts.size(); ++i) {
            this->vbuffer.push_back(verts[i].position);
            this->mesh_aabb.extend(verts[i].position);
        }
    }

    void Visualizer::update_ibuffer() {
        this->ibuffer.clear();

        auto& hexas = this->mesh.get_hexas();
        for (unsigned int i = 0; i < hexas.size(); ++i) {
            Hexa& hexa = mesh.get_hexa(i);

            // PROXIMITY CHECK
            bool surrounded = true;
            auto nav = mesh.navigate(hexa);

            const Vert& v0 = nav.vert();
            //HL_LOG("hexa # %d\n", i);
            do {
                if (nav.dart().hexa_neighbor == -1) {
                    surrounded = false;
                    break;
                }
                nav.flip_vert();
                nav.flip_edge();
                nav.flip_face();
            } while (nav.vert() != v0);
            if (surrounded) {
                hexa.is_visible = false;
                continue;
            }

            // PLANE CULL CHECK
            bool culled = 0;
            const Vert& v1 = nav.vert();
            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.flip_vert().flip_edge(); // next face vertex
            } while(nav.vert() != v1);

            if (culled) {
                hexa.is_visible = false;
                continue;
            }

            nav.flip_edge().flip_face().flip_edge().flip_vert().flip_edge().flip_face(); // front face -> back face

            const Vert& v2 = nav.vert();
            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.flip_vert().flip_edge(); // next face vertex
            } while(nav.vert() != v2);

            if (culled) {
                hexa.is_visible = false;
                continue;
            }

             // DRAW
            nav = mesh.navigate(hexa);
            for (int j = 0; j < 4; ++j) {
                // store the index used as first for both this face's triangles
                Index face_origin = nav.dart().vert;
                // add the first triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_vert().flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);
                
                // add the second triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);

                nav.flip_edge().flip_vert().flip_face().flip_edge().flip_vert();
            }
            nav = mesh.navigate(hexa);
            nav.flip_face();
            {
                Index face_origin = nav.dart().vert;
                // add the first triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_vert().flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);
                
                // add the second triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);
            }
            nav = mesh.navigate(hexa);
            nav.flip_edge().flip_vert().flip_edge().flip_face();
            {
                Index face_origin = nav.dart().vert;
                // add the first triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_vert().flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);
                
                // add the second triangle
                ibuffer.push_back(face_origin);
                
                nav.flip_edge();
                ibuffer.push_back(nav.dart().vert);
                
                nav.flip_vert();
                ibuffer.push_back(nav.dart().vert);
            }     
        }
    }
}