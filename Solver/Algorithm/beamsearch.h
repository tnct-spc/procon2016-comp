#ifndef BEAMSEARCH_H
#define BEAMSEARCH_H

#include "algorithmwrapper.h"


class BeamSearch : public AlgorithmWrapper
{
private:
protected:
    //ビーム幅
    int beam_width;
    int cpu_num;
    int variety_width;
    void initialization();
    virtual void evaluateNextMove
    (std::vector<Evaluation> & evaluations,std::vector<procon::Field> const& field_vec);
    virtual std::vector<procon::Field> makeNextField
    (std::vector<Evaluation> const& evaluations,std::vector<procon::Field> const& field_vec);
    bool removeDuplicateField(std::vector<procon::Field> & field_vec);
    bool canPrune(procon::ExpandedPolygon const& next_frame ,double const& min_angle);
public:
    BeamSearch();
    void run(procon::Field field);

    bool alpha_is_none = false;
    bool beta_is_none = true;
    bool gamma_is_none = false;
    bool delta_is_none = false;

    double alpha = 1;
    double beta = 1;
    double gamma = 4;
    double delta = 1;
};


#endif // BEAMSEARCH_H
