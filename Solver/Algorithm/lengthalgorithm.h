#ifndef LENGTHALGORITHM_H
#define LENGTHALGORITHM_H

#include "field.h"
#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "fit.h"
#include "polygonviewer.h"
#include "algorithmwrapper.h"

class lengthalgorithm : public AlgorithmWrapper
{
public:
    lengthalgorithm();

    void run(procon::Field field);

    procon::Field test(procon::Field field);

private:
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

    // sortPiecesで並び替える組み合わせを入れる配列。
    std::vector<PieceEdge> g_sorted_pieces;

    //ピース辺の組み合わせ
    typedef std::vector<PieceEdge> piece_edges_type;

    //ピース辺の組み合わせの全パターン
    typedef std::vector<piece_edges_type> frame_edge_set_type;

    /*フレームの辺にぴったりはまるピース辺の組み合わせをを全パターン探す*/
    void searchPairSide(double remaining_length, int watched_piece);

    /*fitSide再帰関数で使用*/
    // ピースの情報の入った配列
    std::vector<procon::ExpandedPolygon> g_pieces;

    procon::ExpandedPolygon g_frame;

    // searchPairSide()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
    piece_edges_type g_comb;

    // フレームごとの組み合わせ全てを保存する配列
    std::vector<std::vector<PieceEdge>> g_frame_stack;

    // フレームごとの組み合わせ全てを保存する配列
    std::vector<std::vector<PieceEdge>> g_frame_ok;

    // 組み合わせが全て入る配列
    std::vector<std::vector<std::vector<PieceEdge>>> g_stacks;

    std::vector<std::vector<std::vector<PieceEdge>>> g_cleared_sort;

    //connect pieces
    void bomPush(procon::Field& field, int frame_inner_pos, int frame_number, std::vector<PieceEdge> edges);

public:

    void sortPieces(int watched_list);

    // フレームごとの並び替えた組み合わせがすべて入る配列
    frame_edge_set_type g_frame_sort;

    void clearCorner(int frame,int com_num);

    void clearOverlap(int frame,int com_num);

    void clearEnd(int frame,int com_num);
};

#endif // LENGTHALGORITHM_H
