#include "builder.h"

#include <memory>

namespace HexaLab {

    std::unordered_map<Builder::EdgeMapKey, Index> Builder::edges_map;
    std::unordered_map<Builder::FaceMapKey, Index> Builder::faces_map;

    constexpr Index Builder::hexa_face[6][4];

    void Builder::add_edge(Mesh& mesh, Index h, Index f, const Index* edge) {
        // Lookup/add the edge
        Index e;
        auto search_result = edges_map.find(EdgeMapKey(edge));
        if (search_result != edges_map.end()) {
            e = search_result->second;
        } else {
            e = mesh.edges.size();
            mesh.edges.emplace_back(mesh.darts.size());
            edges_map.insert(std::make_pair(EdgeMapKey(edge), e));
        }

        // Add a dart to each vertex
        assert(mesh.verts.size() > edge[0]);
        if (mesh.verts[edge[0]].dart == -1) mesh.verts[edge[0]].dart = mesh.darts.size();
        mesh.darts.emplace_back(h, f, e, edge[0]);

        assert(mesh.verts.size() > edge[1]);
        if(mesh.verts[edge[1]].dart == -1) mesh.verts[edge[1]].dart = mesh.darts.size();
        mesh.darts.emplace_back(h, f, e, edge[1]);

        // Link darts along the edge
        mesh.darts[mesh.darts.size() - 1].vert_neighbor = mesh.darts.size() - 2;
        mesh.darts[mesh.darts.size() - 2].vert_neighbor = mesh.darts.size() - 1;
    }

    // h: index of the hexa to whom the face is path of.
    // face: array of 4 indices representing the face.
    void Builder::add_face(Mesh& mesh, Index h, const Index* face) {
        // Lookup/add the face
        Index f;
        auto search_result = faces_map.find(FaceMapKey(face));
        if (search_result != faces_map.end()) {
            f = search_result->second;
        } else {
            f = mesh.faces.size();
            faces_map.insert(std::make_pair(FaceMapKey(face), f));
            mesh.faces.emplace_back(mesh.darts.size());
        }

        Index f0_base = mesh.darts.size();

        // add face edges
        for (int i = 0; i < 4; ++i) {
            Index edge_indices[2] = {face[i], face[(i + 1) % 4]};
            add_edge(mesh, h, f, edge_indices);
        }

        // Link faces with the adjacent hexa, if there's one
        if (search_result != faces_map.end()) {
            Index f1_base = mesh.faces[f].dart;

            for (int i = 0; i < 8; ++i) {
                mesh.darts[f1_base + i].hexa_neighbor = f0_base + i;
                mesh.darts[f0_base + i].hexa_neighbor = f1_base + i;
            }
        }

        // link edges along the face
        for (int i = 0; i < 8; i += 2) {
            mesh.darts[f0_base + i].edge_neighbor = f0_base + (i + 1) % 8;
            mesh.darts[f0_base + (i + 1) % 8].edge_neighbor = f0_base + i;
        }
    }

    // hexa: array of 8 indices representing the hexa.
    void Builder::add_hexa(Mesh& mesh, const Index* hexa) {
        const Index h = mesh.hexas.size();
        mesh.hexas.emplace_back(mesh.darts.size());

        for (int i = 0; i < 6; ++i) {
            Index face_indices[4];
            for (int j = 0; j < 4; ++j) {
                face_indices[j] = hexa[Builder::hexa_face[i][j]];
            }
            add_face(mesh, h, face_indices);

            // TODO link face darts along the hexa ...

        }
        
        assert(mesh.darts.size() >= 48);
        Index base_idx = mesh.darts.size() - 48;
        Dart* base = mesh.darts.data() + base_idx;
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

    void Builder::build(Mesh& mesh, const vector<Vector3f>& vertices, const vector<Index>& indices) {
        assert(indices.size() % 8 == 0);

        edges_map.clear();
        faces_map.clear();

        for (size_t v = 0; v < vertices.size(); ++v) {
            mesh.verts.emplace_back(vertices[v]);
        }

        size_t hexa_count = indices.size() / 8;
        for (size_t h = 0; h < hexa_count; ++h) {
            add_hexa(mesh, &indices[h * 8]);
        }
    }

    bool Builder::validate(Mesh& mesh) {
        int surface_darts = 0;
        for (size_t i = 0; i < mesh.get_darts().size(); ++i) {
            Dart& dart = mesh.get_dart(i);

            auto nav = mesh.navigate(dart);

            HL_ASSERT(dart.hexa != -1);
            HL_ASSERT(dart.face != -1);
            HL_ASSERT(dart.edge != -1);
            HL_ASSERT(dart.vert != -1);
            HL_ASSERT(dart.face_neighbor != -1);
            HL_ASSERT(dart.edge_neighbor != -1);
            HL_ASSERT(dart.vert_neighbor != -1);

            if (dart.hexa_neighbor == -1) {
                ++surface_darts;
            } else {
                nav.flip_hexa().flip_hexa();
                HL_ASSERT(dart == nav.dart());
            }
            

            // TODO add more asserts
        }

        for (size_t i = 0; i < mesh.get_verts().size(); ++i) {
            Vert& v = mesh.get_vert(i);
            assert(v.dart != -1);
            auto nav = mesh.navigate(v);
            assert(nav.vert() == v);
            nav.flip_vert().flip_vert();
            assert(nav.vert() == v);
        }

        for (size_t i = 0; i < mesh.get_edges().size(); ++i) {
            Edge& e = mesh.get_edge(i);
            assert(e.dart != -1);
            auto nav = mesh.navigate(e);
            assert(nav.edge() == e);
            nav.flip_edge().flip_edge();
            assert(nav.edge() == e);
        }

        for (size_t i = 0; i < mesh.get_faces().size(); ++i) {
            Face& f = mesh.get_face(i);
            assert(f.dart != -1);
            auto nav = mesh.navigate(f);
            assert(nav.face() == f);
            nav.flip_face();
            nav.flip_face();
            assert(nav.face() == f);
        }

        HL_LOG("[Mesh validator] Surface darts: %d/%d\n", surface_darts, mesh.get_darts().size());

        return true;
    }
}