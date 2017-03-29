#ifndef _HL_DART_H_
#define _HL_DART_H_

#include "common.h"

namespace HexaLab {
    struct Dart {
        Index hexa_neighbor = -1;
        Index face_neighbor = -1;
        Index edge_neighbor = -1;
        Index vert_neighbor = -1;
        Index hexa = -1;
        Index face = -1;
        Index edge = -1;
        Index vert = -1;

        bool operator==(const Dart& other) {
            // Might limit the comparison to the hexa/face/edeg/vert fields
            return this->hexa_neighbor == other.hexa_neighbor
                && this->face_neighbor == other.face_neighbor
                && this->edge_neighbor == other.edge_neighbor
                && this->vert_neighbor == other.vert_neighbor
                && this->hexa == other.hexa
                && this->face == other.face
                && this->edge == other.edge
                && this->vert == other.vert;
        }
    };
}

#endif