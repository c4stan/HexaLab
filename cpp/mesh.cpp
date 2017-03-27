// Header
#include "mesh.h"

// STD
#include <string.h>

// STL
#include <string>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    //std::vector<Vector3f>* g_vbuffer = nullptr;

    Mesh::Mesh() {
    }

    Mesh::~Mesh() {
    }

    // Insertion is basically lookup inside the edges hash table -> found: update fields,
    // not found: insert the edge into the table, update fields, and also insert the new edge's vertices
    void Mesh::insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index h, Index f) {
        Index e = -1;
        EdgeID key;
        key.vertices[0] = indices[v1];
        key.vertices[1] = indices[v2];
        auto search = edges_set.find(key);
        if (search != edges_set.end()) {
            e = search->second;
        }
        if (e != -1) {
            //Edge& edge = edges[e];
            //Face& face = faces[f];
            //edge.faces[edge.f_count++] = f;
            //if (edge.f_count > max_f_count) {
            //    max_f_count = edge.f_count;
            //}
            //face.edges[edge_enum] = e;
        } else {
            Edge edge;
            //edge.vertices[0] = indices[v1];
            //edge.vertices[1] = indices[v2];
            //edge.faces[edge.f_count++] = f;
            //faces[f].edges[edge_enum] = e = edges.size();
            edge.dart = this->darts.size();
            edges.push_back(edge);
            e = edges.size() - 1;

            EdgeID eid;
            eid.vertices[0] = indices[v1];
            eid.vertices[1] = indices[v2];
            std::pair<EdgeID, Index> keyval(eid, e);
            edges_set.insert(keyval);
        }

        // Allocate one vertex
        Vertex& a = vertices[indices[v1]];
        if (a.dart == -1)
            a.dart = this->darts.size();

        // Allocate a dart on that vertex
        this->darts.emplace_back();
        Dart& d1 = this->darts.back();
        d1.hexa = h;
        d1.face = f;
        d1.edge = e;
        d1.vertex = indices[v1];

        // Allocate the other vertex
        Vertex& b = vertices[indices[v2]];
        if (b.dart == -1)
            b.dart = this->darts.size();

        // Allocate a dart on that vertex
        this->darts.emplace_back();
        Dart& d2 = this->darts.back();
        d2.hexa = h;
        d2.face = f;
        d2.edge = e;
        d2.vertex = indices[v2];

        // Link darts
        darts[darts.size() - 1].vertex_neighbor = darts.size() - 2;
        darts[darts.size() - 2].vertex_neighbor = darts.size() - 1;
    }

    Hexa::Face opposite_face(Hexa::Face face) {
        switch (face) {
        case Hexa::Face::Near:
            return Hexa::Face::Far;
        case Hexa::Face::Far:
            return Hexa::Face::Near;
        case Hexa::Face::Right:
            return Hexa::Face::Left;
        case Hexa::Face::Left:
            return Hexa::Face::Right;
        case Hexa::Face::Top:
            return Hexa::Face::Bottom;
        case Hexa::Face::Bottom:
            return Hexa::Face::Top;
        }
        // never reached
        assert(false);
        return Hexa::Face::Near; 
    }

    // It is assumed that the face belongs to the last hexa, TODO fix this
    // Insertion is basically lookup inside the faces hash table -> found: update fields,
    // not found: insert the face into the table, update fields, and also insert the new face's edges
    void Mesh::insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum, Index h) {
        // lookup the face
        Index f = -1;
        FaceID key;
        key.vertices[0] = indices[v1];
        key.vertices[1] = indices[v2];
        key.vertices[2] = indices[v3];
        key.vertices[3] = indices[v4];
        auto search = faces_set.find(key);
        if (search != faces_set.end()) {
            f = search->second.face_idx;
        }


        if (f != -1) {
            // face found, link its hexa with the current one 
            search->second.hexas[hexa_enum] = h;
            Index h2 = search->second.hexas[!hexa_enum];

            Index base_idx = h2 * 48;
            Dart* base = this->darts.data() + base_idx;
            Hexa::Face h2_face = opposite_face(face_enum);
            base[8 * h2_face + Face::Dart::TopRight].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::TopLeft;
            base[8 * h2_face + Face::Dart::TopLeft].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::TopRight;
            base[8 * h2_face + Face::Dart::LeftTop].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::LeftBot;
            base[8 * h2_face + Face::Dart::LeftBot].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::LeftTop;
            base[8 * h2_face + Face::Dart::BotLeft].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::BotRight;
            base[8 * h2_face + Face::Dart::BotRight].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::BotLeft;
            base[8 * h2_face + Face::Dart::RightBot].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::RightTop;
            base[8 * h2_face + Face::Dart::RightTop].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::RightBot;

            base[8 * face_enum + Face::Dart::TopRight].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::TopLeft;
            base[8 * face_enum + Face::Dart::TopLeft].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::TopRight;
            base[8 * face_enum + Face::Dart::LeftTop].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::LeftBot;
            base[8 * face_enum + Face::Dart::LeftBot].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::LeftTop;
            base[8 * face_enum + Face::Dart::BotLeft].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::LeftTop;
            base[8 * face_enum + Face::Dart::BotRight].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::BotLeft;
            base[8 * face_enum + Face::Dart::RightBot].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::RightTop;
            base[8 * face_enum + Face::Dart::RightTop].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::RightBot;

            hexas[h].neighbors[face_enum] = h2;
            hexas[h2].neighbors[h2_face] = h;

            //assert(faces[f].hexas[hexa_enum] == -1);
            //faces[f].hexas[hexa_enum] = h;
            //hexas[h].faces[face_enum] = f;
            //this->hexas[h].is_surface = (faces[f].hexas[0] == -1 || faces[f].hexas[1] == -1);
        } else {
            // face not found, insert
            Face face;
            //face.vertices[0] = indices[v1];
            //face.vertices[1] = indices[v2];
            //face.vertices[2] = indices[v3];
            //face.vertices[3] = indices[v4];
            //face.hexas[hexa_enum] = h;
            //hexas[h].faces[face_enum] = f = faces.size();
            face.dart = this->darts.size();
            this->faces.push_back(face);
            f = this->faces.size() - 1;

            FaceID fid;
            fid.vertices[0] = indices[v1];
            fid.vertices[1] = indices[v2];
            fid.vertices[2] = indices[v3];
            fid.vertices[3] = indices[v4];
            FaceValue fval;
            fval.face_idx = f;
            fval.hexas[hexa_enum] = h;
            std::pair<FaceID, FaceValue> keyval(fid, fval);
            faces_set.insert(keyval);
            
            //this->hexas[h].is_surface = (faces[f].hexas[0] == -1 || faces[f].hexas[1] == -1);
        }

        insert_edge(indices, v1, v2, Face::Edge::Top, h, f);
        insert_edge(indices, v2, v3, Face::Edge::Left, h, f);
        insert_edge(indices, v3, v4, Face::Edge::Bottom, h, f);
        insert_edge(indices, v4, v1, Face::Edge::Right, h, f);

        Index base_idx = this->darts.size() - 8;
        Dart* base = this->darts.data() + base_idx;
        base[Face::Dart::RightTop].edge_neighbor = base_idx + Face::Dart::TopRight;
        base[Face::Dart::TopRight].edge_neighbor = base_idx + Face::Dart::RightTop;
        base[Face::Dart::TopLeft].edge_neighbor = base_idx + Face::Dart::LeftTop;
        base[Face::Dart::LeftTop].edge_neighbor = base_idx + Face::Dart::TopLeft;
        base[Face::Dart::LeftBot].edge_neighbor = base_idx + Face::Dart::BotLeft;
        base[Face::Dart::BotLeft].edge_neighbor = base_idx + Face::Dart::LeftBot;
        base[Face::Dart::BotRight].edge_neighbor = base_idx + Face::Dart::RightBot;
        base[Face::Dart::RightBot].edge_neighbor = base_idx + Face::Dart::BotRight;
    }

    // All the noise in the insert_face call could be removed and reduced to only 1 parameter
    // which would then need to be switched upon, since the links are all 1:1, but a non-predictable
    // switch called this frequent might actually hurt a bit, so all parameters are hard-coded.
    // The hexa is by default visible, the property will then be modified upon insertion of
    // its components, if that's the case.
    void Mesh::insert_hexa(Index* indices) {
        this->hexas.emplace_back();
        Hexa& hexa = this->hexas.back();
        hexa.dart = this->darts.size();
        Index h = this->hexas.size() - 1;

        insert_face(indices, Hexa::Vertex::FarBotRight, Hexa::Vertex::NearBotRight, Hexa::Vertex::NearTopRight, Hexa::Vertex::FarTopRight, Face::Hexa::Front, Hexa::Face::Right, h);
        insert_face(indices, Hexa::Vertex::NearBotLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::NearTopLeft, Face::Hexa::Back, Hexa::Face::Left, h);
        insert_face(indices, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarTopRight, Hexa::Vertex::NearTopRight, Hexa::Vertex::NearTopLeft, Face::Hexa::Front, Hexa::Face::Top, h);
        insert_face(indices, Hexa::Vertex::FarBotRight, Hexa::Vertex::FarBotLeft, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearBotRight, Face::Hexa::Back, Hexa::Face::Bottom, h);
        insert_face(indices, Hexa::Vertex::NearBotRight, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearTopLeft, Hexa::Vertex::NearTopRight, Face::Hexa::Back, Hexa::Face::Near, h);
        insert_face(indices, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarBotRight, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarTopLeft, Face::Hexa::Front, Hexa::Face::Far, h);

        Index base_idx = this->darts.size() - 48;
        Dart* base = this->darts.data() + base_idx;
        base[8 * Hexa::Face::Right + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Right + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Right + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::RightTop;
        base[8 * Hexa::Face::Right + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Right + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Right + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::LeftTop;

        base[8 * Hexa::Face::Left + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Left + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::RightBot;
        base[8 * Hexa::Face::Left + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::RightTop;
        base[8 * Hexa::Face::Left + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::RightBot;
        base[8 * Hexa::Face::Left + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::RightTop;
        base[8 * Hexa::Face::Left + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::LeftTop;

        base[8 * Hexa::Face::Top + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::TopRight;

        base[8 * Hexa::Face::Bottom + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::BotLeft;

        base[8 * Hexa::Face::Near + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Near + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::TopRight;
        base[8 * Hexa::Face::Near + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Near + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::RightBot;
        base[8 * Hexa::Face::Near + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::RightTop;
        base[8 * Hexa::Face::Near + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::TopRight;
        base[8 * Hexa::Face::Near + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Near + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::LeftTop;

        base[8 * Hexa::Face::Far + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Far + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Far + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::BotRight;
        base[8 * Hexa::Face::Far + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::RightBot;
        base[8 * Hexa::Face::Far + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::RightTop;
        base[8 * Hexa::Face::Far + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Far + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::BotRight;
        base[8 * Hexa::Face::Far + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::LeftTop;
    }


    // Build a new ibuffer by reading the mesh data structures.
    void Mesh::make_ibuffer(Hyperplane<float, 3>* plane) {
        this->ibuffer.clear();
        HL_LOG("Building visible ibuffer...\n");
        for (unsigned int h = 0; h < this->hexas.size(); ++h) {

            Dart* d = &this->darts[this->hexas[h].dart];
            this->ibuffer.push_back(d->vertex);
            d = &this->darts[d->edge_neighbor];
            d = &this->darts[d->vertex_neighbor];
            this->ibuffer.push_back(d->vertex);
            d = &this->darts[d->edge_neighbor];
            d = &this->darts[d->vertex_neighbor];
            this->ibuffer.push_back(d->vertex);            
        }

        HL_LOG("%d vertex indices, %d visible faces (%d triangles)\n", this->ibuffer.size(), this->ibuffer.size() / 6, this->ibuffer.size() / 3);
    }

    // Clears the mesh from any previously loaded data
    void Mesh::unload() {
        this->hexas.clear();
        this->edges.clear();
        this->faces.clear();
        this->vertices.clear();
        this->vbuffer.clear();
        this->ibuffer.clear();
    }

    // Loads the mesh from a mesh file. 
    // The mesh file parser is not complete, but it's good enough for the purpose. 
    Result Mesh::load(std::string filename) {
        this->unload();

        FILE* file = fopen(filename.c_str(), "rb");
        if (file == NULL) {
            HL_LOG("Unable to find file %s!\n", filename.c_str());
            return Result::Error;
        }

        char buffer[100];
        int precision;
        int dimension;
        
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
                //g_vbuffer = &this->vbuffer;
            // Quad indices
            } else if (strcmp(buffer, "Quadrilaterals") == 0) {
                int quads_count;
                if (fscanf(file, "%d", &quads_count) == 0) {
                    HL_LOG("ERROR: malformed mesh file. Unexpected value after quads tag.\n");
                    goto error;
                }
                HL_LOG("Reading %d quads... (unused)\n", quads_count);
                for (int i = 0; i < quads_count; ++i) {
                    struct Quad { Index idx[4]; };
                    Quad q;
                    if (fscanf(file, "%d %d %d %d %*d", &q.idx[0], &q.idx[1], &q.idx[2], &q.idx[3]) == 0) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected quad format.\n");
                        goto error;
                    }
                    //for (int j = 0; j < 4; ++j) q.idx[j] = q.idx[j] - 1;
                    //this->quads.push_back(q);
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

        //HL_LOG("max e_count:%d, max f_count:%d\n", this->max_e_count, this->max_f_count);

        return Result::Success;

        error:
        fclose(file);

        return Result::Error;
    }
}
