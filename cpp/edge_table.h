#pragma once

#include "common.h"
#include "hexahedra.h"

#include <eigen/dense>

namespace HexaLab {
    using namespace Eigen;
    
    class EdgeTable {
		/*
			Hash table containing the edges. 
			Insertion takes the Vector3f position of the vertices of the edge, hashes that and linearly probes for insertion.
			The key however is not the Vector3f, but the indices inside the edge struct (storing the vertices would be a waste of space and performance).
			An empty slot has the key set to 0 (both vertex indices inside the edge are 0)
		*/
	private:
		int count;
		int capacity;
		int access_mask;
		Edge* edges;
		Vector3f* vbuffer;
	
		u32 _hash(Vector3f& a, Vector3f& b) {
			Vector3f v = a + b;
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

		EdgeTable(int capacity, Vector3f* vbuffer) {
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

		void set_vbuffer(Vector3f* vbuffer) {
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

			// in order tor esize we need to re-hash, therefore we need to get the vertices Vector3f values from the vbuffer using the indices stored inside each edge.
			for (int i = 0; i < old_capacity; ++i) {
				if (_empty(old_edges + i)) { // skip empty slots
					continue;
				}
				Vector3f& a = vbuffer[old_edges[i].vertices[0]];	// hash
				Vector3f& b = vbuffer[old_edges[i].vertices[1]];
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

			Vector3f& a = vbuffer[edge.vertices[0]];	// hash
			Vector3f& b = vbuffer[edge.vertices[1]];
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
			Vector3f& a = vbuffer[ia];	// hash
			Vector3f& b = vbuffer[ib];
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
}