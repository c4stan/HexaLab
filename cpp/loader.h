#ifndef _HL_LOADER_H_
#define _HL_LOADER_H_

#include "common.h"
#include "mesh_data.h"
#include <eigen/dense>
#include <vector>
#include <unordered_map>

namespace HexaLab {
    class Loader {
    public:
        static MeshData load(const std::string& path);
    };
}

#endif