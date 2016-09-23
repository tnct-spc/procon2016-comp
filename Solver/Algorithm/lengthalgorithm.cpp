#include "lengthalgorithm.h"

#include "utilities.h"
#include "field.h"

lengthalgorithm::lengthalgorithm()
{
    test();
}

// ピースの情報の入った配列
std::vector<procon::ExpandedPolygon> g_pieces;

// func()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
std::vector<lengthalgorithm::PieceEdge> g_comb;

// 組み合わせを保存する配列
std::vector<std::vector<lengthalgorithm::PieceEdge>> g_stack;

// rl : フレーム辺の残りの長さ
// pi : 破片番号。g_pieces[]のインデックス。
void lengthalgorithm::searchPairSide(double remaining_length, int watched_piece)
{
    // フレーム辺の長さに破片がぴったり合ったら表示して、再帰から抜ける。
    if (fabs(remaining_length) < 0.001)
    {
        // 破片とその辺の組み合わせを保存
        g_stack.push_back(g_comb);
        return;
    }

    // すべての破片の組み合わせを試してたら再帰から抜ける。
    if (g_pieces.size() <= watched_piece)
    {
        return;
    }

    // 破片の各辺を入れて再帰する
    procon::ExpandedPolygon piece;
    piece.updatePolygon();

    // 配列からExpolygonを一つ取り出す
    piece = g_pieces[watched_piece];
    for (int e = 0; e < piece.getSize(); e++)
    {
        // フレーム辺の残りの長さより破片の辺が短ければ入れてみる。
        double l = piece.getSideLength()[e];
        if (l <= remaining_length)
        {
            // この破片と辺をスタックに積む
            // 実際のピースの情報を使う際にはpiはピースのIDに変える
            // pi_id = piece.getId();
            g_comb.push_back(PieceEdge(watched_piece, e));
            // 次の破片へ再帰
            searchPairSide(remaining_length - l, watched_piece + 1);
            // スタックから取り除く。
            g_comb.pop_back();
        }
    }

    // この破片は入れずに、次の破片へ再帰する。
    searchPairSide(remaining_length, watched_piece + 1);
}

// 扱い易いようにグローバル関数をローカル関数に変換するための関数
std::vector<std::vector<lengthalgorithm::PieceEdge>> lengthalgorithm::fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces)
{
    // ピースの情報をグローバル化し、実行
    g_pieces = pieces;
    searchPairSide(frame,0);

    // 組合せの保存されたグローバル関数をローカル関数にし返す
    std::vector<std::vector<PieceEdge>> box;
    box = g_stack;

    return box;
}

void lengthalgorithm::test()
{
    // テストデータをセットアップ
    procon::ExpandedPolygon polygon1(0);
    procon::ExpandedPolygon polygon2(0);
    procon::ExpandedPolygon polygon3(0);
    procon::ExpandedPolygon polygon4(0);

    polygon_t sample11;
    sample11.outer().push_back(point_t(0,0));
    sample11.outer().push_back(point_t(0,3));
    sample11.outer().push_back(point_t(2,2));
    sample11.outer().push_back(point_t(2,0));
    sample11.outer().push_back(point_t(0,0));

    polygon_t sample12;
    sample12.outer().push_back(point_t(3,2));
    sample12.outer().push_back(point_t(2,2));
    sample12.outer().push_back(point_t(0,3));
    sample12.outer().push_back(point_t(0,4));
    sample12.outer().push_back(point_t(3,4));
    sample12.outer().push_back(point_t(3,2));

    polygon_t sample13;
    sample13.outer().push_back(point_t(6,0));
    sample13.outer().push_back(point_t(2,0));
    sample13.outer().push_back(point_t(2,2));
    sample13.outer().push_back(point_t(3,2));
    sample13.outer().push_back(point_t(3,4));
    sample13.outer().push_back(point_t(6,0));

    polygon1.setPolygon(sample11);
    polygon2.setPolygon(sample12);
    polygon3.setPolygon(sample13);

    std::vector<procon::ExpandedPolygon> pieces;
    pieces.push_back(polygon1);
    pieces.push_back(polygon2);
    pieces.push_back(polygon3);

    std::vector<double> frames;
    frames.push_back(4.0);
    frames.push_back(3.0);
    frames.push_back(5.0);
    frames.push_back(6.0);

    //フレームの辺の長さとExpolygonの配列を入れると、ぴったりとはまる辺の組合せの配列が返ってくる
    std::vector<std::vector<std::vector<PieceEdge>>> stacks;

    for (int f=0; f<frames.size(); f++)
    {
        stacks.push_back(fitSide(frames[f],pieces));
    }

    printf("OK");
}
