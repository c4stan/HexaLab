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

        // Add face edges
        for (int i = 0; i < 4; ++i) {
            Index edge_indices[2] = {face[i], face[(i + 1) % 4]};
            add_edge(mesh, h, f, edge_indices);
        }

        // Compute face normal, if its the first
        if (search_result == faces_map.end()) {
            Vector3f normal(0, 0, 0);
            Vector3f a, b;
            a = mesh.verts[face[1]].position - mesh.verts[face[0]].position;
            b = mesh.verts[face[3]].position - mesh.verts[face[0]].position;
            normal += a.cross(b).normalized();
            a = mesh.verts[face[2]].position - mesh.verts[face[1]].position;
            b = mesh.verts[face[0]].position - mesh.verts[face[1]].position;
            normal += a.cross(b).normalized();
            a = mesh.verts[face[3]].position - mesh.verts[face[2]].position;
            b = mesh.verts[face[1]].position - mesh.verts[face[2]].position;
            normal += a.cross(b).normalized();
            a = mesh.verts[face[0]].position - mesh.verts[face[3]].position;
            b = mesh.verts[face[2]].position - mesh.verts[face[3]].position;
            normal += a.cross(b).normalized();
            (normal /= 4).normalize();
            mesh.faces.back().normal = normal;
        }

        // Link faces with the adjacent hexa, if there's one
        if (search_result != faces_map.end()) {
            Index f1_base = mesh.faces[f].dart;

            int i = 0, j = 9;
            for (; i < 8; ++i, --j) {
                mesh.darts[f1_base + i].hexa_neighbor = f0_base + j % 8;
                mesh.darts[f0_base + j % 8].hexa_neighbor = f1_base + i;
            }
        }

        // Link edges along the face
        for (int i = 1; i < 8; i += 2) {
            mesh.darts[f0_base + i].edge_neighbor = f0_base + (i + 1) % 8;
            mesh.darts[f0_base + (i + 1) % 8].edge_neighbor = f0_base + i;
        }
    }

    // hexa: array of 8 indices representing the hexa.
    void Builder::add_hexa(Mesh& mesh, const Index* hexa) {
        const Index h = mesh.hexas.size();
        mesh.hexas.emplace_back(mesh.darts.size());

        Index base = mesh.darts.size();

        for (int i = 0; i < 6; ++i) {
            Index face_indices[4];
            for (int j = 0; j < 4; ++j) {
                face_indices[j] = hexa[Builder::hexa_face[i][j]];
            }
            add_face(mesh, h, face_indices);
        }

        // Link side faces
        const int face_size = 8;        // darts in a face
        const int edge_offset = 4;      // dart offset between two side edges (e.g. two left darts, two top darts, two right darts => offset == 4)
        for (int i = 2; i < face_size * 4; i += face_size) {
            mesh.darts[base + i].face_neighbor = base + (i + face_size + edge_offset + 1) % (face_size * 4);
            mesh.darts[base + i + 1].face_neighbor = base + (i + face_size + edge_offset) % (face_size * 4);
        
            mesh.darts[base + (i + face_size + edge_offset + 1) % (face_size * 4)].face_neighbor = base + i;
            mesh.darts[base + (i + face_size + edge_offset) % (face_size * 4)].face_neighbor = base + i + 1;
        }
        // Link bottom face to side faces
        int link_offset = 38;
        int step = 10;
        for (int i = 0; i < face_size * 4; i += face_size) {
            mesh.darts[base + i].face_neighbor = base + i + link_offset + 1;
            mesh.darts[base + i + 1].face_neighbor = base + i + link_offset;

            mesh.darts[base + i + link_offset].face_neighbor = base + i + 1;
            mesh.darts[base + i + link_offset + 1].face_neighbor = base + i;
            
            link_offset -= step;
        }
        // Link top face to side faces
        link_offset = 38;
        step = 6;
        int i = 4;
        for (; i < face_size * 3; i += face_size) {
            mesh.darts[base + i].face_neighbor = base + i + link_offset + 1;
            mesh.darts[base + i + 1].face_neighbor = base + i + link_offset;

            mesh.darts[base + i + link_offset].face_neighbor = base + i + 1;
            mesh.darts[base + i + link_offset + 1].face_neighbor = base + i;

            link_offset -= step;
        }
        // Last face is "irregular", requires an extra step
        {
            link_offset -= 8;
            mesh.darts[base + i].face_neighbor = base + i + link_offset + 1;
            mesh.darts[base + i + 1].face_neighbor = base + i + link_offset;

            mesh.darts[base + i + link_offset].face_neighbor = base + i + 1;
            mesh.darts[base + i + link_offset + 1].face_neighbor = base + i;
        }
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
            HL_ASSERT(v.dart != -1);
            auto nav = mesh.navigate(v);
            HL_ASSERT(nav.vert() == v);
            nav.flip_vert().flip_vert();
            HL_ASSERT(nav.vert() == v);
        }

        for (size_t i = 0; i < mesh.get_edges().size(); ++i) {
            Edge& e = mesh.get_edge(i);
            HL_ASSERT(e.dart != -1);
            auto nav = mesh.navigate(e);
            HL_ASSERT(nav.edge() == e);
            nav.flip_edge().flip_edge();
            HL_ASSERT(nav.edge() == e);
        }

        for (size_t i = 0; i < mesh.get_faces().size(); ++i) {
            Face& f = mesh.get_face(i);
            HL_ASSERT(f.dart != -1);
            auto nav = mesh.navigate(f);
            HL_ASSERT(nav.face() == f);
            nav.flip_face();
            nav.flip_face();
            HL_ASSERT(nav.face() == f);
        }

        for (size_t i = 0; i < mesh.get_hexas().size(); ++i) {
            Hexa& h = mesh.get_hexa(i);
            HL_ASSERT(h.dart != -1);
            auto nav = mesh.navigate(h);
            HL_ASSERT(nav.hexa() == h);
            if (nav.dart().hexa_neighbor != -1) {
                nav.flip_hexa();
                nav.flip_hexa();
                HL_ASSERT(nav.hexa() == h);
            }
        }

        HL_LOG("[Mesh validator] Surface darts: %d/%d\n", surface_darts, mesh.get_darts().size());

        return true;
    }
}