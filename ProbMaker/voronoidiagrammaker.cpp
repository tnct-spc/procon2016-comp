#include "voronoidiagrammaker.h"
VoronoiDiagramMaker::VoronoiDiagramMaker()
{
    subdiv.initDelaunay(cv::Rect(20, 20, 580, 580));
}

std::vector<cv::Point2f> VoronoiDiagramMaker::generateRandomPoints(int n)
{

    std::random_device seed;
    //メルセンヌ・ツイスタ擬似乱数生成器
    std::mt19937 mt(seed());
    //20〜579の一様分布
    std::uniform_int_distribution<int> distribution(20, 579);
    //opencv版point_xy
    std::vector<cv::Point2f> points;
    for(int i = 0; i < n; i++){
        int x = distribution(mt);
        int y = distribution(mt);
        points.push_back(cv::Point2f(x, y));
    }
    return std::move(points);
}

void VoronoiDiagramMaker::generateVoronoiDiagram(int n)
{
    subdiv.insert(generateRandomPoints(n));
    subdiv.getVoronoiFacetList(id_list, vertex_list, generatrix_list);
}

void VoronoiDiagramMaker::printVoronoiDiagram()
{
    cv::Mat img = cv::Mat::zeros(600, 600, CV_8UC3);
    for(auto list = vertex_list.begin(); list != vertex_list.end(); list++)
    {
         cv::Point2f before = list->back();
         for(auto pt = list->begin(); pt != list->end(); pt++)
         {
             cv::Point p1((int)before.x, (int)before.y);
             cv::Point p2((int)pt->x, (int)pt->y);
             cv::line(img, p1, p2, cv::Scalar(255,255,255));
             before = *pt;
         }
    }
    cv::namedWindow("voronoi");
    cv::imshow("voronoi", img);
    cv::waitKey();
}

std::vector<polygon_t> VoronoiDiagramMaker::cvToBoost()
{
    std::vector<polygon_t> pol((int)(vertex_list.end() - vertex_list.begin()));
    std::vector<polygon_t>::iterator pol_it = pol.begin();
    auto reverse_v_list = vertex_list;
    for (std::vector<cv::Point2f> & v : reverse_v_list) {
        v.push_back(v.at(0));
        std::reverse(v.begin(),v.end());
    }
    for (std::vector<cv::Point2f> const& v : reverse_v_list){
        for (cv::Point2f const& cv_p : v){
            point_t b_p(cv_p.x,cv_p.y);
            pol_it->outer().push_back(b_p);
        }
        pol_it++;
    }
    return std::move(pol);
}

/*
std::vector<point_d> BoostVoronoiDiagramMaker::generateRandomPoints(int n){
    std::random_device seed;
    //メルセンヌ・ツイスタ擬似乱数生成器
    std::mt19937 mt(seed());
    //20〜579の一様分布
    std::uniform_int_distribution<int> distribution(20, 579);
    std::vector<point_d> points;
    for(int i = 0; i < n; i++){
        int x = distribution(mt);
        int y = distribution(mt);
        points.push_back(point_d(x, y));
    }
    return std::move(points);
}

void BoostVoronoiDiagramMaker::generateVoronoiDiagram(int n){
    std::vector<point_d> points = generateRandomPoints(n);
    bp::construct_voronoi(points.begin(),points.end(),&voronoi_diagram);
    for (vertex_t v : voronoi_diagram.vertices()){
    }
}
*/
