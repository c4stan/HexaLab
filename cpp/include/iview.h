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
        Mesh& mesh;
    public:
        IView(Mesh& mesh) : mesh(mesh) {};
        virtual void update() = 0;
    };
}

#endif