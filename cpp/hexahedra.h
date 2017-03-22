#pragma once

namespace HexaLab {
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