#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/bind.h>

namespace HexaLab {
	using u8 = uint8_t;
	using u32 = uint32_t;
	using js_ptr = uintptr_t;

	using Index = int32_t;
	static const Index NullIndex = 0xffffffff;

	struct float3 {
		float x;
		float y;
		float z;

		float3(){}

		float3(float x, float y, float z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		float3 operator+(const float3& f3) {
			return float3(this->x + f3.x, this->y + f3.y, this->z + f3.z);
		}
	};

	float dotf3(float3& a, float3& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

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
			Next = 0,
			Prev,
		};
	};

	struct Hexa {
		Index faces[6];
		bool is_visible;
		bool is_surface;

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

	struct Quad {
		Index idx[4];
	};

	class EdgeTable {
		/*
			Hash table containing the edges. 
			Insertion takes the float3 position of the vertices of the edge, hashes that and linearly probes for insertion.
			The key however is not the float3, but the indices inside the edge struct (storing the vertices would be a waste of space and performance).
			An empty slot has the key set to 0 (both vertex indices inside the edge are 0)
		*/
	private:
		int count;
		int capacity;
		int access_mask;
		Edge* edges;
		float3* vbuffer;
	
		u32 _hash(float3& a, float3& b) {
			float3 v = a + b;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t f = (h[0] + h[1]*11 - (h[2]*17)) & 0x7fffffff;
			return (f>>22)^(f>>12)^(f);
		}

		Edge* _get(u32 hash) {
			return this->edges + (hash & this->access_mask);
		}

		Edge* _next(Edge* item) {
			if (item + 1 != this->edges + this->capacity) {
				return item + 1;
			} else {
				return this->edges;
			}
		}

		bool _empty(Edge* item) {
			return item->vertices[0] == 0 && item->vertices[1] == 0;
		}

		bool _match(Edge* edge, Index ia, Index ib) {
			return (edge->vertices[0] == ia && edge->vertices[1] == ib)
				|| (edge->vertices[0] == ib && edge->vertices[1] == ia);
		}
	public:
		EdgeTable() {
			this->count = 0;
			this->capacity = 0;
			this->access_mask = 0;
			this->edges = nullptr;
			this->vbuffer = nullptr;
		}

		EdgeTable(int capacity, float3* vbuffer) {
			this->count = 0;
			this->capacity = capacity;
			this->access_mask = capacity - 1;
			this->edges = new Edge[capacity];
			this->vbuffer = vbuffer;
		}

		~EdgeTable() {
			delete[] this->edges;
			this->edges = nullptr;
		}

		void set_vbuffer(float3* vbuffer) {
			this->vbuffer = vbuffer;
		}

		int get_count() { return this->count; }
		int get_capacity() { return this->capacity; }

		void resize(int new_capacity) {
			// keep old stuff
			Edge* old_edges = this->edges;
			int old_capacity = this->capacity;
			
			// reallocate and update stuff
			this->capacity = new_capacity;
			this->access_mask = new_capacity - 1;
			this->edges = new Edge[this->capacity];

			// in order tor esize we need to re-hash, therefore we need to get the vertices float3 values from the vbuffer using the indices stored inside each edge.
			for (int i = 0; i < old_capacity; ++i) {
				if (_empty(old_edges + i)) { // skip empty slots
					continue;
				}
				float3& a = vbuffer[old_edges[i].vertices[0]];	// hash
				float3& b = vbuffer[old_edges[i].vertices[1]];
				u32 h = _hash(a, b);
				Edge* edge = _get(h);
				while (!_empty(edge)) {	// lieanly probe for an empty slot
					edge = _next(edge);
				}
				*edge = old_edges[i];	// insert
			}

			delete[] old_edges;
		}

		void clear() {
			delete[] this->edges;
			this->edges = new Edge[this->capacity];
			this->vbuffer = nullptr;
			this->count = 0;
		}

		int insert(Edge& edge) {
			// resize when count is > than 3/4 the capacity
			if (this->count * 4 > this->capacity * 3) {
				assert(false);
				resize(this->capacity * 2);
			}

			float3& a = vbuffer[edge.vertices[0]];	// hash
			float3& b = vbuffer[edge.vertices[1]];
			u32 h = _hash(a, b);
			Edge* dest = _get(h);
			while (!_empty(dest)) { 	// lieanly probe for an empty slot
				dest = _next(dest);
			}
			*dest = edge;	// insert
			++this->count;

			return dest - this->edges;
		}

