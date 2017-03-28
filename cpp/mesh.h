#ifndef _HL_MESH_H_
#define _HL_MESH_H_

#include "common.h"
#include "hexahedra.h"
#include <unordered_map>
#include <vector>
#include <eigen/dense>
#include <eigen/geometry>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;
    
	class Mesh {
	private:
        //vector<Vector3f> vbuffer;
        vector<Index> ibuffer;
        vector<Hexa> hexas;
        vector<Face> faces;
        vector<Edge> edges;
        vector<Vertex> vertices;
        vector<Dart> darts;
		AlignedBox3f aabb;

        std::unordered_map<IndexPair, Index> edges_set;
        std::unordered_map<IndexQuad, Index> faces_set;

        // Inserts one edge of one hexa.
		void insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index h, Index f);

        // Inserts one face of one hexa. It is assumed that the face belongs to the last hexa, TODO fix this
		void insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum, Index h);

        // Inserts a new hexahedra into the mesh, from the indices of the hexa's vertices.
		void insert_hexa(Index* indices);

	public:
		//js_ptr get_vbuffer() { return (js_ptr) this->vbuffer.data(); }
		int get_vertices_count() { return this->vertices.size(); }
		js_ptr get_ibuffer() { return (js_ptr) this->ibuffer.data(); }
		int get_indices_count() { return this->ibuffer.size(); }

		float get_center_x() { return this->aabb.center().x(); }
		float get_center_y() { return this->aabb.center().y(); }
		float get_center_z() { return this->aabb.center().z(); }
		float get_diagonal_size() { return this->aabb.diagonal().norm(); }

        // Build a new ibuffer by reading the mesh data structures.
		void make_ibuffer(Hyperplane<float, 3>* plane);

        // Loads the mesh from a mesh file. 
        // The mesh file parser is not complete, but it's good enough for the purpose. 
		Result load(std::string filename);

        // Clears the mesh from any previously loaded data
        void unload();

        void test() {
            for (int i = 0; i < 48; ++i) {
                const Dart& a = darts[i];

                const Dart& f1 = darts[a.face_neighbor];
                const Dart& f2 = darts[f1.face_neighbor];
                assert(a.hexa == f2.hexa
                    && a.face == f2.face
                    && a.edge == f2.edge
                    && a.vertex == f2.vertex);

                const Dart& e1 = darts[a.edge_neighbor];
                const Dart& e2 = darts[e1.edge_neighbor];
                assert(a.hexa == e2.hexa
                    && a.face == e2.face
                    && a.edge == e2.edge
                    && a.vertex == e2.vertex);

                const Dart& v1 = darts[a.edge_neighbor];
                const Dart& v2 = darts[v1.edge_neighbor];
                assert(a.hexa == v2.hexa
                    && a.face == v2.face
                    && a.edge == v2.edge
                    && a.vertex == v2.vertex);

                    
            }
        }
	};
}

#endif