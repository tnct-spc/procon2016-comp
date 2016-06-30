#include "voronoidiagrammaker.h"

VoronoiDiagramMaker::VoronoiDiagramMaker()
{
    subdiv.initDelaunay(cv::Rect(0, 0, 600, 600));
}

std::vector<cv::Point2f> VoronoiDiagramMaker::generateRandomPoints(int n)
{

    std::random_device seed;
    //メルセンヌ・ツイスタ擬似乱数生成器
    std::mt19937 mt(seed());
    //0〜599の一様分布
    std::uniform_int_distribution<int> distribution(0, 599);
    //opencv版point_xy
    std::vector<cv::Point2f> points;
    for(int i = 0; i < n; i++){
        int x = distribution(mt);
        int y = distribution(mt);
        points.push_back(cv::Point2f(x, y));
    }
    return std::move(points);
}

void VoronoiDiagramMaker::generateVoronoiDiagram()
{
    subdiv.insert(generateRandomPoints(60));
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

std::vector<polygon_t> cvToBoost(std::vector<std::vector<cv::Point2f>> const& vertex_list)
{
    for (std::vector<cv::Point2f> i : vertex_list){
        for (cv::Point2f j : i){
            point_t p(j.x,j.y);
        }
    }
}
