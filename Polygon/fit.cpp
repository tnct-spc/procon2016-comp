#include "fit.h"

Fit::Fit()
{

}

bool Fit::operator==(Fit const& fit){
    return (this->start_dot_or_line == fit.start_dot_or_line &&
            this->end_dot_or_line == fit.end_dot_or_line &&
            this->start_id == fit.start_id &&
            this->end_id == fit.end_id &&
            this->frame_inner_pos == fit.frame_inner_pos &&
            this->is_end_straight  == fit.is_end_straight &&
            this->is_start_straight == fit.is_start_straight);
}
