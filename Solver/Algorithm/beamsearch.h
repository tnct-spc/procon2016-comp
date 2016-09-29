#ifndef BEAMSEARCH_H
#define BEAMSEARCH_H

#include "algorithmwrapper.h"


class BeamSearch : public AlgorithmWrapper
{
private:
protected:
    //ビーム幅
    double beam_width;
    int cpu_num;
    virtual void initialization();
    virtual void evaluateNextMove
    (std::vector<Evaluation> & evaluations,std::vector<procon::Field> const& field_vec);
    virtual std::vector<procon::Field> makeNextField
    (std::vector<Evaluation> const& evaluations,std::vector<procon::Field> const& field_vec);

    bool canPrune(procon::ExpandedPolygon const& next_frame ,double const& min_angle);
public:
    BeamSearch();
    virtual procon::Field run(procon::Field field);
};


#endif // BEAMSEARCH_H
