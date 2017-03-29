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

        MeshNavigator navigate(Dart& dart) { return MeshNavigator(&dart, &dart, this); }
        MeshNavigator navigate(Hexa& hexa) { Dart& d = get_dart(hexa); return navigate(d); }
        MeshNavigator navigate(Face& face) { Dart& d = get_dart(face); return navigate(d); }
        MeshNavigator navigate(Edge& edge) { Dart& d = get_dart(edge); return navigate(d); }
        MeshNavigator navigate(Vert& vert) { Dart& d = get_dart(vert); return navigate(d); }

        MeshNavigator flipH(const Dart& dart) { Dart& d = get_dart(dart.hexa_neighbor); return navigate(d); }
        MeshNavigator flipF(const Dart& dart) { Dart& d = get_dart(dart.face_neighbor); return navigate(d); }
        MeshNavigator flipE(const Dart& dart) { Dart& d = get_dart(dart.edge_neighbor); return navigate(d); }
        MeshNavigator flipV(const Dart& dart) { Dart& d = get_dart(dart.vert_neighbor); return navigate(d); }

        void validate();
	};
}

#endif