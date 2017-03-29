#include "loader.h"

#include <fstream>

namespace HexaLab {
    
    MeshData Loader::load(const std::string& path) {
        MeshData data;

        std::string header;

        std::ifstream stream(path, std::ifstream::in | std::ifstream::binary);
        if (!stream.is_open()) {
            HL_LOG("Unable to open file %s!\n", path.c_str());
            goto error;
        }

        //char buffer[100];
        int precision;
        int dimension;
        
        while (stream.good()) {
            // Read a line
            if (!(stream >> header)) {
                HL_LOG("ERROR: malformed mesh file. Is the file ended correctly?\n");
                goto error;
            }

            // Precision
            if (header.compare("MeshVersionFormatted") == 0) {
                if (!(stream >> precision)) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after %s tag.\n", header.c_str());
                    goto error;
                }
            // Dimension
            } else if (header.compare("Dimension") == 0) {
                if (!(stream >> dimension)) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after %s tag.\n", header.c_str());
                    goto error;
                }
            // Vertices
            } else if (header.compare("Vertices") == 0) {
                int vertices_count;
                if (!(stream >> vertices_count)) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after %s tag.\n", header.c_str());
                    goto error;
                }
                HL_LOG("Reading %d vertices...\n", vertices_count);
                data.verts.reserve(vertices_count);
                for (int i = 0; i < vertices_count; ++i) {
                    Eigen::Vector3f v;
                    float x;
                    if (!(stream >> v.x() >> v.y() >> v.z() >> x)) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected vertex data format.\n");
                        goto error;                       
                    }
                    data.verts.push_back(v);
                }
            // Quad indices
            } else if (header.compare("Quadrilaterals") == 0) {
                int quads_count;
                if (!(stream >> quads_count)) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after quads tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d quads... (unused)\n", quads_count);
                for (int i = 0; i < quads_count; ++i) {
                    Index idx[4];
                    Index x;
                    if (!(stream >> idx[0] >> idx[1] >> idx[2] >> idx[3] >> x)) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected quad format.\n");
                        goto error;                       
                    }
                }
            // Hex indices
            } else if (header.compare("Hexahedra") == 0) {
                int hexas_count;
                if (!(stream >> hexas_count)) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected tag after hexahedras tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d hexas...\n", hexas_count);
                for (int h = 0; h < hexas_count; ++h) {
                    MeshData::Hexa hexa;
                    Index x;
                    if (!(stream >> hexa.verts[0] >> hexa.verts[1] >> hexa.verts[2] >> hexa.verts[3] >> hexa.verts[4] >> hexa.verts[5] >> hexa.verts[6] >> hexa.verts[7] >> x)) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected hexahedra data format.\n");
                        goto error;
                    }
                    for (int j = 0; j < 8; ++j) hexa.verts[j] = hexa.verts[j] - 1;                    
                    data.hexas.push_back(hexa);
                }
            // End of file
            } else if (header.compare("End") == 0) {
                break;
            // Unknown token
            } else {
                HL_LOG("ERROR: malformed mesh file. Unexpected header tag.\n");
                goto error;
            }
        }

        // Make sure at least vertex and hexa index data was read
        if (data.verts.size() == 0) {
            HL_LOG("ERROR: mesh does not containt any vertex!\n");
            goto error;
        }
        if (data.hexas.size() == 0) {
            HL_LOG("ERROR: mesh does not contain any hexa!\n");
            goto error;
        }

        data._is_good = true;
        return data;

        error:
        data._is_good = false;
        return data;
    }
}