		int lookup(Index ia, Index ib) {
			float3& a = vbuffer[ia];	// hash
			float3& b = vbuffer[ib];
			u32 h = _hash(a, b);
			Edge* edge = _get(h);
			while (!_empty(edge) && !_match(edge, ia, ib)) {	// either find a match or an empty slot
				edge = _next(edge);
			}
			if (_match(edge, ia, ib)) {	// a match means a successful lookup
				return edge - this->edges;
			}
			// an empty slot means the edge isn't contained
			return -1;
		}

		Edge& get(int idx) {
			return this->edges[idx];
		}
	};

	class FaceTable {
		/*
			Hash table containing the faces. 
			Insertion takes the float3 position of the vertices defining the face, hashes that and linearly probes for insertion.
			The key however arent the float3, but the indices inside the face struct (storing the vertices would be a waste of space and performance).
			An empty slot has the key set to 0 (all 4 vertex indices inside the face are 0)
		*/
	private:
		int count;
		int capacity;
		int access_mask;
		Face* faces;
		float3* vbuffer;

		u32 _hash(float3& a, float3& b, float3& c, float3& d) {
			float3 v = a + b + c + d;
			const uint32_t* h = (const uint32_t*) &v;
			uint32_t f = (h[0] + h[1]*11 - (h[2]*17))&0x7fffffff;
			return (f>>22)^(f>>12)^(f);
		}

		Face* _get(u32 hash) {
			return this->faces + (hash & this->access_mask);
		}

		Face* _next(Face* item) {
			if (item + 1 != this->faces + this->capacity) { 
				return item + 1;
			} else {
				return this->faces;
			}
		}

		bool _empty(Face* item) {
			return item->vertices[0] == 0 && item->vertices[1] == 0 && item->vertices[2] == 0 && item->vertices[3] == 0;
		}

		bool _match(Face* face, Index ia, Index ib, Index ic, Index id) {
			return (face->vertices[0] == ia && face->vertices[1] == ib && face->vertices[2] == ic && face->vertices[3] == id);
		}

	public:
		FaceTable() {
			this->count = 0;
			this->capacity = 0;
			this->access_mask = 0;
			this->faces = nullptr;
			this->vbuffer = nullptr;
		}

		FaceTable(int capacity, float3* vbuffer) {
			this->count = 0;
			this->capacity = capacity;
			this->access_mask = capacity - 1;
			this->faces = new Face[capacity];
			this->vbuffer = vbuffer;
		}

		~FaceTable() {
			delete[] this->faces;
			this->faces = nullptr;
		}

		void set_vbuffer(float3* vbuffer) {
			this->vbuffer = vbuffer;
		}

		int get_count() { return this->count; }
		int get_capacity() { return this->capacity; }

		void resize(int new_capacity) {
			// store old stuff
			Face* old_faces = this->faces;
			int old_capacity = this->capacity;

			// allocate and set new stuff
			this->faces = new Face[new_capacity];
			this->capacity = new_capacity;
			this->access_mask = new_capacity - 1;

			for (int i = 0; i < old_capacity; ++i) {
				float3& a = vbuffer[old_faces[i].vertices[0]];	// hash
				float3& b = vbuffer[old_faces[i].vertices[1]];
				float3& c = vbuffer[old_faces[i].vertices[2]];
				float3& d = vbuffer[old_faces[i].vertices[3]];
				u32 h = _hash(a, b, c, d);
				Face* face = _get(h);
				while (!_empty(face)) {		// linearly probe for empty slot
					face = _next(face);
				}
				*face = old_faces[i];	// insert
			}

			delete[] old_faces;
		}

		void clear() {
			delete[] this->faces;
			this->faces = new Face[this->capacity];
			this->vbuffer = nullptr;
			this->count = 0;
		}

		int insert(Face& face) {
			if (this->count * 4 > this->capacity * 3) {
				assert(false);
				resize(this->capacity * 2);
			}

			float3& a = vbuffer[face.vertices[0]];	// hash
			float3& b = vbuffer[face.vertices[1]];
			float3& c = vbuffer[face.vertices[2]];
			float3& d = vbuffer[face.vertices[3]];
			u32 h = _hash(a, b, c, d);
			Face* dest = _get(h);
			while (!_empty(dest)) {		// linearly probe for empty slot
				dest = _next(dest);
			}
			*dest = face;	// insert
			++this->count;

			return dest - this->faces;
		}

