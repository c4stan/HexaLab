#ifndef _HL_LOW_QUALITY_VIEW_H_
#define _HL_LOW_QUALITY_VIEW_H_

#include <model.h>
#include <mesh.h>
#include <iview.h>

namespace HexaLab {
    class LowQualityView : public IView {
    private:
        Model lowq_model;
        Model hidden_model;

        void add_lowq_face(Dart& dart, float normal_sign);
        void add_lowq_wireframe(Dart& dart);
        void add_hidden_face(Dart& dart);
        void add_hidden_wireframe(Dart& dart);

        vector<uint32_t> edge_marks;

        float mark = 0;

    public:
        LowQualityView() : IView("Low Quality View") {}

        void set_mesh(js_ptr mesh);
        void update();

        float quality_threshold;

        Model& get_low_quality_model() { return lowq_model; }
        Model& get_hidden_model() { return hidden_model; }
    };
}

#endif