#ifndef LENGTHALGORITHM_H
#define LENGTHALGORITHM_H

#include "field.h"
#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "Utils/fit.h"
#include "polygonviewer.h"

class lengthalgorithm
{
private:
    lengthalgorithm();

public:

    // 当てはまるピースのIDをいれるクラス
    struct PieceEdge
    {
        // 空のコンストラクタ
        PieceEdge() {}
        // 破片と辺番号を指定してオブジェクトを作成
        PieceEdge(int _piece, int _edge) :
            piece(_piece),
            edge(_edge)
        {}

    public:
        int piece;
        int edge;
    };

    static  void searchPairSide(double remaining_length, int watched_piece);

    static std::vector<std::vector<PieceEdge>> fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces);

    static void test();

};

#endif // LENGTHALGORITHM_H
