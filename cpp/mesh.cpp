// Header
#include "mesh.h"

// STD
#include <string.h>

// STL
#include <string>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    std::vector<Vector3f>* g_vbuffer = nullptr;

    Mesh::Mesh() {
    }

    Mesh::~Mesh() {
    }

    // Insertion is basically lookup inside the edges hash table -> found: update fields,
    // not found: insert the edge into the table, update fields, and also insert the new edge's vertices
    void Mesh::insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index f) {
        Index e = -1;
        EdgeID fake;
        fake.vertices[0] = indices[v1];
        fake.vertices[1] = indices[v2];
        auto search = edges_set.find(fake);
        if (search != edges_set.end()) {
            e = search->edge_idx;
        }
        if (e != -1) {
            Edge& edge = edges[e];
            Face& face = faces[f];
            edge.faces[edge.f_count++] = f;
            if (edge.f_count > max_f_count) {
                max_f_count = edge.f_count;
            }
            face.edges[edge_enum] = e;
        } else {
            Edge edge;
            edge.vertices[0] = indices[v1];
            edge.vertices[1] = indices[v2];
            edge.faces[edge.f_count++] = f;
            faces[f].edges[edge_enum] = e = edges.size();
            edges.push_back(edge);

            EdgeID eid;
            eid.edge_idx = edges.size() - 1;
            eid.vertices[0] = edge.vertices[0];
            eid.vertices[1] = edge.vertices[1];
            edges_set.insert(eid);

            Vertex& a = vertices[indices[v1]];
            a.edges[a.e_count++] = e;
            if (a.e_count > max_e_count) {
                max_e_count = a.e_count;
            }

            Vertex& b = vertices[indices[v2]];
            b.edges[b.e_count++] = e;
            if (b.e_count > max_e_count) {
                max_e_count = b.e_count;
            }
        }
    }

    // It is assumed that the face belongs to the last hexa, TODO fix this
    // Insertion is basically lookup inside the faces hash table -> found: update fields,
    // not found: insert the face into the table, update fields, and also insert the new face's edges
    void Mesh::insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum, Index h) {
        Index f = -1;
        FaceID fake;
        fake.vertices[0] = indices[v1];
        fake.vertices[1] = indices[v2];
        fake.vertices[2] = indices[v3];
        fake.vertices[3] = indices[v4];
        auto search = faces_set.find(fake);
        if (search != faces_set.end()) {
            f = search->face_idx;
        }
        if (f != -1) {
            assert(faces[f].hexas[hexa_enum] == -1);
            faces[f].hexas[hexa_enum] = h;
            hexas[h].faces[face_enum] = f;
            this->hexas[h].is_surface = (faces[f].hexas[0] == -1 || faces[f].hexas[1] == -1);
        } else {
            Face face;
            face.vertices[0] = indices[v1];
            face.vertices[1] = indices[v2];
            face.vertices[2] = indices[v3];
            face.vertices[3] = indices[v4];
            face.hexas[hexa_enum] = h;
            hexas[h].faces[face_enum] = f = faces.size();
            faces.push_back(face);
            
            FaceID fid;
            fid.face_idx = faces.size() - 1;
            fid.vertices[0] = face.vertices[0];
            fid.vertices[1] = face.vertices[1];
            fid.vertices[2] = face.vertices[2];
            fid.vertices[3] = face.vertices[3];
            faces_set.insert(fid);
            
            this->hexas[h].is_surface = (faces[f].hexas[0] == -1 || faces[f].hexas[1] == -1);
        
            insert_edge(indices, v1, v2, Face::Edge::Top, f);
            insert_edge(indices, v2, v3, Face::Edge::Left, f);
            insert_edge(indices, v3, v4, Face::Edge::Bottom, f);
            insert_edge(indices, v4, v1, Face::Edge::Right, f);
        }
    }

    // All the noise in the insert_face call could be removed and reduced to only 1 parameter
    // which would then need to be switched upon, since the links are all 1:1, but a non-predictable
    // switch called this frequent might actually hurt a bit, so all parameters are hard-coded.
    // The hexa is by default visible, the property will then be modified upon insertion of
    // its components, if that's the case.
    void Mesh::insert_hexa(Index* indices) {
        this->hexas.emplace_back();
        Hexa& hexa = this->hexas.back();
        Index h = this->hexas.size() - 1;
        hexa.is_surface = 1;

        insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::NearTopLeft, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearBotRight, Face::Hexa::Back, Hexa::Face::Near, h);
        insert_face(indices, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarBotRight, Face::Hexa::Front, Hexa::Face::Far, h);
        insert_face(indices, Hexa::Vertex::NearTopLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::NearBotLeft, Face::Hexa::Back, Hexa::Face::Left, h);
        insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarBotRight, Hexa::Vertex::NearBotRight, Face::Hexa::Front, Hexa::Face::Right, h);
        insert_face(indices, Hexa::Vertex::NearBotRight, Hexa::Vertex::NearBotLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarBotRight, Face::Hexa::Back, Hexa::Face::Bottom, h);
        insert_face(indices, Hexa::Vertex::NearTopRight, Hexa::Vertex::NearTopLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarTopRight, Face::Hexa::Front, Hexa::Face::Top, h);
    }

    // Build a new ibuffer by reading the mesh data structures.
    void Mesh::make_ibuffer(Hyperplane<float, 3>* plane) {
        this->ibuffer.clear();
        HL_LOG("Building visible ibuffer...\n");
        for (int h = 0; h < this->hexas.size(); ++h) {
            // Test for plane culling.
            // If the hexa is culled mark its corresponding flag.
            bool culled = false;
            if (plane != nullptr) {
                Face& near_face = this->faces[this->hexas[h].faces[Hexa::Face::Near]];
                for (int i = 0; i < 4; ++i) {
                    if (plane->signedDistance(this->vbuffer[near_face.vertices[i]]) < 0) {
                        this->hexas[h].is_culled = culled = true;
                        break;
                    }
                }
                if (!culled) {
                    Face& far_face = this->faces[this->hexas[h].faces[Hexa::Face::Far]];
                    for (int i = 0; i < 4; ++i) {
                        if (plane->signedDistance(this->vbuffer[far_face.vertices[i]]) < 0) {
                            this->hexas[h].is_culled = culled = true;
                            break;
                        }
                    }
                }
            }
            // If it is culled, skip directly to the next hexa
            if (!culled) this->hexas[h].is_culled = false;
            else continue;

            // Test for occlusion.
            // Obviously not a real occlusion test: if all faces of the hexa
            // have another adjacent hexa, it gets culled.
            for (int f = 0; f < 6; ++f) {
                Face& face = this->faces[this->hexas[h].faces[f]];
                bool front_face = !hexas[face.hexas[Face::Hexa::Back]].is_culled && (face.hexas[Face::Hexa::Front] == -1 || hexas[face.hexas[Face::Hexa::Front]].is_culled);
                bool back_face = !hexas[face.hexas[Face::Hexa::Front]].is_culled && (face.hexas[Face::Hexa::Back] == -1 || hexas[face.hexas[Face::Hexa::Back]].is_culled);
                if (front_face || back_face) {
                    // When inserting inside the ibuffer, windiwng order needs to be taken into account.
                    if (back_face) {		// CCW winding
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopRight]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopRight]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotRight]);
                    } else { //if (back_face) {				// CW winding
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopRight]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotRight]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::BotLeft]);
                        this->ibuffer.push_back(face.vertices[Face::Vertex::TopRight]);
                    }
                    
                }
            }
        }
        HL_LOG("%d vertex indices, %d visible faces (%d triangles)\n", this->ibuffer.size(), this->ibuffer.size() / 6, this->ibuffer.size() / 3);
    }

    // Loads the mesh from a mesh file. 
    // The mesh file parser is not complete, but it's good enough for the purpose. 
    Result Mesh::load(std::string filename) { 
        FILE* file = fopen(filename.c_str(), "rb");
        if (file == NULL) {
            HL_LOG("Unable to find file %s!\n", filename.c_str());
            return Result::Error;
        }

        char buffer[100];
        int precision;
        int dimension;

        this->hexas.clear();
        this->edges.clear();
        this->faces.clear();
        this->vertices.clear();
        this->vbuffer.clear();
        this->quads.clear();
        
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
                int vertices_count;
                if (fscanf(file, "%d", &vertices_count) == 0) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after vertices tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d vertices...\n", vertices_count);
                this->vbuffer.reserve(vertices_count);
                for (int i = 0; i < vertices_count; ++i) {
                    Vector3f v;
                    if (fscanf(file, "%f %f %f %*f", &v.x(), &v.y(), &v.z()) == 0) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected vertex format.\n");
                        goto error;
                    }
                    this->vbuffer.push_back(v);
                    this->aabb.extend(v);
                }
                g_vbuffer = &this->vbuffer;
            // Quad indices
            } else if (strcmp(buffer, "Quadrilaterals") == 0) {
                int quads_count;
                if (fscanf(file, "%d", &quads_count) == 0) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after quads tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d quads... (unused)\n", quads_count);
                for (int i = 0; i < quads_count; ++i) {
                    Quad q;
                    if (fscanf(file, "%d %d %d %d %*d", &q.idx[0], &q.idx[1], &q.idx[2], &q.idx[3]) == 0) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected quad format.\n");
                        goto error;
                    }
                    for (int j = 0; j < 4; ++j) q.idx[j] = q.idx[j] - 1;
                    this->quads.push_back(q);
                }
            // Hex indices
            } else if (strcmp(buffer, "Hexahedra") == 0) {
                int hexas_count;
                if (fscanf(file, "%d", &hexas_count) == 0) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected tag after hexahedras tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d hexas...\n", hexas_count);
                vertices.resize(vbuffer.size());
                edges_set.reserve(1<<20);
                edges.reserve(1<<20);
                faces_set.reserve(1<<19);
                faces.reserve(1<<19);
                for (int h = 0; h < hexas_count; ++h) {
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
        if (this->vertices.size() == 0) {
            HL_LOG("ERROR: mesh does not containt any vertex!\n");
            goto error;
        }
        if (this->hexas.size() == 0) {
            HL_LOG("ERROR: mesh does not contain any hexa!\n");
            goto error;
        }

        HL_LOG("%d edges total\n", edges.size());
        HL_LOG("%d faces total\n", faces.size());

        HL_LOG("max e_count:%d, max f_count:%d\n", this->max_e_count, this->max_f_count);

        return Result::Success;

        error:
        fclose(file);

        return Result::Error;
    }
}
