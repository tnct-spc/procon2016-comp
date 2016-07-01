#ifndef VORONOIDIAGRAMMAKER_H
#define VORONOIDIAGRAMMAKER_H
#include <iostream>
#include <random>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/geometry.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
namespace bg = boost::geometry;
typedef bg::model::d2::point_xy<double> point_t;
typedef bg::model::polygon<point_t> polygon_t;

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

#endif // VORONOIDIAGRAMMAKER_H
