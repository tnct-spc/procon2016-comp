#ifndef FIELDMAKER_H
#define FIELDMAKER_H
#include <boost/geometry.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include "field.h"
namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_t;
typedef bg::model::polygon<point_t> polygon_t;

class FieldMaker
{
public:
    FieldMaker(std::vector<polygon_t> const& pol);
};

#endif // FIELDMAKER_H
