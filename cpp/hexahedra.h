#pragma once

// HexaLab
#include "common.h"

// STD
#include <string.h>

// STL
#include <vector>

// Eigen
#include <eigen/dense>

namespace HexaLab {
	using namespace std;

	// A global reference to the current vertex buffer is kept in order to be able to hash
	// vertices from the indices stored in Edge/FaceID instances
	//extern std::vector<Eigen::Vector3f>* g_vbuffer;

	struct Dart {
        Index hexa_neighbor;
        Index face_neighbor;
        Index edge_neighbor;
        Index vertex_neighbor;
        Index hexa;
        Index face;
        Index edge;
        Index vertex;

		Dart() {
			this->hexa_neighbor = -1;
			this->face_neighbor = -1;
			this->edge_neighbor = -1;
			this->vertex_neighbor = -1;
		}
    };

    struct Vertex {
		Index dart;

		Vertex() {
			this->dart = -1;
		}
	};

	struct Edge {
		Index dart;

		Edge() {
			this->dart = -1;
		}
	};

	// This structure gets hashed and inserted into a set during the mesh loading.
	// This way we can tell when the current edge has already been processed, and if it was,
	// we also get access to it.
	struct EdgeID {
		Index vertices[2];

		EdgeID() {
			vertices[0] = -1;
			vertices[1] = -1;
		}

		bool operator==(const EdgeID& other) const {
			return this->vertices[0] == other.vertices[0]
				&& this->vertices[1] == other.vertices[1];
		}
	};

	struct Face {
		Index dart;

		Face() {
			this->dart = -1;
		}

		enum Dart {
			BotRight,
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

		/*enum Vertex {
			TopRight = 0,
			TopLeft,
			BotLeft,
			BotRight
		};*/

		enum Hexa {
			Front = 0,
			Back
		};
	};

	// This structure gets hashed and inserted into a set during the mesh loading.
	// This way we can tell when the current edge has already been processed, and if it was,
	// we also get access to it.
	struct FaceID {
		Index vertices[4];

		bool operator==(const FaceID& other) const {
			return this->vertices[0] == other.vertices[0] 
				&& this->vertices[1] == other.vertices[1]
				&& this->vertices[2] == other.vertices[2]
				&& this->vertices[3] == other.vertices[3];
		}
	};

	struct FaceValue {
		Index face_idx;
		Index hexas[2];

		FaceValue() {
			face_idx = -1;
			hexas[0] = -1;
			hexas[1] = -1;
		}
	};


	struct Hexa {
		Index neighbors[6];
		Index dart;

		Hexa() {
			this->neighbors[0] = -1;
			this->neighbors[1] = -1;
			this->neighbors[2] = -1;
			this->neighbors[3] = -1;
			this->neighbors[4] = -1;
			this->neighbors[5] = -1;
			dart = -1;
		}

		enum Face {
			Right = 0,
			Left,
			Top,
			Bottom,
			Near,
			Far
		};

		enum Vertex {
			NearBotRight = 0,
			NearBotLeft = 1,
			NearTopLeft = 2,
			NearTopRight = 3,
			FarBotRight = 4,
			FarBotLeft = 5,
			FarTopLeft = 6,
			FarTopRight = 7,
		};
	};
}


namespace std {
	template <> struct hash<HexaLab::EdgeID> {
		size_t operator()(const HexaLab::EdgeID& e)const  {
			/*Eigen::Vector3f& a = (*HexaLab::g_vbuffer)[e.vertices[0]];
			Eigen::Vector3f& b = (*HexaLab::g_vbuffer)[e.vertices[1]];
			Eigen::Vector3f v = a + b;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t f = (h[0] + h[1]*11 - (h[2]*17)) & 0x7fffffff;
			return (f>>22)^(f>>12)^(f);*/
			return e.vertices[0] + e.vertices[1];
		}
	};
}

namespace std {
	template <> struct hash<HexaLab::FaceID> {
		size_t operator()(const HexaLab::FaceID& f)const  {
			/*Eigen::Vector3f& a = (*HexaLab::g_vbuffer)[f.vertices[0]];
			Eigen::Vector3f& b = (*HexaLab::g_vbuffer)[f.vertices[1]];
			Eigen::Vector3f& c = (*HexaLab::g_vbuffer)[f.vertices[2]];
			Eigen::Vector3f& d = (*HexaLab::g_vbuffer)[f.vertices[3]];
			Eigen::Vector3f v = a + b + c + d;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t x = (h[0] + h[1]*11 - (h[2]*17)) & 0x7fffffff;
			return (x>>22)^(x>>12)^(x);*/
			return f.vertices[0] + f.vertices[1] + f.vertices[2] + f.vertices[3];
		}
	};
}