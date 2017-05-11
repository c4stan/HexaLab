#ifndef _HL_STATS_VIEW_
#define _HL_STATS_VIEW_

#include <iview.h>

namespace HexaLab {
    class StatsView : public IView {
    private:
        vector<float> hexa_quality;

    public:
        StatsView() : IView("Stats") {}

        void set_mesh(js_ptr mesh) {
            this->mesh = (Mesh*)mesh;
            
            hexa_quality.clear();
            hexa_quality.reserve(this->mesh->hexas.size());

            for (int i = 0; i < this->mesh->hexas.size(); ++i) {
                hexa_quality.push_back(this->mesh->hexas[i].scaled_jacobian);
            }
        }

        void update() {}
    
        vector<float>* get_hexa_quality() { return &hexa_quality; }
    };
}

#endif