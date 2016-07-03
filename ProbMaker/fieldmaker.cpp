#include "fieldmaker.h"

FieldMaker::FieldMaker(std::vector<polygon_t> const& polygon)
{
    makeFlame();
    limitPieces(polygon);
}
void FieldMaker::limitPieces(std::vector<polygon_t> const& polygon){
    polygon_t f = flame;
    for(polygon_t pol : polygon){
        std::vector<polygon_t> dif_pieces;
        bg::difference(pol,f,dif_pieces);
        if (static_cast<int>(dif_pieces.size()) > 1){
            for(polygon_t dp : dif_pieces){
                bool useless_flag = false;
                for(point_t p : dp.outer()){
                    double x = p.get<0>();
                    double y = p.get<1>();
                    if (x >= 600 || x <= 0 || y >= 600 || y <= 0) useless_flag = true;
                }
                if (!useless_flag){
                    pieces.push_back(dp);
                    exterior_pieces.push_back(std::move(dp));
                    break; //これ以上forループを回す意味がないため
                }
            }
        } else {
            pieces.push_back(std::move(dif_pieces.at(0)));
        }
    }
    for (polygon_t p : pieces) std::cout << bg::dsv(p) << std::endl;
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
