#include "lengthalgorithm.h"

#include "utilities.h"
#include "field.h"

lengthalgorithm::lengthalgorithm()
{
}

procon::Field lengthalgorithm::run(procon::Field field)
{
    //fake
    return field;
}

// rl : フレーム辺の残りの長さ
// pi : 破片番号。g_pieces[]のインデックス。
void lengthalgorithm::searchPairSide(double remaining_length, int watched_piece)
{

    // フレーム辺の長さに破片がぴったり合ったら表示して、再帰から抜ける。
    if (fabs(remaining_length) < 0.001)
    {

        // 破片とその辺の組み合わせを保存
        g_frame_stack.push_back(g_comb);
        return;
    }

    // すべての破片の組み合わせを試してたら再帰から抜ける。
    if ((int)g_pieces.size() <= watched_piece)
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
lengthalgorithm::frame_edge_set_type lengthalgorithm::fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces)
{

    // ピースの情報をグローバル化し、実行
    g_pieces = pieces;
    g_frame_stack.clear();
    searchPairSide(frame,0);

    // 組合せの保存されたグローバル関数をローカル関数にし返す
    frame_edge_set_type return_stack;
    return_stack = g_frame_stack;

    return return_stack;
}

// ピースの組み合わせをすべてのパターンで並び替える。
// focus : 並び替える組み合わせの配列での番号
void lengthalgorithm::sortPieces(int focus)
{
    // もし、最後まで調べ終わったら戻る
    if ((int)(g_pieces_sorted.size()) <= focus)
    {
        this->g_sort_list.push_back(g_pieces_sorted);
        return;
    }

    // 破片を並び替えながら再帰していく
    for (int i = focus; i < (int)g_pieces_sorted.size(); i++)
    {
        std::swap(g_pieces_sorted[focus], g_pieces_sorted[i]);
        sortPieces(focus + 1);
        std::swap(g_pieces_sorted[focus], g_pieces_sorted[i]);
    }
}

// 受け取った組み合わせをすべて並び替える。
// stacks : 並び替える組み合わせの配列
std::vector<lengthalgorithm::frame_edge_set_type> lengthalgorithm::piecesAlignmentSequence(std::vector<lengthalgorithm::frame_edge_set_type> stacks)
{
    std::vector<frame_edge_set_type> sort_lists;
    for (int f=0; f<(int)stacks.size(); f++)
    {
        for (int c=0; c<(int)stacks[f].size(); c++)
        {
            g_pieces_sorted = stacks[f][c];
            sortPieces(0);
        }
        sort_lists.push_back(g_sort_list);

        // 毎フレームごとに中身をクリア
        g_sort_list.clear();
    }
    return sort_lists;
}

void lengthalgorithm::test()
{
    // テストデータをセットアップ
    procon::ExpandedPolygon polygon1(0);
    procon::ExpandedPolygon polygon2(0);
    procon::ExpandedPolygon polygon3(0);

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

    // 組み合わせが全て入る配列
    std::vector<frame_edge_set_type> stacks;

    // フレームの長さぴったりのピースと辺の組み合わせを探す。
    for (int f=0; f<(int)frames.size(); f++)
    {
        stacks.push_back(fitSide(frames[f],pieces));
    }

    // 前に出てきた組み合わせを全パターンに並び替える。
    std::vector<frame_edge_set_type> sort_list;
    sort_list = piecesAlignmentSequence(stacks);

}
