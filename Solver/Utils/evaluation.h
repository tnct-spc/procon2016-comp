#ifndef EVALUATION_H
#define EVALUATION_H

#include "Utils/fit.h"
#include "field.h"
#include <array>

class Evaluation
{
public:
    Evaluation();
    std::array<Fit,2> fits;
    int piece_id;
    int frame_id;
    int vector_id;
    bool inverse_flag;
    double evaluation;
};

#endif // EVALUATION_H
