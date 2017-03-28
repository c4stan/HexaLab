#ifndef _HL_LOADER_H_
#define _HL_LOADER_H_

#include "common.h"
#include <eigen/dense>
#include <vector>
#include <unordered_map>

namespace HexaLab {

    class MeshData {
        friend class Loader;

    public:
        using Vert = Eigen::Vector3f;

        struct Hexa {
            Index verts[8];
        };

        std::vector<Vert> verts;
        std::vector<Hexa> hexas;

        bool is_good() {
            return this->_is_good;
        }
    private:
        bool _is_good;
    };

    class Loader {
    public:
        static MeshData load(const std::string& path);
    };
}

#endif