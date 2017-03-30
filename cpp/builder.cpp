#include "builder.h"

#include <memory>

namespace HexaLab {

    std::unordered_map<Builder::IndexPair, Builder::EdgeRef> Builder::edges_map;
    std::unordered_map<Builder::IndexQuad, Builder::FaceRef> Builder::faces_map;

    Mesh*           Builder::mesh;
    const MeshData* Builder::mesh_data;

    void Builder::add_edge(Index h, Index f, IndexPair indices) {
        Index e = -1;
        auto search_result = edges_map.find(indices);
        if (search_result != edges_map.end()) {
            e = search_result->second.idx;
        } else {
            e = mesh->edges.size();
            Edge edge;
            edge.dart = mesh->darts.size();
            mesh->edges.push_back(edge);
            edges_map.insert(std::make_pair(indices, e));
        }

        // Vertex A
        Index i1 = std::get<0>(indices);
        assert(mesh->verts.capacity() > i1);
        Vert& a = mesh->verts[i1];
        if (a.dart == -1) {   // Store the first dart on the vertex in the vertex
            a.dart = mesh->darts.size();
            a.position = mesh_data->get_vert(i1);
        }
        mesh->darts.emplace_back();
        Dart& d1 = mesh->darts.back();
        d1.hexa = h;
        d1.face = f;
        d1.edge = e;
        d1.vert = i1;

        // Vertex B
        Index i2 = std::get<1>(indices);
        assert(mesh->verts.capacity() > i2);
        Vert& b = mesh->verts[i2];
        if (b.dart == -1) {  // Store the first dart on the vertex in the vertex
            b.dart = mesh->darts.size();
            b.position = mesh_data->get_vert(i2);
        }
        mesh->darts.emplace_back();
        Dart& d2 = mesh->darts.back();
        d2.hexa = h;
        d2.face = f;
        d2.edge = e;
        d2.vert = i2;

        // Link darts along the edge
        mesh->darts[mesh->darts.size() - 1].vert_neighbor = mesh->darts.size() - 2;
        mesh->darts[mesh->darts.size() - 2].vert_neighbor = mesh->darts.size() - 1;
    }

    Builder::EFace Builder::opposite(EFace face) {
        return (EFace)((face + 3) % 6);
    }

    void Builder::add_face(Index h, EFace face_enum, IndexQuad indices, EHexa hexa_enum) {
        Index f = -1;
        auto search_result = faces_map.find(indices);
        if (search_result != faces_map.end()) {
            f = search_result->second.idx;
            Index* hexas = search_result->second.hexas;

            Index h2 = hexas[!hexa_enum];
            EFace h2_face = opposite(face_enum);

            hexas[hexa_enum] = h;            

            Index base_idx = h2 * 48;
            Dart* base = mesh->darts.data() + base_idx;
            base[8 * h2_face + EDart::TopRight].hexa_neighbor = base_idx + 8 * face_enum + EDart::TopRight;
            base[8 * h2_face + EDart::TopLeft].hexa_neighbor = base_idx + 8 * face_enum + EDart::TopLeft;
            base[8 * h2_face + EDart::LeftTop].hexa_neighbor = base_idx + 8 * face_enum + EDart::LeftTop;
            base[8 * h2_face + EDart::LeftBot].hexa_neighbor = base_idx + 8 * face_enum + EDart::LeftBot;
            base[8 * h2_face + EDart::BotLeft].hexa_neighbor = base_idx + 8 * face_enum + EDart::BotLeft;
            base[8 * h2_face + EDart::BotRight].hexa_neighbor = base_idx + 8 * face_enum + EDart::BotRight;
            base[8 * h2_face + EDart::RightBot].hexa_neighbor = base_idx + 8 * face_enum + EDart::RightBot;
            base[8 * h2_face + EDart::RightTop].hexa_neighbor = base_idx + 8 * face_enum + EDart::RightTop;

            base[8 * face_enum + EDart::TopRight].hexa_neighbor = base_idx + 8 * h2_face + EDart::TopRight;
            base[8 * face_enum + EDart::TopLeft].hexa_neighbor = base_idx + 8 * h2_face + EDart::TopLeft;
            base[8 * face_enum + EDart::LeftTop].hexa_neighbor = base_idx + 8 * h2_face + EDart::LeftTop;
            base[8 * face_enum + EDart::LeftBot].hexa_neighbor = base_idx + 8 * h2_face + EDart::LeftBot;
            base[8 * face_enum + EDart::BotLeft].hexa_neighbor = base_idx + 8 * h2_face + EDart::BotLeft;
            base[8 * face_enum + EDart::BotRight].hexa_neighbor = base_idx + 8 * h2_face + EDart::BotRight;
            base[8 * face_enum + EDart::RightBot].hexa_neighbor = base_idx + 8 * h2_face + EDart::RightBot;
            base[8 * face_enum + EDart::RightTop].hexa_neighbor = base_idx + 8 * h2_face + EDart::RightTop;
        } else {
            // face not found, insert
            f = mesh->faces.size();
            Face face;
            face.dart = mesh->darts.size();
            mesh->faces.push_back(face);
            FaceRef f_ref(f);
            f_ref.hexas[hexa_enum] = h;
            faces_map.insert(std::make_pair(indices, f_ref));
        }

        // Order matters here !
        add_edge(h, f, std::make_tuple(std::get<0>(indices), std::get<1>(indices)));
        add_edge(h, f, std::make_tuple(std::get<1>(indices), std::get<2>(indices)));
        add_edge(h, f, std::make_tuple(std::get<2>(indices), std::get<3>(indices)));
        add_edge(h, f, std::make_tuple(std::get<3>(indices), std::get<0>(indices)));

        Index base_idx = mesh->darts.size() - 8;
        Dart* base = mesh->darts.data() + base_idx;
        base[EDart::RightTop].edge_neighbor = base_idx + EDart::TopRight;
        base[EDart::TopRight].edge_neighbor = base_idx + EDart::RightTop;
        base[EDart::TopLeft].edge_neighbor = base_idx + EDart::LeftTop;
        base[EDart::LeftTop].edge_neighbor = base_idx + EDart::TopLeft;
        base[EDart::LeftBot].edge_neighbor = base_idx + EDart::BotLeft;
        base[EDart::BotLeft].edge_neighbor = base_idx + EDart::LeftBot;
        base[EDart::BotRight].edge_neighbor = base_idx + EDart::RightBot;
        base[EDart::RightBot].edge_neighbor = base_idx + EDart::BotRight;
    }

