#ifndef _HL_MESH_DATA_H_
#define _HL_MESH_DATA_H_

#include <vector>
#include <eigen/dense>

namespace HexaLab {
    using namespace std;
    using namespace Eigen;

    class MeshData {
        friend class Loader;

    public:
        struct Hexa {
            Index verts[8];
        };

    private:
        vector<Vector3f> verts;
        vector<Hexa> hexas;

        bool _is_good;

    public:
        const vector<Vector3f>& get_verts() const { return this->verts; }
        const vector<Hexa>&     get_hexas() const { return this->hexas; }

        const Vector3f& get_vert(Index i) const { return this->verts[i]; }
        const Hexa&     get_hexa(Index i) const { return this->hexas[i]; }

        bool is_good() { return this->_is_good; }
    };
}

#endif