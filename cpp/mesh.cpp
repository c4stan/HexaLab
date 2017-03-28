#include "mesh.h"

#include <string.h>
#include <string>

namespace HexaLab {
    using namespace Eigen;
    using namespace std;

    // Insertion is basically lookup inside the edges hash table -> found: update fields,
    // not found: insert the edge into the table, update fields, and also insert the new edge's vertices
    void Mesh::insert_edge(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Face::Edge edge_enum, Index h, Index f) {
        Index e = -1;
        auto search_result = edges_set.find(std::make_tuple(indices[v1], indices[v2]));
        if (search_result != edges_set.end()) {
            e = search_result->second;
        } else {
            e = edges.size();
            Edge edge;
            edge.dart = this->darts.size();
            edges.push_back(edge);
            edges_set.insert(std::make_pair(std::make_tuple(indices[v1], indices[v2]), e));
        }

        // Vertex A
        Vertex& a = vertices[indices[v1]];
        if (a.dart == -1)   // Store the first dart on the vertex in the vertex
            a.dart = this->darts.size();
        this->darts.emplace_back();
        Dart& d1 = this->darts.back();
        d1.hexa = h;
        d1.face = f;
        d1.edge = e;
        d1.vertex = indices[v1];

        // Vertex B
        Vertex& b = vertices[indices[v2]];
        if (b.dart == -1)   // Store the first dart on the vertex in the vertex
            b.dart = this->darts.size();
        this->darts.emplace_back();
        Dart& d2 = this->darts.back();
        d2.hexa = h;
        d2.face = f;
        d2.edge = e;
        d2.vertex = indices[v2];

        // Link darts along the edge
        darts[darts.size() - 1].vertex_neighbor = darts.size() - 2;
        darts[darts.size() - 2].vertex_neighbor = darts.size() - 1;
    }

