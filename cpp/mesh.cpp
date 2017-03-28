#include "mesh.h"

#include <string.h>
#include <string>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    void Mesh::validate() {
        for (int i = 0; i < 48; ++i) {
            const Dart& a = darts[i];

            const Dart& f1 = darts[a.face_neighbor];
            const Dart& f2 = darts[f1.face_neighbor];
            assert(a.hexa == f2.hexa
                && a.face == f2.face
                && a.edge == f2.edge
                && a.vertex == f2.vertex);

            const Dart& e1 = darts[a.edge_neighbor];
            const Dart& e2 = darts[e1.edge_neighbor];
            assert(a.hexa == e2.hexa
                && a.face == e2.face
                && a.edge == e2.edge
                && a.vertex == e2.vertex);

            const Dart& v1 = darts[a.edge_neighbor];
            const Dart& v2 = darts[v1.edge_neighbor];
            assert(a.hexa == v2.hexa
                && a.face == v2.face
                && a.edge == v2.edge
                && a.vertex == v2.vertex);

            // TODO add more asserts
        }
    }
}
