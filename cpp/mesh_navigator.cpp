#include "mesh_navigator.h"

#include "mesh.h"

namespace HexaLab {
    MeshNavigator& MeshNavigator::flipH() { _dart = &_mesh->get_dart(_dart->hexa_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flipF() { _dart = &_mesh->get_dart(_dart->hexa_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flipE() { _dart = &_mesh->get_dart(_dart->hexa_neighbor); return *this; }
    MeshNavigator& MeshNavigator::flipV() { _dart = &_mesh->get_dart(_dart->hexa_neighbor); return *this; }

    Hexa& MeshNavigator::hexa() { return _mesh->get_hexa(_dart->hexa_neighbor); }
    Face& MeshNavigator::face() { return _mesh->get_face(_dart->face_neighbor); }
    Edge& MeshNavigator::edge() { return _mesh->get_edge(_dart->edge_neighbor); }
    Vert& MeshNavigator::vert() { return _mesh->get_vert(_dart->vert_neighbor); }
}