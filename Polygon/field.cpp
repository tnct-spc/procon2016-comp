#include "field.h"

Field::Field()
{
    fieldPiece.reserve(50);
}

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

int Field::pieceSize(){
    return (int)(fieldPiece.end() - fieldPiece.begin());
}

void Field::printFlame(){
    std::cout << bg::dsv(fieldFlame.getPolygon()) << std::endl;
}

void Field::printPiece(){
    std::for_each(fieldPiece.begin(),fieldPiece.end(),[](PolygonExpansion &a){std::cout << bg::dsv(a.getPolygon()) << std::endl;});
}
