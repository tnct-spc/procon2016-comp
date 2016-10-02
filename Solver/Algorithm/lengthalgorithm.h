#ifndef LENGTHALGORITHM_H
#define LENGTHALGORITHM_H

#include "field.h"
#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "Utils/fit.h"
#include "polygonviewer.h"
#include "algorithmwrapper.h"

class lengthalgorithm : public AlgorithmWrapper
{
public:
    lengthalgorithm();

    procon::Field run(procon::Field field);

    void test();

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

    // sortPiecesで並び替える組み合わせを入れる配列。
    std::vector<PieceEdge> g_sorted_pieces;
    
    // フレームごとの並び替えた組み合わせがすべて入る配列
    std::vector<std::vector<PieceEdge>> g_sort_list;

    //ピース辺の組み合わせ
    typedef std::vector<PieceEdge> piece_edges_type;

    //ピース辺の組み合わせの全パターン
    typedef std::vector<piece_edges_type> frame_edge_set_type;

    /*フレームの辺にぴったりはまるピース辺の組み合わせをを全パターン探す*/
    frame_edge_set_type fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces);
    void searchPairSide(double remaining_length, int watched_piece);

    /*fitSide再帰関数で使用*/
    // ピースの情報の入った配列
    std::vector<procon::ExpandedPolygon> g_pieces;

    // searchPairSide()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
    piece_edges_type g_comb;

    // フレームごとの組み合わせ全てを保存する配列
    std::vector<std::vector<PieceEdge>> g_frame_stack;
    
    // 組み合わせが全て入る配列
    std::vector<std::vector<std::vector<PieceEdge>>> g_stacks;

    std::vector<std::vector<std::vector<PieceEdge>>> g_cleared_sort;

public:

    frame_edge_set_type g_frame_stack;

    /*ピース辺の組み合わせを、すべての順番で作って返す*/
    std::vector<frame_edge_set_type> piecesAlignmentSequence(std::vector<frame_edge_set_type> stacks);

    void sortPieces(int watched_list);

    /*pieceAlignmentSequenceの再帰関数で使用*/
    // sortPiecesで並び替える組み合わせを入れる配列。
    piece_edges_type g_pieces_sorted;

    // フレームごとの並び替えた組み合わせがすべて入る配列
    frame_edge_set_type g_sort_list;

    int clearOverlap(int frame,int com_num);

};

#endif // LENGTHALGORITHM_H
