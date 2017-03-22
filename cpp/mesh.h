#pragma once

#include "common.h"
#include "edge_table.h"
#include "face_table.h"

#include <float.h>

#include <eigen/dense>
#include <eigen/geometry>

namespace HexaLab {
    using namespace Eigen;

	struct Quad {
		Index idx[4];
	};

    // -------------------------------------------------------------------------------------
    // Mesh class. Contains the vertex/index buffers, along with a few table that
    // cross-reference each other to index the mesh structure:
    //     vertices:
    //         each element is in the same index as its corresponding position in the vbuffer.
    //         each element stores the indices of all the edges having him as a vertex.
    //     edges:
    //         they are stored sparse in a hash table. Hashing them by their position
    //         allows to quickly look up an edge during mesh construction.
    //         each element stores the indices of its vertices and the indices of its faces.
    //     faces:
    //         they are also stored sparse in a hash table, for the same reason (quick lookup
    //         during mesh construction).
    //         each element stores the the indices of its vertices, the indices of its edges and
    //         the indices of the two hexahedras having him as a face.
    //         The vertex indices are redundant and could be retrieved anyway with a double
    //         indirection. They are cached here, too, to avoid that indirection (they also fit
    //         nicely, making the size of a face exactly 32byte).
    //     hexas:
    //         they are stored densely, allocated linearly in the same order they're read from
    //         the mesh file.
    //         each hexa stores the indices to its faces and some additional properties.
    // The main two APIs of the class are load, which reads a mesh file and builds the mesh
    // and all the data structures previously mentioned, and make_ibuffer, which rebuilds
    // an ibuffer of the mesh surface from the mesh's elements properties and adjacencies.
    
	class Mesh {
	public:
		int max_f_count = 0;
		int max_e_count = 0;
	private:
		Vector3f* vbuffer;
		Vertex* vertices;
		int vertices_count;
		Index* ibuffer;
		int indices_count;
		Hexa* hexas;
		int hexas_count;
		Quad* quads;	// Not always available
		int quads_count;
		EdgeTable edges;
		FaceTable faces;
		AlignedBox3f aabb;

        // Inserts one edge of one hexa.
		void insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index f);

        // Inserts one face of one hexa. It is assumed that the face belongs to the last hexa, TODO fix this
		void insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum);

        // Inserts a new hexahedra into the mesh, from the indices of the hexa's vertices.
		void insert_hexa(Index* indices);

	public:
		Mesh();
		~Mesh();

		js_ptr get_vbuffer() { return (js_ptr) this->vbuffer; }
		int get_vertices_count() { return this->vertices_count; }
		js_ptr get_ibuffer() { return (js_ptr) this->ibuffer; }
		int get_indices_count() { return this->indices_count; }

		float get_center_x() { return this->aabb.center().x(); }
		float get_center_y() { return this->aabb.center().y(); }
		float get_center_z() { return this->aabb.center().z(); }
        Vector3f get_center() { return this->aabb.center(); }
		float get_diagonal_size() { return this->aabb.diagonal().norm(); }

        // Build a new ibuffer by reading the mesh data structures.
		void make_ibuffer(Hyperplane<float, 3>* plane);

        // Loads the mesh from a mesh file. 
        // The mesh file parser is not complete, but it's good enough for the purpose. 
		Result load(std::string filename);
	};
}
