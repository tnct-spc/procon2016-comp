#ifndef POLYGONCONNECTOR_H
#define POLYGONCONNECTOR_H

#include "expandedpolygon.h"
#include "field.h"
#include "fit.h"
#include "Utils/fit.h"

using Ring = std::vector<point_t>;

class PolygonConnector
{
public:


    static bool joinPolygon(procon::ExpandedPolygon Polygon1, procon::ExpandedPolygon Polygon2, procon::ExpandedPolygon& new_polygon, std::array<Fit,2> join_data);
private:
    static int increment(int num, int size){return (num + 1) % size;}
    static int decrement(int num, int size){return ((num - 1) % size + size) % size;}

    PolygonConnector();
    static Ring popRingByPolygon(procon::ExpandedPolygon& polygon, int inner_position = -1);
    static void pushRingToPolygon(Ring& ring, procon::ExpandedPolygon& polygon, int inner_position = -1);
    static bool hasConflict(Ring ring1, Ring ring2, Fit fit1, Fit fit2);
    static Fit searchFieldConnection(procon::Field field,procon::ExpandedPolygon polygon);
};

#endif // POLYGONCONNECTOR_H
