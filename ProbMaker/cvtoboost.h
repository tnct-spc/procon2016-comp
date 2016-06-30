#ifndef CVTOBOOST_H
#define CVTOBOOST_H
#include <iostream>
#include <vector>

class CvToBoost
{
public:
    CvToBoost(std::vector<std::vector<cv::Point2f>> const vertex_list);
};

#endif // CVTOBOOST_H
