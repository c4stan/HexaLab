#include "builder.h"

#include <memory>

namespace HexaLab {

    std::unordered_map<Builder::IndexPair, Builder::EdgeRef> Builder::edges_map;
    std::unordered_map<Builder::IndexQuad, Builder::FaceRef> Builder::faces_map;

    Mesh* Builder::s_mesh;
    MeshData* Builder::s_data;

    void Builder::add_edge(Index h, Index f, IndexPair indices) {
        Index e = -1;
        auto search_result = edges_map.find(indices);
        if (search_result != edges_map.end()) {
            e = search_result->second.idx;
        } else {
            e = s_mesh->edges.size();
            Mesh::Edge edge;
            edge.dart = s_mesh->darts.size();
            s_mesh->edges.push_back(edge);
            edges_map.insert(std::make_pair(indices, e));
        }

        // Vertex A
        Mesh::Vert& a = s_mesh->verts[std::get<0>(indices)];
        if (a.dart == -1)   // Store the first dart on the vertex in the vertex
            a.dart = s_mesh->darts.size();
        s_mesh->darts.emplace_back();
        Mesh::Dart& d1 = s_mesh->darts.back();
        d1.hexa = h;
        d1.face = f;
        d1.edge = e;
        d1.vert = std::get<0>(indices);

        // Vertex B
        Mesh::Vert& b = s_mesh->verts[std::get<1>(indices)];
        if (b.dart == -1)   // Store the first dart on the vertex in the vertex
            b.dart = s_mesh->darts.size();
        s_mesh->darts.emplace_back();
        Mesh::Dart& d2 = s_mesh->darts.back();
        d2.hexa = h;
        d2.face = f;
        d2.edge = e;
        d2.vert = std::get<1>(indices);

        // Link darts along the edge
        s_mesh->darts[s_mesh->darts.size() - 1].vert_neighbor = s_mesh->darts.size() - 2;
        s_mesh->darts[s_mesh->darts.size() - 2].vert_neighbor = s_mesh->darts.size() - 1;
    }

    Builder::Hexa::Face Builder::opposite(Hexa::Face face) {
        return (Hexa::Face)((face + 3) % 6);
    }

    void Builder::add_face(Index h, Hexa::Face face_enum, IndexQuad indices, Face::Hexa hexa_enum) {
        Index f = -1;
        auto search_result = faces_map.find(indices);
        if (search_result != faces_map.end()) {
            f = search_result->second.idx;
            Index* hexas = search_result->second.hexas;

            Index h2 = hexas[!hexa_enum];
            Hexa::Face h2_face = opposite(face_enum);

            hexas[hexa_enum] = h;            
            s_mesh->hexas[h].neighbors[face_enum] = h2;
            s_mesh->hexas[h2].neighbors[h2_face] = h;

            Index base_idx = h2 * 48;
            Mesh::Dart* base = s_mesh->darts.data() + base_idx;
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
            f = s_mesh->faces.size();
            Mesh::Face face;
            face.dart = s_mesh->darts.size();
            s_mesh->faces.push_back(face);
            FaceRef f_ref(f);
            f_ref.hexas[hexa_enum] = h;
            faces_map.insert(std::make_pair(indices, f_ref));
        }

        // Order matters here !
        add_edge(h, f, std::make_tuple(std::get<0>(indices), std::get<1>(indices)));
        add_edge(h, f, std::make_tuple(std::get<1>(indices), std::get<2>(indices)));
        add_edge(h, f, std::make_tuple(std::get<2>(indices), std::get<3>(indices)));
        add_edge(h, f, std::make_tuple(std::get<3>(indices), std::get<0>(indices)));

        Index base_idx = s_mesh->darts.size() - 8;
        Mesh::Dart* base = s_mesh->darts.data() + base_idx;
        base[Face::Dart::RightTop].edge_neighbor = base_idx + Face::Dart::TopRight;
        base[Face::Dart::TopRight].edge_neighbor = base_idx + Face::Dart::RightTop;
        base[Face::Dart::TopLeft].edge_neighbor = base_idx + Face::Dart::LeftTop;
        base[Face::Dart::LeftTop].edge_neighbor = base_idx + Face::Dart::TopLeft;
        base[Face::Dart::LeftBot].edge_neighbor = base_idx + Face::Dart::BotLeft;
        base[Face::Dart::BotLeft].edge_neighbor = base_idx + Face::Dart::LeftBot;
        base[Face::Dart::BotRight].edge_neighbor = base_idx + Face::Dart::RightBot;
        base[Face::Dart::RightBot].edge_neighbor = base_idx + Face::Dart::BotRight;
    }

    void Builder::add_hexa(MeshData::Hexa hexa) {
        Index h = s_mesh->hexas.size();
        s_mesh->hexas.emplace_back();
        Mesh::Hexa& mesh_hexa = s_mesh->hexas.back();
        mesh_hexa.dart = s_mesh->darts.size();

        // Order matters here ! sort it Hexa::Face - wise.
        IndexQuad face;
        face = std::make_tuple(hexa.verts[Hexa::Vert::NearBotLeft],
                               hexa.verts[Hexa::Vert::FarBotLeft],
                               hexa.verts[Hexa::Vert::FarTopLeft],
                               hexa.verts[Hexa::Vert::NearTopLeft]);
        add_face(h, Hexa::Face::Left, face, Face::Hexa::Back);

        face = std::make_tuple(hexa.verts[Hexa::Vert::FarBotRight],
                               hexa.verts[Hexa::Vert::FarBotLeft],
                               hexa.verts[Hexa::Vert::NearBotLeft],
                               hexa.verts[Hexa::Vert::NearBotRight]);
        add_face(h, Hexa::Face::Bottom, face, Face::Hexa::Back);

        face = std::make_tuple(hexa.verts[Hexa::Vert::NearBotRight],
                               hexa.verts[Hexa::Vert::NearBotLeft],
                               hexa.verts[Hexa::Vert::NearTopLeft],
                               hexa.verts[Hexa::Vert::NearTopRight]);
        add_face(h, Hexa::Face::Near, face, Face::Hexa::Back);

        face = std::make_tuple(hexa.verts[Hexa::Vert::NearBotRight],
                               hexa.verts[Hexa::Vert::FarBotRight],
                               hexa.verts[Hexa::Vert::FarTopRight],
                               hexa.verts[Hexa::Vert::NearTopRight]);
        add_face(h, Hexa::Face::Right, face, Face::Hexa::Front);

        face = std::make_tuple(hexa.verts[Hexa::Vert::FarTopRight],
                               hexa.verts[Hexa::Vert::FarTopLeft],
                               hexa.verts[Hexa::Vert::NearTopLeft],
                               hexa.verts[Hexa::Vert::NearTopRight]);
        add_face(h, Hexa::Face::Top, face, Face::Hexa::Front);

        face = std::make_tuple(hexa.verts[Hexa::Vert::FarBotRight],
                               hexa.verts[Hexa::Vert::FarBotLeft],
                               hexa.verts[Hexa::Vert::FarTopLeft],
                               hexa.verts[Hexa::Vert::FarTopRight]);
        add_face(h, Hexa::Face::Far, face, Face::Hexa::Front);

        Index base_idx = s_mesh->darts.size() - 48;
        Mesh::Dart* base = s_mesh->darts.data() + base_idx;
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

    void Builder::build(Mesh& mesh, MeshData& data) {
        s_mesh = &mesh;
        s_data = &data;

        edges_map.clear();
        faces_map.clear();

        for (unsigned int h = 0; h < data.hexas.size(); ++h) {
            add_hexa(data.hexas[h]);
        }
    }
}