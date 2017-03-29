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
        Dart* _origin;
        Dart* _dart;
        Mesh* _mesh;
    public:
        MeshNavigator(Dart* origin, Dart* dart, Mesh* mesh)
            : _origin(origin)
            , _dart(dart)
            , _mesh(mesh) {}

        MeshNavigator& flipH();
        MeshNavigator& flipF();
        MeshNavigator& flipE();
        MeshNavigator& flipV();

        Hexa& hexa();
        Face& face();
        Edge& edge();
        Vert& vert();
        Dart& dart() { return *_dart; }

        // Is a shallow comparison correct ?
        bool is_origin() { return _dart == _origin; }

        void set_origin() { _origin = _dart; }
    };
}

#endif