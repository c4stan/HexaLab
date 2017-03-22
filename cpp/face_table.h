#pragma once

#include "common.h"
#include "hexahedra.h"

#include <eigen/dense>

namespace HexaLab {
    using namespace Eigen;

    class FaceTable {
		/*
			Hash table containing the faces. 
			Insertion takes the Vector3f position of the vertices defining the face, hashes that and linearly probes for insertion.
			The key however arent the Vector3f, but the indices inside the face struct (storing the vertices would be a waste of space and performance).
			An empty slot has the key set to 0 (all 4 vertex indices inside the face are 0)
		*/
	private:
		int count;
		int capacity;
		int access_mask;
		Face* faces;
		Vector3f* vbuffer;

		u32 _hash(Vector3f& a, Vector3f& b, Vector3f& c, Vector3f& d) {
			Vector3f v = a + b + c + d;
			const uint32_t* h = (const uint32_t*) &v[0];
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

		FaceTable(int capacity, Vector3f* vbuffer) {
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

		void set_vbuffer(Vector3f* vbuffer) {
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
				Vector3f& a = vbuffer[old_faces[i].vertices[0]];	// hash
				Vector3f& b = vbuffer[old_faces[i].vertices[1]];
				Vector3f& c = vbuffer[old_faces[i].vertices[2]];
				Vector3f& d = vbuffer[old_faces[i].vertices[3]];
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

			Vector3f& a = vbuffer[face.vertices[0]];	// hash
			Vector3f& b = vbuffer[face.vertices[1]];
			Vector3f& c = vbuffer[face.vertices[2]];
			Vector3f& d = vbuffer[face.vertices[3]];
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
			Vector3f& a = vbuffer[ia];	// hash
			Vector3f& b = vbuffer[ib];
			Vector3f& c = vbuffer[ic];
			Vector3f& d = vbuffer[id];
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
}