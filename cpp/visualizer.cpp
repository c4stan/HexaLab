#include "visualizer.h"

#include "mesh_navigator.h"

namespace HexaLab {
    void Visualizer::update_vbuffer() {
        assert(this->mesh != nullptr);
        
        auto& verts = this->mesh->get_verts();
        for (unsigned int i = 0; i < verts.size(); ++i) {
            this->vbuffer.push_back(verts[i].position);
        }
    }

    void Visualizer::update_ibuffer() {
        this->ibuffer.clear();

        auto& hexas = this->mesh->get_hexas();
        for (unsigned int i = 0; i < hexas.size(); ++i) {
            Hexa& hexa = mesh->get_hexa(i);

            // Try to ignore the hexa if it's surrounded
            bool surrounded = true;
            for (int j = 0; j < 6; ++j) {
                if (hexa.neighbors[j] == -1) {
                    surrounded = false;
                    break;
                }
            }
            if (surrounded) {
                hexa.is_visible = false;
                continue;
            }

            // Try to cull the hexa with the plane
            bool culled = 0;
            auto nav = mesh->navigate(hexa);

            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.flipV().flipE(); // next face vertex
            } while(!nav.is_origin());
            if (culled) {
                hexa.is_visible = false;
                continue;
            }

            nav.flipE().flipF().flipE().flipV().flipE().flipF(); // front face -> back face

            nav.set_origin();
            do {
                if (plane.signedDistance(nav.vert().position) < 0) {
                    culled = true;
                    break;
                }
                nav.flipV().flipE(); // next face vertex
            } while(!nav.is_origin());
            if (culled) {
                hexa.is_visible = false;
                continue;
            }

            // The hexa is visible. Add it to the ibuffer.
            nav = mesh->navigate(hexa);
            for (int j = 0; j < 6; ++j) {
                // store the index used as first for both this face's triangles
                Index face_origin = nav.dart().vert;
                // add the first triangle
                ibuffer.push_back(face_origin);
                nav.flipV().flipE();
                ibuffer.push_back(nav.dart().vert);
                nav.flipV();
                ibuffer.push_back(nav.dart().vert);
                // add the second triangle
                ibuffer.push_back(face_origin);
                nav.flipE();
                ibuffer.push_back(nav.dart().vert);
                nav.flipV();
                ibuffer.push_back(nav.dart().vert);
            }
        }
    }
}