#include "probmaker.h"
#include "voronoidiagrammaker.h"
#include "fieldmaker.h"
#include <QApplication>
#include <boost/foreach.hpp>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*
    ProbMaker w;
    w.show();
    w.run();
    */
    VoronoiDiagramMaker v;
    v.generateVoronoiDiagram(50);
    //v.printVoronoiDiagram();
    std::vector<polygon_t> pol  = v.cvToBoost();
    //std::for_each(pol.begin(),pol.end(),[](polygon_t &a){std::cout << bg::area(a) << std::endl;});
    FieldMaker field(pol);
    /*
    polygon_t hoge;
    polygon_t huga,foo,flame;
    std::list<polygon_t> piyo;
    flame.inners().push_back(polygon_t::ring_type());
    flame.inners().back().push_back(point_t(20,20));
    flame.inners().back().push_back(point_t(580,20));
    flame.inners().back().push_back(point_t(580,580));
    flame.inners().back().push_back(point_t(20,580));
    flame.inners().back().push_back(point_t(20,20));
    bg::exterior_ring(hoge) = boost::assign::list_of<point_t>(0,0)(0,300)(300,0)(0,0);
    bg::exterior_ring(huga) = boost::assign::list_of<point_t>(-1,-1)(-2,-1)(-1,-2)(-1,1);
    bg::exterior_ring(flame) = boost::assign::list_of<point_t>(0,0)(0,600)(600,600)(600,0)(0,0);
    bg::difference(hoge,flame,piyo);
    std::cout << bg::area(flame) << std::endl;
    std::cout << bg::area(hoge) << std::endl;
    std::cout << piyo.size() << std::endl;
    */
}
