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
        Index dart = -1;
        bool is_visible;

        Hexa(){}
        Hexa(Index dart) { this->dart = dart; }

        Hexa(const Hexa& other) = delete;
        Hexa(const Hexa&& other) {
            this->dart = dart;
            this->is_visible = other.is_visible;
        }

        bool operator==(const Hexa& other) const {
            return this->dart == other.dart;
        }
        bool operator!=(const Hexa& other) const { return !(*this == other); }
    };

    struct Face {
        Index dart = -1;

        Face(){}
        Face(Index dart) { this->dart = dart; }

        Face(const Face& other) = delete;
        Face(const Face&& other) {
            this->dart = other.dart;    
        }

        bool operator==(const Face& other) const {
            return this->dart == other.dart;
        }
        bool operator!=(const Face& other) const { return !(*this == other); }
    };

    struct Edge {
        Index dart = -1;

        Edge(){}
        Edge(Index dart) { this->dart = dart; }

        Edge(const Edge& other) = delete;
        Edge(const Edge&& other) {
            this->dart = other.dart;
        }

        bool operator==(const Edge& other) const {
            return this->dart == other.dart;
        }
        bool operator!=(const Edge& other) const { return !(*this == other); }
    };

    struct Vert {
        Index dart = -1;
        Vector3f position;

        Vert(){}
        Vert(Vector3f position) {
            this->position = position;
        }
        Vert(Vector3f position, Index dart) {
            this->position = position;
            this->dart = dart;
        }

        Vert(const Vert& other) = delete;
        Vert(const Vert&& other) {
            this->dart = other.dart;
            this->position = other.position;
        }

        bool operator==(const Vert& other) const {
            return this->dart == other.dart
                && this->position == other.position;    // TODO non necessary ?
        }
        bool operator!=(const Vert& other) const { return !(*this == other); }
    };
    
class Mesh {
    friend class Builder;

        vector<Hexa> hexas;
        vector<Face> faces;
        vector<Edge> edges;
        vector<Vert> verts;
        vector<Dart> darts;

    public:
        std::vector<Hexa>& get_hexas() { return this->hexas; }
        std::vector<Face>& get_faces() { return this->faces; }
        std::vector<Edge>& get_edges() { return this->edges; }
        std::vector<Vert>& get_verts() { return this->verts; }

        Hexa& get_hexa(Index i) { return this->hexas[i]; }
        Face& get_face(Index i) { return this->faces[i]; }
        Edge& get_edge(Index i) { return this->edges[i]; }
        Vert& get_vert(Index i) { return this->verts[i]; }

        std::vector<Dart>& get_darts() { return this->darts; }

        Dart& get_dart(Index i) { return this->darts[i]; }
        Dart& get_dart(Hexa& hexa) { return this->darts[hexa.dart]; }
        Dart& get_dart(Face& face) { return this->darts[face.dart]; }
        Dart& get_dart(Edge& edge) { return this->darts[edge.dart]; }
        Dart& get_dart(Vert& vert) { return this->darts[vert.dart]; }

        MeshNavigator navigate(Dart& dart) { return MeshNavigator(dart, *this); }
        MeshNavigator navigate(Hexa& hexa) { Dart& d = get_dart(hexa); return navigate(d); }
        MeshNavigator navigate(Face& face) { Dart& d = get_dart(face); return navigate(d); }
        MeshNavigator navigate(Edge& edge) { Dart& d = get_dart(edge); return navigate(d); }
        MeshNavigator navigate(Vert& vert) { Dart& d = get_dart(vert); return navigate(d); }
	};
}

#endif