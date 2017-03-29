#ifndef _HL_MESH_H_
#define _HL_MESH_H_

#include "common.h"
#include "dart.h"
#include "mesh_navigator.h"
#include <vector>
#include <eigen/dense>
#include <eigen/geometry>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    struct Hexa {
        Index dart;
        Index neighbors[6] = { -1, -1, -1, -1, -1, -1};
        bool is_visible;
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
    
	class Mesh {
        friend class Builder;

	private:
        vector<Hexa> hexas;
        vector<Face> faces;
        vector<Edge> edges;
        vector<Vert> verts;
        vector<Dart> darts;
		AlignedBox3f aabb;

	public:
        vector<Hexa>& get_hexas() { return this->hexas; }
        vector<Face>& get_faces() { return this->faces; }
        vector<Edge>& get_edges() { return this->edges; }
        vector<Vert>& get_verts() { return this->verts; }

        Hexa& get_hexa(Index i) { return this->hexas[i]; }
        Face& get_face(Index i) { return this->faces[i]; }
        Edge& get_edge(Index i) { return this->edges[i]; }
        Vert& get_vert(Index i) { return this->verts[i]; }

        Dart& get_dart(const Hexa& hexa) { return this->darts[hexa.dart]; }
        Dart& get_dart(const Face& face) { return this->darts[face.dart]; }
        Dart& get_dart(const Edge& edge) { return this->darts[edge.dart]; }
        Dart& get_dart(const Vert& vert) { return this->darts[vert.dart]; }
        Dart& get_dart(Index i) { return this->darts[i]; }

        AlignedBox3f& get_aabb() { return this->aabb; }
		float get_center_x() { return this->aabb.center().x(); }
		float get_center_y() { return this->aabb.center().y(); }
		float get_center_z() { return this->aabb.center().z(); }
		float get_diagonal_size() { return this->aabb.diagonal().norm(); }

        MeshNavigator navigate(Dart& dart) { return MeshNavigator(&dart, &dart, this); }
        MeshNavigator navigate(Hexa& hexa) { Dart& dart = get_dart(hexa); return navigate(dart); }
        MeshNavigator navigate(Face& face) { Dart& dart = get_dart(face); return navigate(dart); }
        MeshNavigator navigate(Edge& edge) { Dart& dart = get_dart(edge); return navigate(dart); }
        MeshNavigator navigate(Vert& vert) { Dart& dart = get_dart(vert); return navigate(dart); }

        MeshNavigator flipH(Dart& dart) { return MeshNavigator(&dart, &get_dart(dart.hexa_neighbor), this); }
        MeshNavigator flipF(Dart& dart) { return MeshNavigator(&dart, &get_dart(dart.face_neighbor), this); }
        MeshNavigator flipE(Dart& dart) { return MeshNavigator(&dart, &get_dart(dart.edge_neighbor), this); }
        MeshNavigator flipV(Dart& dart) { return MeshNavigator(&dart, &get_dart(dart.vert_neighbor), this); }

        void validate();
	};
}

#endif