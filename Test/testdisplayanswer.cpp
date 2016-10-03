#include "testdisplayanswer.h"
#include "field.h"
#include "Utils/polygonconnector.h"
#include <iostream>
#include "expandedpolygon.h"
#include <cmath>
#include <tuple>

TestDisplayAnswer::TestDisplayAnswer()
{
}

bool TestDisplayAnswer::run()
{
    polygon_t polygon;


    polygon.inners().push_back(boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>>::ring_type());

    polygon.inners().at(0).push_back(point_t(0.00000002,0.00000009));
    polygon.inners().at(0).push_back(point_t(5.000000096,0.00000005));
    polygon.inners().at(0).push_back(point_t(7.00000009,15.00000002));
    polygon.inners().at(0).push_back(point_t(10.00000006,0.00000002));
    polygon.inners().at(0).push_back(point_t(15.00000004,0.00000005));
    polygon.inners().at(0).push_back(point_t(15.00000006,15.00000009));
    polygon.inners().at(0).push_back(point_t(0.00000003,15.00000006));
    //polygon.inners().at(0).push_back(point_t(0.00004,0.000000009));
    polygon.inners().at(0).push_back(point_t(0.00000002,0.00000009));

    procon::ExpandedPolygon polygont;

    polygont.resetPolygonForce(polygon);

    procon::Field field;

    field.setFrame(polygont);

    std::tuple<bool,bool,int,int> result = PolygonConnector::searchFieldConnection(field);

    std::cout << "success:" << std::get<0>(result) << std::endl;
    std::cout << "dot or line:" << std::get<1>(result) << "(true:dot,false:line)"<< std::endl;
    std::cout << "big number:" << std::get<2>(result) << std::endl;
    std::cout << "small number:" << std::get<3>(result) << std::endl;

    /*
<<<<<<< HEAD
    std::cout << fit_sample.start_dot_or_line << std::endl;
    std::cout << fit_sample.end_dot_or_line << std::endl;
    std::cout << fit_sample.flame_inner_pos << std::endl;
    std::cout << fit_sample.start_id << std::endl;
    std::cout << fit_sample.end_id << std::endl;
=======
    std::cout << "Hello,display!" << std::endl;
    polygon_t b,c,d;
    bg::exterior_ring(b) = boost::assign::list_of<point_t>(0, 0)(30, 0)(30,30)(0, 30)(0, 2)(2, 2)(2,28)(28, 28)(28, 2)(0, 2)(0,0);
    bg::exterior_ring(c) = boost::assign::list_of<point_t>(0, 0)(0,1)(1,0)(0,0);
    Field f;
    PolygonExpansion hoge,huga;
    hoge.resetPolygonForce(b);
    huga.resetPolygonForce(c);
    f.setFrame(hoge);
    f.pushPiece(huga);
    f.setPiece(huga,1919);
    f.printFrame();
    f.printPiece();
    disp->setField(f);
>>>>>>> develop
    */

    return true;
}
