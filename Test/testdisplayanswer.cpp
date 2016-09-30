#include "testdisplayanswer.h"
#include "field.h"
#include "Utils/polygonconnector.h"
#include "Utils/fit.h"
#include <iostream>
#include "expandedpolygon.h"
#include <cmath>

TestDisplayAnswer::TestDisplayAnswer()
{
}

bool TestDisplayAnswer::run()
{
    polygon_t polygon;


    polygon.inners().push_back(boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>>::ring_type());

    polygon.inners().at(0).push_back(point_t(0.00000002,0.00000009));
    polygon.inners().at(0).push_back(point_t(5.00000006,0.00000005));
    polygon.inners().at(0).push_back(point_t(7.00000009,15.00000002));
    polygon.inners().at(0).push_back(point_t(10.00000006,0.00000002));
    polygon.inners().at(0).push_back(point_t(15.00000004,0.00000005));
    polygon.inners().at(0).push_back(point_t(15.00000006,15.00000009));
    polygon.inners().at(0).push_back(point_t(0.00000003,15.00000006));
    //polygon.inners().at(0).push_back(point_t(0.00004,0.000000009));
    polygon.inners().at(0).push_back(point_t(0.00000003,0.00000002));

    procon::ExpandedPolygon polygont;

    polygont.setPolygon(polygon);

    procon::Field field;

    field.setFlame(polygont);

    //static bool succeedddd;

    Fit fit_sample;

    fit_sample = PolygonConnector::searchFieldConnection(field);

    /*
    std::cout << fit_sample.start_dot_or_line << std::endl;
    std::cout << fit_sample.end_dot_or_line << std::endl;
    std::cout << fit_sample.flame_inner_pos << std::endl;
    std::cout << fit_sample.start_id << std::endl;
    std::cout << fit_sample.end_id << std::endl;
    */

    return true;
}
