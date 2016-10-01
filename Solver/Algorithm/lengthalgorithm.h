#ifndef LENGTHALGORITHM_H
#define LENGTHALGORITHM_H

#include "field.h"
#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "Utils/fit.h"
#include "polygonviewer.h"

class lengthalgorithm
{
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

        int piece;
        int edge;
    };
private:
    std::vector<PieceEdge> g_pieces_sorted;
    std::vector<std::vector<PieceEdge>> g_sort_list;
    // ピースの情報の入った配列
    std::vector<procon::ExpandedPolygon> g_pieces;
    // func()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
    std::vector<lengthalgorithm::PieceEdge> g_comb;
    // 組み合わせを保存する配列
    std::vector<std::vector<lengthalgorithm::PieceEdge>> g_frame_stack;
    //フレームの辺の長さとExpolygonの配列を入れると、ぴったりとはまる辺の組合せの配列が返ってくる
    std::vector<std::vector<std::vector<PieceEdge>>> g_stacks;

public:

    lengthalgorithm();

    void searchPairSide(double remaining_length, int watched_piece);

    std::vector<std::vector<PieceEdge>> fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces);

    void sortPieces(int watched_list);

    std::vector<std::vector<std::vector<PieceEdge>>> piecesAlignmentSequence(std::vector<std::vector<std::vector<PieceEdge>>> stacks);

    void test();

};

#endif // LENGTHALGORITHM_H