		int lookup(Index ia, Index ib, Index ic, Index id) {
			float3& a = vbuffer[ia];	// hash
			float3& b = vbuffer[ib];
			float3& c = vbuffer[ic];
			float3& d = vbuffer[id];
			u32 h = _hash(a, b, c, d);
			Face* face = _get(h);
			while (!_empty(face) && !_match(face, ia, ib, ic, id)) {
				face = _next(face);
			}
			if (_match(face, ia, ib, ic, id)) {
				return face - this->faces;
			}
			return -1;
		}

		Face& get(int idx) {
			return this->faces[idx];
		}
	};

	class Plane {
		float3 normal;
		float d;

	public:
		Plane(float3& position, float3& normal) {
			this->normal = normal;
			this->d = -dotf3(position, normal);
		}

		void move(float3& position) {
			this->d = -dotf3(position, this->normal);
		}

		void orient(float3& normal) {
			this->normal = normal;			
		}

		int solve(float3& point) {
			return dotf3(this->normal, point) + this->d;
		}
	};

	enum class Result : u8 {
		Success,
		Error
	};

	#define HL_LOG(...) printf(__VA_ARGS__)

	class Mesh {
	private:
		float3* vbuffer;
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

		void insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index f) {
			Index e = edges.lookup(indices[v1], indices[v2]);
			if (e != -1) {
				Edge& edge = edges.get(e);
				Face& face = faces.get(f);
				edge.faces[edge.f_count++] = f;
				if (edge.f_count > 8) {
					int qwe = 1;
				}
				face.edges[edge_enum] = e;
			} else {
				Edge edge;
				edge.vertices[0] = indices[v1];
				edge.vertices[1] = indices[v2];
				edge.faces[edge.f_count++] = f;
				faces.get(f).edges[edge_enum] = e = edges.insert(edge);

				Vertex& a = vertices[indices[v1]];
				a.edges[a.e_count++] = e;
				if (a.e_count > 8) {
					int qwe = 1;
				}

				Vertex& b = vertices[indices[v2]];
				b.edges[b.e_count++] = e;
				if (b.e_count > 8) {
					int qew  =1;
				}
			}
		}

		void insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum) {
			Index f = faces.lookup(indices[v1], indices[v2], indices[v3], indices[v4]);
			if (f != -1) {
				assert(faces.get(f).hexas[hexa_enum] == -1);
				faces.get(f).hexas[hexa_enum] = hexas_count;
				hexas[hexas_count].faces[face_enum] = f;
			} else {
				Face face;
				face.vertices[0] = indices[v1];
				face.vertices[1] = indices[v2];
				face.vertices[2] = indices[v3];
				face.vertices[3] = indices[v4];
				face.hexas[hexa_enum] = hexas_count;
				hexas[hexas_count].faces[face_enum] = f = faces.insert(face);
			
				insert_edge(indices, v1, v2, Face::Edge::Top, f);
				insert_edge(indices, v2, v3, Face::Edge::Left, f);
				insert_edge(indices, v3, v4, Face::Edge::Bottom, f);
				insert_edge(indices, v4, v1, Face::Edge::Right, f);
			}

			
		}

