#ifndef _HL_MESH_NAVIGATOR_H_
#define _HL_MESH_NAVIGATOR_H_

#include "common.h"
#include "dart.h"

namespace HexaLab {
    class Mesh;
    struct Hexa;
    struct Face;
    struct Edge;
    struct Vert;

    class MeshNavigator {
    private:
        Dart* _dart;
        Mesh* _mesh;
    public:
        MeshNavigator(Dart& dart, Mesh& mesh)
            : _dart(&dart)
            , _mesh(&mesh) {}

        MeshNavigator& flip_hexa();
        MeshNavigator& flip_face();
        MeshNavigator& flip_edge();
        MeshNavigator& flip_vert();

        Hexa& hexa();
        Face& face();
        Edge& edge();
        Vert& vert();
        Dart& dart() { return *_dart; }
    };
}

#endif