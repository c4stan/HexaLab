#ifndef _HL_MESH_H_
#define _HL_MESH_H_

#include "common.h"
#include <vector>
#include <eigen/dense>
#include <eigen/geometry>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;
    
	class Mesh {
        friend class Builder;

    public:
        struct Hexa {
            Index dart;
            Index neighbors[6] = { -1, -1, -1, -1, -1, -1};
        };

        struct Face {
            Index dart;
        };

        struct Edge {
            Index dart;
        };

        struct Vert {
            Index dart;
            Vector3f position;
        };

        struct Dart {
            Index hexa_neighbor = -1;
            Index face_neighbor = -1;
            Index edge_neighbor = -1;
            Index vert_neighbor = -1;
            Index hexa = -1;
            Index face = -1;
            Index edge = -1;
            Index vert = -1;
        };

	private:
        vector<Hexa> hexas;
        vector<Face> faces;
        vector<Edge> edges;
        vector<Vert> verts;
        vector<Dart> darts;
		AlignedBox3f aabb;

	public:
        vector<Vert>& get_verts() { return this->verts; }
        vector<Edge>& get_edges() { return this->edges; }
        vector<Face>& get_faces() { return this->faces; }
        vector<Hexa>& get_hexas() { return this->hexas; }

        AlignedBox3f& get_aabb() { return this->aabb; }
		float get_center_x() { return this->aabb.center().x(); }
		float get_center_y() { return this->aabb.center().y(); }
		float get_center_z() { return this->aabb.center().z(); }
		float get_diagonal_size() { return this->aabb.diagonal().norm(); }

        void validate();

        Dart& flipV(Dart& dart) { return this->darts[dart.vert_neighbor]; }
        Dart& flipE(Dart& dart) { return this->darts[dart.edge_neighbor]; }
        Dart& flipF(Dart& dart) { return this->darts[dart.face_neighbor]; }
        Dart& flipH(Dart& dart) { return this->darts[dart.hexa_neighbor]; }
	};
}

#endif