		void insert_hexa(Index* indices) {
			insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::NearTopLeft, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearBotRight, Face::Hexa::Next, Hexa::Face::Near);
			insert_face(indices, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarBotRight, Face::Hexa::Prev, Hexa::Face::Far);
			insert_face(indices, Hexa::Vertex::NearTopLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::NearBotLeft, Face::Hexa::Next, Hexa::Face::Left);
			insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarBotRight, Hexa::Vertex::NearBotRight, Face::Hexa::Prev, Hexa::Face::Right);
			insert_face(indices, Hexa::Vertex::NearBotRight, Hexa::Vertex::NearBotLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarBotRight, Face::Hexa::Next, Hexa::Face::Bottom);
			insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::NearTopLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarTopRight, Face::Hexa::Prev, Hexa::Face::Top);

			++hexas_count;
		}

	public:
		Mesh() {
			this->vbuffer = nullptr;
			this->vertices = nullptr;
			this->vertices_count = 0;
			this->ibuffer = nullptr;
			this->indices_count = 0;
			this->hexas = nullptr;
			this->hexas_count = 0;
			this->quads = nullptr;
			this->quads_count = 0;
		}

		~Mesh() {
			delete[] this->vbuffer;
			this->vbuffer = nullptr;
			delete[] this->vertices;
			this->vertices = nullptr;
			this->vertices_count = 0;

			delete[] this->ibuffer;
			this->ibuffer = nullptr;
			this->indices_count = 0;

			delete[] this->hexas;
			this->hexas = nullptr;
			this->hexas_count = 0;

			delete[] this->quads;
			this->quads = nullptr;
			this->quads_count = 0;
		}

		js_ptr get_vbuffer() { return (js_ptr) this->vbuffer; }
		int get_vertices_count() { return this->vertices_count; }

		js_ptr get_ibuffer() { return (js_ptr) this->ibuffer; }
		int get_indices_count() { return this->indices_count; }

		void make_ibuffer() {
			delete[] this->ibuffer;
			this->ibuffer = new Index[this->vertices_count];
			int count = 0, capacity = vertices_count;
			HL_LOG("Building visible ibuffer...\n");
			for (int h = 0; h < this->hexas_count; ++h) {
				for (int f = 0; f < 6; ++f) {
					Face& face = this->faces.get(this->hexas[h].faces[f]);
					if (face.hexas[Face::Hexa::Next] == -1 || face.hexas[Face::Hexa::Prev] == -1) {
						// resize check
						if (count + 6 > capacity) {
							Index* old = this->ibuffer;
							this->ibuffer = new Index[capacity * 2];
							memcpy(this->ibuffer, old, sizeof(Index) * count);
							delete[] old;
							capacity *= 2;
						}

						if (face.hexas[Face::Hexa::Prev] != -1) {	// CCW winding
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotRight];
						} else {									// CW winding
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
						}
						
					}
				}
			}
			this->indices_count = count;
			HL_LOG("%d vertex indices, %d visible faces (%d triangles)\n", count, count / 6, count / 3);
		}

		void make_ibuffer(Plane* plane) {
			delete[] this->ibuffer;
			this->ibuffer = new Index[this->vertices_count];
			int count = 0, capacity = vertices_count;
			HL_LOG("Building visible ibuffer...\n");
			for (int h = 0; h < this->hexas_count; ++h) {
				for (int f = 0; f < 6; ++f) {
					Face& face = this->faces.get(this->hexas[h].faces[f]);
					if (face.hexas[Face::Hexa::Next] == -1 || face.hexas[Face::Hexa::Prev] == -1) {
						// resize check
						if (count + 6 > capacity) {
							Index* old = this->ibuffer;
							this->ibuffer = new Index[capacity * 2];
							memcpy(this->ibuffer, old, sizeof(Index) * count);
							delete[] old;
							capacity *= 2;
						}

						if (face.hexas[Face::Hexa::Prev] != -1) {	// CCW winding
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotRight];
						} else {									// CW winding
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotRight];
							this->ibuffer[count++] = face.vertices[Face::Vertex::BotLeft];
							this->ibuffer[count++] = face.vertices[Face::Vertex::TopRight];
						}
						
					}
				}
			}
			this->indices_count = count;
			HL_LOG("%d vertex indices, %d visible faces (%d triangles)\n", count, count / 6, count / 3);
		}

		Result load(std::string filename) { 
			FILE* file = fopen(filename.c_str(), "rb");
			if (file == NULL) {
				HL_LOG("Unable to find file %s!\n", filename.c_str());
				return Result::Error;
			}

			char buffer[100];
			int precision;
			int dimension;

			delete[] this->vbuffer;
			this->vbuffer = nullptr;
			delete[] this->vertices;
			this->vertices = nullptr;
			this->vertices_count = 0;

			delete[] this->ibuffer;
			this->ibuffer = nullptr;
			this->indices_count = 0;

			delete[] this->hexas;
			this->hexas = nullptr;
			this->hexas_count = 0;

			delete[] this->quads;
			this->quads = nullptr;
			this->quads_count = 0;

			if (this->edges.get_count() > 0) {
				this->edges.clear();				
			}
			if (this->faces.get_count() > 0) {
				this->faces.clear();
			}
			
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
					if (fscanf(file, "%d", &this->vertices_count) == 0) {
						HL_LOG("ERROR: malformed mesh file. Unexpected value after vertices tag.\n");
						goto error;
					}
					if (this->vbuffer != nullptr) {
						HL_LOG("ERROR: found two separate vertex lists?\n");
						goto error;
					}
					HL_LOG("Reading %d vertices...\n", this->vertices_count);
					this->vbuffer = new float3[this->vertices_count];
					for (int i = 0; i < this->vertices_count; ++i) {
						if (fscanf(file, "%f %f %f %*f", &this->vbuffer[i].x, &this->vbuffer[i].y, &this->vbuffer[i].z) == 0) {
							HL_LOG("ERROR: malformed mesh file. Unexpected vertex format.\n");
							goto error;
						}
					}
				// Quad indices
				} else if (strcmp(buffer, "Quadrilaterals") == 0) {
					if (fscanf(file, "%d", &quads_count) == 0 || quads != nullptr) {
						HL_LOG("ERROR: malformed mesh file. Unexpected value after quads tag.\n");
						goto error;
					}
					HL_LOG("Reading %d quads... (unused)\n", quads_count);
					quads = new Quad[quads_count];
					for (int i = 0; i < quads_count; ++i) {
						Index idx[4];
						if (fscanf(file, "%d %d %d %d %*d", idx, idx + 1, idx + 2, idx + 3) == 0) {
							HL_LOG("ERROR: malformed mesh file. Unexpected quad format.\n");
							goto error;
						}
						for (int j = 0; j < 4; ++j) quads[i].idx[j] = idx[j] - 1;
					}
				// Hex indices
				} else if (strcmp(buffer, "Hexahedra") == 0) {
					int total_hexas_count;
					if (fscanf(file, "%d", &total_hexas_count) == 0) {
						HL_LOG("ERROR: malformed mesh file. Unexpected tag after hexahedras tag.\n");
						goto error;
					}
					if (hexas != nullptr) {
						HL_LOG("ERROR: found two separate hexas lists?\n");
						goto error;
					}
					if (vbuffer == nullptr) {
						HL_LOG("ERROR: hexas list found before vertex list!\n");
						goto error;
					}
					HL_LOG("Reading %d hexas...\n", total_hexas_count);
					hexas = new Hexa[total_hexas_count];
					faces.set_vbuffer(this->vbuffer);
					faces.resize(1 << 19);	// ~500k faces
					edges.set_vbuffer(this->vbuffer);
					edges.resize(1 << 20);	// ~1kk edges
					vertices = new Vertex[vertices_count];	
					Index f, e;
					Vertex* v;
					for (int h = 0; h < total_hexas_count; ++h) {
						Index idx[8];
						if (fscanf(file, "%d %d %d %d %d %d %d %d %*d", idx, idx + 1, idx + 2, idx + 3, idx + 4, idx + 5, idx + 6, idx + 7) == 0) {
							HL_LOG("ERROR: malformed mesh file. Unexpected hexahedra format.\n");
							goto error;
						}
						for (int j = 0; j < 8; ++j) idx[j] = idx[j] - 1;
						insert_hexa(idx);
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

			fclose(file);

			// Make sure at least vertex and hexa index data was read
			if (vertices == nullptr) {
				HL_LOG("ERROR: mesh does not containt any vertex!\n");
				goto error;
			}
			if (hexas == nullptr) {
				HL_LOG("ERROR: mesh does not contain any hexa!\n");
				goto error;
			}

			HL_LOG("%d edges total\n", edges.get_count());
			HL_LOG("%d faces total\n", faces.get_count());

			return Result::Success;

			error:
			fclose(file);

			delete[] this->vbuffer;
			this->vbuffer = nullptr;
			delete[] this->vertices;
			this->vertices = nullptr;
			this->vertices_count = 0;

			delete[] this->ibuffer;
			this->ibuffer = nullptr;
			this->indices_count = 0;
			
			delete[] this->hexas;
			this->hexas = nullptr;
			this->hexas_count = 0;

			delete[] this->quads;
			this->quads = nullptr;
			this->quads_count = 0;

			return Result::Error;
		}

	};
}

/*
int main() {
	using namespace HexaLab;
	Mesh m;
	m.load("C:/Code/HexaLab/data/doubletorus.mesh");
	m.make_ibuffer();
	m.load("C:/Code/HexaLab/data/block.mesh");
	m.make_ibuffer();
}
*/

// Emscripten bindings

using namespace emscripten;

EMSCRIPTEN_BINDINGS(Result) {
    enum_<HexaLab::Result>("Result")
	.value("Success", HexaLab::Result::Success)
	.value("Error", HexaLab::Result::Error)
	;
}

EMSCRIPTEN_BINDINGS(Mesh) {
	class_<HexaLab::Mesh>("Mesh")
    .constructor<>()
    .function("load",				&HexaLab::Mesh::load)
    .function("get_vbuffer",		&HexaLab::Mesh::get_vbuffer)
    .function("get_vertices_count",	&HexaLab::Mesh::get_vertices_count)
    .function("get_ibuffer",		&HexaLab::Mesh::get_ibuffer)
    .function("get_indices_count",	&HexaLab::Mesh::get_indices_count)
    .function("make_ibuffer",		&HexaLab::Mesh::make_ibuffer)
    ;
}
