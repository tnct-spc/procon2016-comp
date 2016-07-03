#ifndef FIELDMAKER_H
#define FIELDMAKER_H
#include <random>
#include <cmath>
#include <boost/geometry.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_t;
typedef bg::model::polygon<point_t> polygon_t;

class FieldMaker
{
    polygon_t flame;
    std::vector<polygon_t> pieces;
    std::vector<polygon_t> exterior_pieces;
public:
    FieldMaker(std::vector<polygon_t> const& pol);
    void limitPieces(std::vector<polygon_t> const& polygon);
    void makeFlame();
    void unionFlameAndPieces(int probability);
};

namespace comp {
    double round(double a,int n);
}
#endif // FIELDMAKER_H
