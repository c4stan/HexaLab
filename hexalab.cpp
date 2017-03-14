#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

struct HLVertex {
	float x;
	float y;
	float z;
};

typedef uint32_t HLIndex;

struct HLQuad {
	HLIndex face[4];
};

struct HLHexa {
	HLIndex volume[8];
};

struct HLMesh {
	HLVertex* vertices;
	HLQuad* quads;
	HLHexa* hexas;
	uint32_t vertices_count;
	uint32_t quads_count;
	uint32_t hexas_count;
};

enum HLResult {
	HL_SUCCESS,
	HL_ERROR
};

#define HL_LOG(...) printf(__VA_ARGS__)

extern "C" 
{

EMSCRIPTEN_KEEPALIVE
void hello_fun() {
	printf("hello, world!\n");
}

EMSCRIPTEN_KEEPALIVE
HLResult load_mesh(HLMesh* mesh, const char* filename) { 
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		HL_LOG("Unable to find file %s!\n", filename);
		return HL_ERROR;
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
			HL_LOG("ERROR: malformed mesh file.\n");
			goto error;
		}

		// Precision
		if (strcmp(buffer, "MeshVersionFormatted") == 0) {
			if (fscanf(file, "%d", &precision) == 0) {
				HL_LOG("ERROR: malformed mesh file.\n");
				goto error;
			}
		// Dimension
		} else if (strcmp(buffer, "Dimension") == 0) {
			if (fscanf(file, "%d", &dimension) == 0) {
				HL_LOG("ERROR: malformed mesh file.\n");
				goto error;
			}
		// Vertices
		} else if (strcmp(buffer, "Vertices") == 0) {
			if (fscanf(file, "%d", &vertices_count) != 0 || vertices != NULL) {
				HL_LOG("ERROR: malformed mesh file.\n");
				goto error;
			}
			vertices = new HLVertex[vertices_count];
			for (int i = 0; i < vertices_count; ++i) {
				if (fscanf(file, "%f %f %f %*f", &vertices[i].x, &vertices[i].y, &vertices[i].z) == 0) {
					HL_LOG("ERROR: malformed mesh file.\n");
					goto error;
				}
			}
		// Quad indices
		} else if (strcmp(buffer, "Quadrilaterals") == 0) {
			if (fscanf(file, "%d", &quads_count) != 0 || quads != NULL) {
				HL_LOG("ERROR: malformed mesh file.\n");
				goto error;
			}
			quads = new HLQuad[quads_count];
			for (int i = 0; i < quads_count; ++i) {
				if (fscanf(file, "%d %d %d %d %*d",
					&quads[i].face[0], &quads[i].face[1], &quads[i].face[2], &quads[i].face[3]) == 0) {
					HL_LOG("ERROR: malformed mesh file.\n");
					goto error;
				}
			}
		// Hex indices
		} else if (strcmp(buffer, "Hexahedra") == 0) {
			if (fscanf(file, "%d", &hexas_count) != 0 || hexas != NULL) {
				HL_LOG("ERROR: malformed mesh file.\n");
				goto error;
			}
			hexas = new HLHexa[hexas_count];
			for (int i = 0; i < hexas_count; ++i) {
				if (fscanf(file, "%d %d %d %d %d %d %d %d %*d", 
					&hexas[i].volume[0], &hexas[i].volume[1], &hexas[i].volume[2], &hexas[i].volume[3],
					&hexas[i].volume[4], &hexas[i].volume[5], &hexas[i].volume[6], &hexas[i].volume[7]) == 0) {
					HL_LOG("ERROR: malformed mesh file.\n");
					goto error;
				}
			}
		// End of file
		} else if (strcmp(buffer, "End") == 0) {
			break;
		}
	}

	// Make sure at least vertex and hexa index data was read
	if (vertices == NULL || hexas == NULL) {
		HL_LOG("ERROR: malformed mesh file.\n");
		goto error;
	}

	// Build the mesh and return
	mesh->vertices = vertices;
	mesh->vertices_count = vertices_count;
	mesh->quads = quads;
	mesh->quads_count = quads_count;
	mesh->hexas = hexas;
	mesh->hexas_count = hexas_count;

	return HL_SUCCESS;

	error:
	free(vertices);
	free(quads);
	free(hexas);
	return HL_ERROR;
}
}