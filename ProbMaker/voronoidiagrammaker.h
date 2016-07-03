#ifndef VORONOIDIAGRAMMAKER_H
#define VORONOIDIAGRAMMAKER_H
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/geometry.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
/*---------------------------------------
#include <boost/polygon/voronoi.hpp>
#include <boost/polygon/point_data.hpp>
-----------------------------------------*/
namespace bg = boost::geometry;
using point_t = bg::model::d2::point_xy<double>;
using polygon_t = bg::model::polygon<point_t>;
/*------------------------------------------------------
using point_d = bp::point_data<double>;
namespace bp = boost::polygon;
using cell_t = bp::voronoi_diagram<double>::cell_type;
using vertex_t = bp::voronoi_diagram<double>::vertex_type;
using edge_t = bp::voronoi_diagram<double>::edge_type;
--------------------------------------------------------*/
class VoronoiDiagramMaker
{
    cv::Subdiv2D subdiv;
    std::vector<int> id_list;
    std::vector<std::vector<cv::Point2f>> vertex_list;
    std::vector<cv::Point2f> generatrix_list;
public:
    VoronoiDiagramMaker();
    std::vector<cv::Point2f> generateRandomPoints(int n);
    void generateVoronoiDiagram(int n);
    void printVoronoiDiagram();
    std::vector<polygon_t> cvToBoost();
    
};



//Boostにもvoronoi作るやつがあったけど見なかったことにしよう
/*---------------------------------------------------------
class BoostVoronoiDiagramMaker
{
    bp::voronoi_diagram<double> voronoi_diagram;
public:
    std::vector<point_d> generateRandomPoints(int n);
    void generateVoronoiDiagram(int n);

};
-----------------------------------------------------------*/

#endif // VORONOIDIAGRAMMAKER_H