    void Builder::add_hexa(MeshData::Hexa hexa) {
        Index h = mesh->hexas.size();
        mesh->hexas.emplace_back();
        Hexa& mesh_hexa = mesh->hexas.back();
        mesh_hexa.dart = mesh->darts.size();

        // Order matters here ! sort it EFace - wise.
        IndexQuad face;
        face = std::make_tuple(hexa.verts[EVert::NearBotLeft],
                               hexa.verts[EVert::FarBotLeft],
                               hexa.verts[EVert::FarTopLeft],
                               hexa.verts[EVert::NearTopLeft]);
        add_face(h, EFace::Left, face, EHexa::Back);

        face = std::make_tuple(hexa.verts[EVert::FarBotRight],
                               hexa.verts[EVert::FarBotLeft],
                               hexa.verts[EVert::NearBotLeft],
                               hexa.verts[EVert::NearBotRight]);
        add_face(h, EFace::Bottom, face, EHexa::Back);

        face = std::make_tuple(hexa.verts[EVert::NearBotRight],
                               hexa.verts[EVert::NearBotLeft],
                               hexa.verts[EVert::NearTopLeft],
                               hexa.verts[EVert::NearTopRight]);
        add_face(h, EFace::Near, face, EHexa::Back);

        face = std::make_tuple(hexa.verts[EVert::NearBotRight],
                               hexa.verts[EVert::FarBotRight],
                               hexa.verts[EVert::FarTopRight],
                               hexa.verts[EVert::NearTopRight]);
        add_face(h, EFace::Right, face, EHexa::Front);

        face = std::make_tuple(hexa.verts[EVert::FarTopRight],
                               hexa.verts[EVert::FarTopLeft],
                               hexa.verts[EVert::NearTopLeft],
                               hexa.verts[EVert::NearTopRight]);
        add_face(h, EFace::Top, face, EHexa::Front);

        face = std::make_tuple(hexa.verts[EVert::FarBotRight],
                               hexa.verts[EVert::FarBotLeft],
                               hexa.verts[EVert::FarTopLeft],
                               hexa.verts[EVert::FarTopRight]);
        add_face(h, EFace::Far, face, EHexa::Front);

        Index base_idx = mesh->darts.size() - 48;
        Dart* base = mesh->darts.data() + base_idx;
        base[8 * EFace::Left + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Near + EDart::LeftBot;
        base[8 * EFace::Left + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::LeftTop;
        base[8 * EFace::Left + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::LeftBot;
        base[8 * EFace::Left + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Far + EDart::LeftBot;
        base[8 * EFace::Left + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Far + EDart::LeftTop;
        base[8 * EFace::Left + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Top + EDart::LeftBot;
        base[8 * EFace::Left + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Top + EDart::LeftTop;
        base[8 * EFace::Left + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Near + EDart::LeftTop;

        base[8 * EFace::Right + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Near + EDart::RightBot;
        base[8 * EFace::Right + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::RightTop;
        base[8 * EFace::Right + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::RightBot;
        base[8 * EFace::Right + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Far + EDart::RightBot;
        base[8 * EFace::Right + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Far + EDart::RightTop;
        base[8 * EFace::Right + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Top + EDart::RightBot;
        base[8 * EFace::Right + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Top + EDart::RightTop;
        base[8 * EFace::Right + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Near + EDart::RightTop;

        base[8 * EFace::Top + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Right + EDart::TopLeft;
        base[8 * EFace::Top + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Far + EDart::TopRight;
        base[8 * EFace::Top + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Far + EDart::TopLeft;
        base[8 * EFace::Top + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Left + EDart::TopLeft;
        base[8 * EFace::Top + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Left + EDart::TopRight;
        base[8 * EFace::Top + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Near + EDart::TopLeft;
        base[8 * EFace::Top + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Near + EDart::TopRight;
        base[8 * EFace::Top + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Right + EDart::TopRight;

        base[8 * EFace::Bottom + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Right + EDart::BotLeft;
        base[8 * EFace::Bottom + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Far + EDart::BotRight;
        base[8 * EFace::Bottom + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Far + EDart::BotLeft;
        base[8 * EFace::Bottom + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Left + EDart::BotLeft;
        base[8 * EFace::Bottom + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Left + EDart::BotRight;
        base[8 * EFace::Bottom + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Near + EDart::BotLeft;
        base[8 * EFace::Bottom + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Near + EDart::BotRight;
        base[8 * EFace::Bottom + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Right + EDart::BotRight;

        base[8 * EFace::Near + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Right + EDart::RightBot;
        base[8 * EFace::Near + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::TopRight;
        base[8 * EFace::Near + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::TopLeft;
        base[8 * EFace::Near + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Left + EDart::RightBot;
        base[8 * EFace::Near + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Left + EDart::RightTop;
        base[8 * EFace::Near + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Top + EDart::TopLeft;
        base[8 * EFace::Near + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Top + EDart::TopRight;
        base[8 * EFace::Near + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Right + EDart::RightTop;

        base[8 * EFace::Far + EDart::RightBot].face_neighbor = base_idx + 8 * EFace::Right + EDart::LeftBot;
        base[8 * EFace::Far + EDart::BotRight].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::BotRight;
        base[8 * EFace::Far + EDart::BotLeft].face_neighbor = base_idx + 8 * EFace::Bottom + EDart::BotLeft;
        base[8 * EFace::Far + EDart::LeftBot].face_neighbor = base_idx + 8 * EFace::Left + EDart::LeftBot;
        base[8 * EFace::Far + EDart::LeftTop].face_neighbor = base_idx + 8 * EFace::Left + EDart::LeftTop;
        base[8 * EFace::Far + EDart::TopLeft].face_neighbor = base_idx + 8 * EFace::Top + EDart::BotLeft;
        base[8 * EFace::Far + EDart::TopRight].face_neighbor = base_idx + 8 * EFace::Top + EDart::BotRight;
        base[8 * EFace::Far + EDart::RightTop].face_neighbor = base_idx + 8 * EFace::Right + EDart::LeftTop;
    }

    Mesh Builder::build(const MeshData& data) {
        Mesh mesh;

        Builder::mesh      = &mesh;
        Builder::mesh_data = &data;

        edges_map.clear();
        faces_map.clear();

        mesh.verts.resize(data.get_verts().size());

        for (unsigned int h = 0; h < data.get_hexas().size(); ++h) {
            add_hexa(data.get_hexa(h));
        }

        return mesh;
    }

    Result Builder::validate(Mesh& mesh) {
        for (size_t i = 0; i < mesh.get_darts().size(); ++i) {
            Dart& dart = mesh.get_dart(i);

            HL_ASSERT(dart.hexa != -1);
            HL_ASSERT(dart.face != -1);
            HL_ASSERT(dart.edge != -1);
            HL_ASSERT(dart.vert != -1);
            HL_ASSERT(dart.face_neighbor != -1);
            HL_ASSERT(dart.edge_neighbor != -1);
            HL_ASSERT(dart.vert_neighbor != -1);

            MeshNavigator nav = mesh.navigate(dart);

            nav.flip_face().flip_face();
            HL_ASSERT(dart == nav.dart());

            nav.flip_edge().flip_edge();
            HL_ASSERT(dart == nav.dart());

            nav.flip_vert().flip_vert();
            HL_ASSERT(dart == nav.dart());

            // TODO add more asserts
        }

        for (size_t i = 0; i < mesh.get_verts().size(); ++i) {
            HL_ASSERT(mesh.get_vert(i).dart != -1);
        }

        for (size_t i = 0; i < mesh.get_edges().size(); ++i) {
            HL_ASSERT(mesh.get_edge(i).dart != -1);
        }

        for (size_t i = 0; i < mesh.get_faces().size(); ++i) {
            HL_ASSERT(mesh.get_face(i).dart != -1);
        }

        return Result::Success;
    }
}