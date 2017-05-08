#ifndef _HL_IVIEW_H_
#define _HL_IVIEW_H_

#include <mesh.h>

#include <Eigen\dense>
#include <string>

namespace HexaLab {
    using namespace std;
    using namespace Eigen;

    class IView {
    protected:
        Mesh* mesh = nullptr;
        Eigen::AlignedBox3f aabb;
        const char* name;
    public:
        IView(const char* name) { this->name = name; }
        virtual void set_mesh(js_ptr mesh) = 0;
        virtual void update() = 0;
        float get_size() { return mesh != nullptr ? aabb.diagonal().norm() : 0; };
        Vector3f get_center() { return aabb.center(); };
        const char* get_name() { return name; }
    };
}

#endif