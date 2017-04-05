#include "mesh_navigator.h"

#include "mesh.h"

namespace HexaLab {
    MeshNavigator& MeshNavigator::flip_hexa() { _dart = &_mesh->get_dart(_dart->hexa_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flip_face() { _dart = &_mesh->get_dart(_dart->face_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flip_edge() { _dart = &_mesh->get_dart(_dart->edge_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flip_vert() { _dart = &_mesh->get_dart(_dart->vert_neighbor); return *this; }

    MeshNavigator& MeshNavigator::rotate_on_edge() { return flip_hexa().flip_face(); }
    MeshNavigator& MeshNavigator::rotate_on_face() { return flip_vert().flip_edge(); }
    MeshNavigator& MeshNavigator::rotate_on_hexa() { return flip_vert().flip_edge().flip_face().flip_edge(); }

    MeshNavigator& MeshNavigator::next_hexa_face() { return flip_vert().flip_edge().flip_face(); }

    Hexa& MeshNavigator::hexa() { return _mesh->get_hexa(_dart->hexa); }
    Face& MeshNavigator::face() { return _mesh->get_face(_dart->face); }
    Edge& MeshNavigator::edge() { return _mesh->get_edge(_dart->edge); }
    Vert& MeshNavigator::vert() { return _mesh->get_vert(_dart->vert); }

    Hexa& MeshNavigator::peek_hexa() { return _mesh->get_hexa(_mesh->get_dart(_dart->hexa_neighbor).hexa); }
}