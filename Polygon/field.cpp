#include "field.h"

Field::Field()
{
    field_piece.reserve(50);
}
void Field::setFlame(const PolygonExpansion &flame){
    field_flame = flame;
}

void Field::setPiece(const PolygonExpansion &piece,const int &n){
    if (n < pieceSize()) field_piece[n] = piece;
    else pushPiece(piece);
}

void Field::pushPiece(const PolygonExpansion &piece){
    field_piece.push_back(piece);
}

PolygonExpansion Field::popPiece(){
    PolygonExpansion tmp = field_piece.back();
    field_piece.pop_back();
    return tmp;
}

PolygonExpansion Field::getPiece(const int &n){
    return field_piece[n];
}

PolygonExpansion Field::getFlame(){
    return field_flame;
}


/*後方互換*/
/*
void Field::setFlame(const polygon_t &flame){
    fieldFlame.setPolygon(flame);
}

void Field::setPiece(const polygon_t &piece,const int &n){
    if (n < pieceSize()) fieldPiece[n].setPolygon(piece);
    else pushPiece(piece);
}

void Field::pushPiece(const polygon_t &piece){
    PolygonExpansion tmp;
    tmp.setPolygon(piece);
    fieldPiece.push_back(tmp);
}

polygon_t Field::popPiece(){
    PolygonExpansion tmp = fieldPiece.back();
    fieldPiece.pop_back();
    return tmp.getPolygon();
}

polygon_t Field::getFlame() {
    return fieldFlame.getPolygon();
}

polygon_t Field::getPiece(const int &n) {
    return fieldPiece[n].getPolygon();
}
*/
/*ここまで*/

int Field::pieceSize(){
    return (int)(field_piece.size());
}

void Field::printFlame(){
    std::cout << bg::dsv(field_flame.getPolygon()) << std::endl;
}

void Field::printPiece(){
    std::for_each(field_piece.begin(),field_piece.end(),[](PolygonExpansion &a){std::cout << bg::dsv(a.getPolygon()) << std::endl;});
}
