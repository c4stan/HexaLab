#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>

struct HLVertex {
	float x;
	float y;
	float z;
};

using HLIndex = int32_t;

struct HLQuad {
	HLIndex idx[4];
};

struct HLHexa {
	HLIndex idx[8];
};

enum class HLResult : uint8_t {
	Success,
	Error
};

#define HL_LOG(...) printf(__VA_ARGS__)

class HLMesh {
private:
	HLVertex* vertices;
	HLQuad* quads;
	HLHexa* hexas;
	int32_t vertices_count;
	int32_t quads_count;
	int32_t hexas_count;

public:
	HLMesh() {
		this->vertices = NULL;
		this->quads = NULL;
		this->hexas = NULL;
		this->vertices_count = 0;
		this->quads_count = 0;
		this->hexas_count = 0;
	}

	uintptr_t get_vertices() { return (uintptr_t) this->vertices; }
	int get_vertices_count() { return this->vertices_count; }

	uintptr_t get_quads() { return (uintptr_t) this->quads; }
	int get_quads_count() { return this->quads_count; }

	uintptr_t get_hexas() { return (uintptr_t) this->hexas; }
	int get_hexas_count() { return this->hexas_count; }

	HLResult load(std::string filename) { 
		FILE* file = fopen(filename.c_str(), "rb");
		if (file == NULL) {
			HL_LOG("Unable to find file %s!\n", filename.c_str());
			return HLResult::Error;
		}

		char buffer[100];
		int precision;
		int dimension;
		
		uint32_t vertices_count = 0;
		uint32_t quads_count = 0;
		uint32_t hexas_count = 0;

		HLVertex* vertices = NULL;
		HLQuad* quads = NULL;
		HLHexa* hexas = NULL;

		while (1) {
			// Read a line
			if (fscanf(file, "%s", buffer) == 0) {
				HL_LOG("ERROR: malformed mesh file. Unexpected file ending.\n");
				goto error;
			}

			// Precision
			if (strcmp(buffer, "MeshVersionFormatted") == 0) {
				if (fscanf(file, "%d", &precision) == 0) {
					HL_LOG("ERROR: malformed mesh file. Unexpected value after precision tag.\n");
					goto error;
				}
			// Dimension
			} else if (strcmp(buffer, "Dimension") == 0) {
				if (fscanf(file, "%d", &dimension) == 0) {
					HL_LOG("ERROR: malformed mesh file. Unexpected value after dimension tag.\n");
					goto error;
				}
			// Vertices
			} else if (strcmp(buffer, "Vertices") == 0) {
				if (fscanf(file, "%d", &vertices_count) == 0 || vertices != NULL) {
					HL_LOG("ERROR: malformed mesh file. Unexpected value after vertices tag.\n");
					goto error;
				}
				vertices = new HLVertex[vertices_count];
				for (int i = 0; i < vertices_count; ++i) {
					if (fscanf(file, "%f %f %f %*f", &vertices[i].x, &vertices[i].y, &vertices[i].z) == 0) {
						HL_LOG("ERROR: malformed mesh file. Unexpected vertex format.\n");
						goto error;
					}
				}
			// Quad indices
			} else if (strcmp(buffer, "Quadrilaterals") == 0) {
				if (fscanf(file, "%d", &quads_count) == 0 || quads != NULL) {
					HL_LOG("ERROR: malformed mesh file. Unexpected value after quads tag.\n");
					goto error;
				}
				quads = new HLQuad[quads_count];
				for (int i = 0; i < quads_count; ++i) {
					uint32_t idx[4];
					if (fscanf(file, "%d %d %d %d %*d", idx, idx + 1, idx + 2, idx + 3) == 0) {
						HL_LOG("ERROR: malformed mesh file. Unexpected quad format.\n");
						goto error;
					}
					for (int j = 0; j < 4; ++j) quads[i].idx[j] = idx[j] - 1;
				}
			// Hex indices
			} else if (strcmp(buffer, "Hexahedra") == 0) {
				if (fscanf(file, "%d", &hexas_count) == 0 || hexas != NULL) {
					HL_LOG("ERROR: malformed mesh file. Unexpected tag after hexahedras tag.\n");
					goto error;
				}
				hexas = new HLHexa[hexas_count];
				for (int i = 0; i < hexas_count; ++i) {
					uint32_t idx[8];
					if (fscanf(file, "%d %d %d %d %d %d %d %d %*d", idx, idx + 1, idx + 2, idx + 3, idx + 4, idx + 5, idx + 6, idx + 7) == 0) {
						HL_LOG("ERROR: malformed mesh file. Unexpected hexahedra format.\n");
						goto error;
					}
					for (int j = 0; j < 8; ++j) hexas[i].idx[j] = idx[j] - 1;
				}
			// End of file
			} else if (strcmp(buffer, "End") == 0) {
				break;
			// Unknown token
			} else {
				HL_LOG("ERROR: malformed mesh file. Unexpected tag.\n");
				goto error;
			}
		}

		// Make sure at least vertex and hexa index data was read
		if (vertices == NULL) {
			HL_LOG("ERROR: mesh does not containt any vertex!\n");
			goto error;
		}
		if (hexas == NULL) {
			HL_LOG("ERROR: mesh does not contain any hexa!\n");
			goto error;
		}
		if (quads == NULL) {
			HL_LOG("WARNING: mesh does not contain any quad!\n");
		}
		
		// Build the mesh and return
		this->vertices = vertices;
		this->vertices_count = vertices_count;
		this->quads = quads;
		this->quads_count = quads_count;
		this->hexas = hexas;
		this->hexas_count = hexas_count;

		return HLResult::Success;

		error:
		free(vertices);
		free(quads);
		free(hexas);
		return HLResult::Error;
	}
};

// Emscripten bindings
using namespace emscripten;

EMSCRIPTEN_BINDINGS(HLResult) {
    enum_<HLResult>("HLResult")
	.value("Success", HLResult::Success)
	.value("Error", HLResult::Error)
	;
}

EMSCRIPTEN_BINDINGS(HLMesh) {
	class_<HLMesh>("HLMesh")
    .constructor<>()
    .function("load",				&HLMesh::load)
    .function("get_vertices",		&HLMesh::get_vertices)
    .function("get_vertices_count",	&HLMesh::get_vertices_count)
    .function("get_quads",			&HLMesh::get_quads)
    .function("get_quads_count",	&HLMesh::get_quads_count)
    .function("get_hexas",			&HLMesh::get_hexas)
    .function("get_hexas_count",	&HLMesh::get_hexas_count)
    ;
}