#ifndef _HL_HEXAHEDRA_H_
#define _HL_HEXAHEDRA_H_

#include "common.h"
#include <eigen/dense>
#include <string.h>
#include <vector>

namespace HexaLab {
	using namespace std;
	using namespace Eigen;

	using IndexPair = std::tuple<Index, Index>;
	using IndexQuad = std::tuple<Index, Index, Index, Index>;

	struct Dart {
        Index hexa_neighbor = -1;
        Index face_neighbor = -1;
        Index edge_neighbor = -1;
        Index vertex_neighbor = -1;
        Index hexa = -1;
        Index face = -1;
        Index edge = -1;
        Index vertex = -1;
    };

    struct Vertex {
		Index dart = -1;
		Vector3f position;
	};

	struct Edge {
		Index dart = -1;
		Index vertices[2] = { -1, -1 };

		bool operator==(const Edge& other) const {
			return (this->vertices[0] == other.vertices[0] && this->vertices[1] == other.vertices[1])
				|| (this->vertices[0] == other.vertices[1] && this->vertices[1] == other.vertices[0]);
		}
	};

	struct Face {
		Index dart = -1;
		Index vertices[4] = { -1, -1, -1, -1 };
		Index hexas[2] = { -1, -1 };
		Vector3f normal;

		enum Dart {
			BotRight = 0,
			BotLeft,
			LeftBot,
			LeftTop,
			TopLeft,
			TopRight,
			RightTop,
			RightBot,
		};

		enum Edge {
			Right = 0,
			Left,
			Top,
			Bottom,
		};

		enum Hexa {
			Front = 0,
			Back
		};

		bool operator==(const Face& other) const {
			return this->vertices[0] == other.vertices[0] 
				&& this->vertices[1] == other.vertices[1]
				&& this->vertices[2] == other.vertices[2]
				&& this->vertices[3] == other.vertices[3];
		}
	};

	struct Hexa {
		Index dart = -1;
		Index neighbors[6] = { -1, -1, -1, -1, -1, -1 };

		enum Face {
			Left = 0,
			Right,
			Bottom,
			Top,
			Near,
			Far,
		};

		static Hexa::Face opposite(Hexa::Face face) {
        switch (face) {
        case Hexa::Face::Near:
            return Hexa::Face::Far;
        case Hexa::Face::Far:
            return Hexa::Face::Near;
        case Hexa::Face::Right:
            return Hexa::Face::Left;
        case Hexa::Face::Left:
            return Hexa::Face::Right;
        case Hexa::Face::Top:
            return Hexa::Face::Bottom;
        case Hexa::Face::Bottom:
            return Hexa::Face::Top;
        }
        // never reached
        assert(false);
        return Hexa::Face::Near; 
    }

		enum Vertex {
			NearBotRight = 0,
			NearBotLeft,
			NearTopLeft,
			NearTopRight,
			FarBotRight,
			FarBotLeft,
			FarTopLeft,
			FarTopRight,
		};
	};
}

namespace std {
	template <> struct hash<HexaLab::IndexPair> {
		size_t operator()(const HexaLab::IndexPair& e)const  {
			return std::get<0>(e) + std::get<1>(e);
		}
	};
}

namespace std {
	template <> struct hash<HexaLab::IndexQuad> {
		size_t operator()(const HexaLab::IndexQuad& f)const  {
			return std::get<0>(f) + std::get<1>(f) + std::get<2>(f) + std::get<3>(f);
		}
	};
}

#endif	// _HL_HEXAHEDRA_H_