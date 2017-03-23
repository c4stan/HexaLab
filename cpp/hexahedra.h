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
	extern std::vector<Eigen::Vector3f>* g_vbuffer;

    struct Vertex {
		Index edges[20];
		int e_count;

		Vertex() {
			this->e_count = 0;
		}
	};

	struct Edge {
		Index vertices[2];
		Index faces[20];
		int f_count;

		Edge() {
			this->vertices[0] = 0;
			this->vertices[1] = 0;
			this->f_count = 0;
		}
	};

	// This structure gets hashed and inserted into a set during the mesh loading.
	// This way we can tell when the current edge has already been processed, and if it was,
	// we also get access to it.
	struct EdgeID {
		Index edge_idx;
		Index vertices[2];

		bool operator==(const EdgeID& other) const {
			return this->vertices[0] == other.vertices[0]
				&& this->vertices[1] == other.vertices[1];
		}
	};

	struct Face {
		Index vertices[4];
		Index edges[4];
		Index hexas[2];

		Face() {
			this->vertices[0] = 0;
			this->vertices[1] = 0;
			this->vertices[2] = 0;
			this->vertices[3] = 0;
			this->edges[0] = -1;
			this->edges[1] = -1;
			this->edges[2] = -1;
			this->edges[3] = -1;
			this->hexas[0] = -1;
			this->hexas[1] = -1;
		}

		enum Edge {
			Right = 0,
			Left,
			Top,
			Bottom,
		};

		enum Vertex {
			TopRight = 0,
			TopLeft,
			BotLeft,
			BotRight
		};

		enum Hexa {
			Front = 0,
			Back,
		};
	};

	// This structure gets hashed and inserted into a set during the mesh loading.
	// This way we can tell when the current edge has already been processed, and if it was,
	// we also get access to it.
	struct FaceID {
		Index face_idx;
		Index vertices[4];

		bool operator==(const FaceID& other) const {
			return this->vertices[0] == other.vertices[0] 
				&& this->vertices[1] == other.vertices[1]
				&& this->vertices[2] == other.vertices[2]
				&& this->vertices[3] == other.vertices[3];
		}
	};

	struct Hexa {
		Index faces[6];
		u32 is_culled 		: 1;
		u32 is_surface 		: 1;
		u32 _pad;

		Hexa() {
			this->faces[0] = -1;
			this->faces[1] = -1;
			this->faces[2] = -1;
			this->faces[3] = -1;
			this->faces[4] = -1;
			this->faces[5] = -1;
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
			NearTopRight = 0,
			NearTopLeft = 1,
			NearBotLeft = 2,
			NearBotRight = 3,
			FarTopRight = 4,
			FarTopLeft = 5,
			FarBotLeft = 6,
			FarBotRight = 7
		};
	};
}


namespace std {
	template <> struct hash<HexaLab::EdgeID> {
		size_t operator()(const HexaLab::EdgeID& e)const  {
			Eigen::Vector3f& a = (*HexaLab::g_vbuffer)[e.vertices[0]];
			Eigen::Vector3f& b = (*HexaLab::g_vbuffer)[e.vertices[1]];
			Eigen::Vector3f v = a + b;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t f = (h[0] + h[1]*11 - (h[2]*17)) & 0x7fffffff;
			return (f>>22)^(f>>12)^(f);
		}
	};
}

namespace std {
	template <> struct hash<HexaLab::FaceID> {
		size_t operator()(const HexaLab::FaceID& f)const  {
			Eigen::Vector3f& a = (*HexaLab::g_vbuffer)[f.vertices[0]];
			Eigen::Vector3f& b = (*HexaLab::g_vbuffer)[f.vertices[1]];
			Eigen::Vector3f& c = (*HexaLab::g_vbuffer)[f.vertices[2]];
			Eigen::Vector3f& d = (*HexaLab::g_vbuffer)[f.vertices[3]];
			Eigen::Vector3f v = a + b + c + d;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t x = (h[0] + h[1]*11 - (h[2]*17)) & 0x7fffffff;
			return (x>>22)^(x>>12)^(x);
		}
	};
}