    // It is assumed that the face belongs to the last hexa, TODO fix this
    // Insertion is basically lookup inside the faces hash table -> found: update fields,
    // not found: insert the face into the table, update fields, and also insert the new face's edges
    void Mesh::insert_face(Index* indices, Hexa::Vertex v1, Hexa::Vertex v2, Hexa::Vertex v3, Hexa::Vertex v4, Face::Hexa hexa_enum, Hexa::Face face_enum, Index h) {
        Index f = -1;
        auto search_result = faces_set.find(std::make_tuple(indices[v1], indices[v2], indices[v3], indices[v4]));
        if (search_result != faces_set.end()) {
            f = search_result->second;

            Index h2 = faces[f].hexas[!hexa_enum];
            Hexa::Face h2_face = Hexa::opposite(face_enum);

            faces[f].hexas[hexa_enum] = h;            
            hexas[h].neighbors[face_enum] = h2;
            hexas[h2].neighbors[h2_face] = h;

            Index base_idx = h2 * 48;
            Dart* base = this->darts.data() + base_idx;
            base[8 * h2_face + Face::Dart::TopRight].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::TopRight;
            base[8 * h2_face + Face::Dart::TopLeft].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::TopLeft;
            base[8 * h2_face + Face::Dart::LeftTop].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::LeftTop;
            base[8 * h2_face + Face::Dart::LeftBot].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::LeftBot;
            base[8 * h2_face + Face::Dart::BotLeft].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::BotLeft;
            base[8 * h2_face + Face::Dart::BotRight].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::BotRight;
            base[8 * h2_face + Face::Dart::RightBot].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::RightBot;
            base[8 * h2_face + Face::Dart::RightTop].hexa_neighbor = base_idx + 8 * face_enum + Face::Dart::RightTop;

            base[8 * face_enum + Face::Dart::TopRight].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::TopRight;
            base[8 * face_enum + Face::Dart::TopLeft].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::TopLeft;
            base[8 * face_enum + Face::Dart::LeftTop].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::LeftTop;
            base[8 * face_enum + Face::Dart::LeftBot].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::LeftBot;
            base[8 * face_enum + Face::Dart::BotLeft].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::BotLeft;
            base[8 * face_enum + Face::Dart::BotRight].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::BotRight;
            base[8 * face_enum + Face::Dart::RightBot].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::RightBot;
            base[8 * face_enum + Face::Dart::RightTop].hexa_neighbor = base_idx + 8 * h2_face + Face::Dart::RightTop;
        } else {
            // face not found, insert
            f = this->faces.size();
            Face face;
            face.dart = this->darts.size();
            face.hexas[hexa_enum] = h;
            this->faces.push_back(face);
            faces_set.insert(std::make_pair(std::make_tuple(indices[v1], indices[v2], indices[v3], indices[v4]), f));
        }

        // Order matters here !
        insert_edge(indices, v1, v2, Face::Edge::Bottom, h, f);
        insert_edge(indices, v2, v3, Face::Edge::Left, h, f);
        insert_edge(indices, v3, v4, Face::Edge::Top, h, f);
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

    void Mesh::insert_hexa(Index* indices) {
        Index h = this->hexas.size();
        this->hexas.emplace_back();
        Hexa& hexa = this->hexas.back();
        hexa.dart = this->darts.size();

        // Order matters here ! sort it Hexa::Face - wise.
        insert_face(indices, Hexa::Vertex::NearBotLeft, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::NearTopLeft, Face::Hexa::Back, Hexa::Face::Left, h);
        insert_face(indices, Hexa::Vertex::NearBotRight, Hexa::Vertex::FarBotRight, Hexa::Vertex::FarTopRight, Hexa::Vertex::NearTopRight, Face::Hexa::Front, Hexa::Face::Right, h);
        insert_face(indices, Hexa::Vertex::FarBotRight, Hexa::Vertex::FarBotLeft, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearBotRight, Face::Hexa::Back, Hexa::Face::Bottom, h);
        insert_face(indices, Hexa::Vertex::FarTopRight, Hexa::Vertex::FarTopLeft, Hexa::Vertex::NearTopLeft, Hexa::Vertex::NearTopRight, Face::Hexa::Front, Hexa::Face::Top, h);
        insert_face(indices, Hexa::Vertex::NearBotRight, Hexa::Vertex::NearBotLeft, Hexa::Vertex::NearTopLeft, Hexa::Vertex::NearTopRight, Face::Hexa::Back, Hexa::Face::Near, h);
        insert_face(indices, Hexa::Vertex::FarBotRight, Hexa::Vertex::FarBotLeft, Hexa::Vertex::FarTopLeft, Hexa::Vertex::FarTopRight, Face::Hexa::Front, Hexa::Face::Far, h);

        Index base_idx = this->darts.size() - 48;
        Dart* base = this->darts.data() + base_idx;
        base[8 * Hexa::Face::Left + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Left + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Left + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Left + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Left + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::LeftTop;

        base[8 * Hexa::Face::Right + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::RightTop;
        base[8 * Hexa::Face::Right + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::RightTop;
        base[8 * Hexa::Face::Right + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::RightBot;
        base[8 * Hexa::Face::Right + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::RightTop;
        base[8 * Hexa::Face::Right + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::RightTop;

        base[8 * Hexa::Face::Top + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Top + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::TopRight;
        base[8 * Hexa::Face::Top + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::TopRight;

        base[8 * Hexa::Face::Bottom + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Far + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Bottom + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Near + Face::Dart::BotRight;
        base[8 * Hexa::Face::Bottom + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::BotRight;

        base[8 * Hexa::Face::Near + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::RightBot;
        base[8 * Hexa::Face::Near + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::TopRight;
        base[8 * Hexa::Face::Near + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Near + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::RightBot;
        base[8 * Hexa::Face::Near + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::RightTop;
        base[8 * Hexa::Face::Near + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::TopLeft;
        base[8 * Hexa::Face::Near + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::TopRight;
        base[8 * Hexa::Face::Near + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::RightTop;

        base[8 * Hexa::Face::Far + Face::Dart::RightBot].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Far + Face::Dart::BotRight].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::BotRight;
        base[8 * Hexa::Face::Far + Face::Dart::BotLeft].face_neighbor = base_idx + 8 * Hexa::Face::Bottom + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Far + Face::Dart::LeftBot].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::LeftBot;
        base[8 * Hexa::Face::Far + Face::Dart::LeftTop].face_neighbor = base_idx + 8 * Hexa::Face::Left + Face::Dart::LeftTop;
        base[8 * Hexa::Face::Far + Face::Dart::TopLeft].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::BotLeft;
        base[8 * Hexa::Face::Far + Face::Dart::TopRight].face_neighbor = base_idx + 8 * Hexa::Face::Top + Face::Dart::BotRight;
        base[8 * Hexa::Face::Far + Face::Dart::RightTop].face_neighbor = base_idx + 8 * Hexa::Face::Right + Face::Dart::LeftTop;
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
        //this->ibuffer.clear();
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
                this->vertices.reserve(vertices_count);
                for (int i = 0; i < vertices_count; ++i) {
                    Vertex v;
                    if (fscanf(file, "%f %f %f %*f", &v.position.x(), &v.position.y(), &v.position.z()) == 0) {
                        HL_LOG("ERROR: malformed mesh file. Unexpected vertex format.\n");
                        goto error;
                    }
                    this->vertices.push_back(v);
                    this->aabb.extend(v.position);
                }
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
