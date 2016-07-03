#include "fieldmaker.h"

FieldMaker::FieldMaker(std::vector<polygon_t> const& polygon)
{
    makeFlame();
    limitPieces(polygon);
    unionFlameAndPieces(50);
}
void FieldMaker::limitPieces(std::vector<polygon_t> const& polygon){
    polygon_t f = flame;
    for(polygon_t pol : polygon){
        std::vector<polygon_t> dif_pieces;
        bg::difference(pol,f,dif_pieces);
        if (static_cast<int>(dif_pieces.size()) > 1){
            for(polygon_t dp : dif_pieces){
                bool useless_flag = false;
                for(point_t & p : dp.outer()){
                    double x = comp::round(p.x(),2);
                    double y = comp::round(p.y(),2);
                    p.x(x);
                    p.y(y);
                    if (x >= 600 || x <= 0 || y >= 600 || y <= 0) useless_flag = true;
                }
                if (!useless_flag){
                    //pieces.push_back(dp);
                    exterior_pieces.push_back(dp);
                    break; //これ以上forループを回す意味がないため
                }
            }
        } else {
            pieces.push_back(dif_pieces.at(0));
        }
    }
    //for (polygon_t p : pieces) std::cout << bg::dsv(p) << std::endl;
}

void FieldMaker::makeFlame(){
    bg::exterior_ring(flame) = boost::assign::list_of<point_t>(0,0)(0,600)(600,600)(600,0)(0,0);
    flame.inners().push_back(polygon_t::ring_type());
    flame.inners().back().push_back(point_t(20,20));
    flame.inners().back().push_back(point_t(580,20));
    flame.inners().back().push_back(point_t(580,580));
    flame.inners().back().push_back(point_t(20,580));
    flame.inners().back().push_back(point_t(20,20));
}

void FieldMaker::unionFlameAndPieces(int probability){
    std::random_device seed;
    //メルセンヌ・ツイスタ擬似乱数生成器
    std::mt19937 mt(seed());
    //0〜99の一様分布
    std::uniform_int_distribution<int> rand100(0,99);
    std::vector<polygon_t> tmp;
    std::cout << "flame" << bg::dsv(flame) << std::endl;
    for (polygon_t p : exterior_pieces){
        if (probability > rand100(mt)){
            tmp.clear();
            bg::union_(p,flame,tmp);
            flame = std::move(tmp.at(0));
        } else {
            pieces.push_back(p);
        }
    }
    std::cout << "flame" << bg::dsv(flame) << std::endl;
}

double comp::round(double a, int n){
    a *= std::pow(10,n);
    a = std::round(a);
    a *= std::pow(10,-n);
    return a;
}

