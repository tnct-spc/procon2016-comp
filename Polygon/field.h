#ifndef FIELD_H
#define FIELD_H
#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include "polygonexpansion.h"
namespace bg = boost::geometry;
using point_t = bg::model::d2::point_xy<double>;
using polygon_t = bg::model::polygon<point_t>;


class Field
{
protected:
    PolygonExpansion field_flame;
    std::vector<PolygonExpansion> field_piece;
public:
    Field();
    //setterとgetter
    void setFlame(const PolygonExpansion &flame); 
    void setPiece(const PolygonExpansion &piece,const int &n);
    void pushPiece(const PolygonExpansion &piece);
    PolygonExpansion popPiece();
    PolygonExpansion getPiece(const int &n) ;
    PolygonExpansion getFlame() ;
    //fieldPieceにセットされているピースの数
    int pieceSize();
    //コンソール出力
    void printFlame();
    void printPiece();
};

#endif // FIELD_H
