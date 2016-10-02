#include "lengthalgorithm.h"

#include "utilities.h"
#include "field.h"

// PIを使うため
#define _USE_MATH_DEFINES
#include <math.h>

lengthalgorithm::lengthalgorithm()
{
    test();
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
std::vector<std::vector<lengthalgorithm::PieceEdge>> lengthalgorithm::fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces)
{

    // ピースの情報をグローバル化し、実行
    g_pieces = pieces;
    searchPairSide(frame,0);

    // 組合せの保存されたグローバル関数をローカル関数にし返す
    std::vector<std::vector<PieceEdge>> return_stack;
    return_stack = g_frame_stack;

    return return_stack;
}

// ピースの組み合わせをすべてのパターンで並び替える。
// focus : 並び替える組み合わせの配列での番号
void lengthalgorithm::sortPieces(int focus)
{
    // もし、最後まで調べ終わったら戻る
    if ((int)(g_sorted_pieces.size()) <= focus)
    {
        this->g_sort_list.push_back(g_sorted_pieces);
        return;
    }

    // 破片を並び替えながら再帰していく
    for (int i = focus; i < (int)g_sorted_pieces.size(); i++)
    {
        std::swap(g_sorted_pieces[focus], g_sorted_pieces[i]);
        sortPieces(focus + 1);
        std::swap(g_sorted_pieces[focus], g_sorted_pieces[i]);
    }
}

// 受け取った組み合わせをすべて並び替える。
// stacks : 並び替える組み合わせの配列
std::vector<std::vector<std::vector<lengthalgorithm::PieceEdge>>> lengthalgorithm::piecesAlignmentSequence(std::vector<std::vector<std::vector<lengthalgorithm::PieceEdge>>> stacks)
{
    std::vector<std::vector<std::vector<PieceEdge>>> sort_lists;
    for (int f=0; f<(int)stacks.size(); f++)
    {
        for (int c=0; c<(int)stacks[f].size(); c++)
        {
            g_sorted_pieces = stacks[f][c];
            sortPieces(0);
        }
        sort_lists.push_back(g_sort_list);
        
        // 毎フレームごとに中身をクリア
        g_sort_list.clear();
    }
    return sort_lists;
}

// 隣同士の破片が重なる場合リストから削除
// com_num : 調べる並び順の番号
int lengthalgorithm::clearOverlap(int frame,int com_num)
{
    // その並び順が削除されたかを確認
    int check=0;

    // ピースが隣り合っているところを調べる
    for (int p = 0; p < (int)g_cleared_sort[frame][com_num].size() - 1; p++)
    {
        // 隣り合った左右の破片の隣り合う角を調べ180度を越さないか確かめる
        procon::ExpandedPolygon Polygon1 = g_pieces[g_cleared_sort[frame][com_num][p].piece];
        procon::ExpandedPolygon Polygon2 = g_pieces[g_cleared_sort[frame][com_num][p + 1].piece];
        Polygon1.updatePolygon();
        Polygon2.updatePolygon();
        double deg1 = Polygon1.getSideAngle()[g_cleared_sort[frame][com_num][p].edge] * 180 / M_PI;
        double deg2 = Polygon2.getSideAngle()[(g_cleared_sort[frame][com_num][p + 1].edge + 1) % Polygon2.getSize()] * 180 / M_PI;

        // 破片同士が重なっていたら削除
        if ((180 - deg1 - deg2) < -1.0)
        {
            g_cleared_sort[frame].erase(g_cleared_sort[frame].begin() + com_num);
            check = 1;
            return check;
        }
    }
    return check;
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
    sample11.outer().push_back(point_t(0,4));
    sample11.outer().push_back(point_t(2,0));
    sample11.outer().push_back(point_t(0,0));

    polygon_t sample12;
    sample12.outer().push_back(point_t(2,0));
    sample12.outer().push_back(point_t(1,2));
    sample12.outer().push_back(point_t(3,2));
    sample12.outer().push_back(point_t(3,0));
    sample12.outer().push_back(point_t(2,0));

    polygon_t sample13;
    sample13.outer().push_back(point_t(1,2));
    sample13.outer().push_back(point_t(0,4));
    sample13.outer().push_back(point_t(4,4));
    sample13.outer().push_back(point_t(5,0));
    sample13.outer().push_back(point_t(3,0));
    sample13.outer().push_back(point_t(3,2));
    sample13.outer().push_back(point_t(1,2));

    polygon_t sample14;
    sample14.outer().push_back(point_t(5,0));
    sample14.outer().push_back(point_t(4,4));
    sample14.outer().push_back(point_t(6,4));
    sample14.outer().push_back(point_t(6,0));
    sample14.outer().push_back(point_t(5,0));

    polygon1.setPolygon(sample11);
    polygon2.setPolygon(sample12);
    polygon3.setPolygon(sample13);
    polygon4.setPolygon(sample14);

    std::vector<procon::ExpandedPolygon> pieces;
    pieces.push_back(polygon1);
    pieces.push_back(polygon2);
    pieces.push_back(polygon3);
    pieces.push_back(polygon4);

    std::vector<double> frames;
    frames.push_back(6.0);
    frames.push_back(4.0);
    frames.push_back(6.0);
    frames.push_back(4.0);

    // フレームの長さぴったりのピースと辺の組み合わせを探す。
    for (int f=0; f<(int)frames.size(); f++)
    {
        g_stacks.push_back(fitSide(frames[f],pieces));
        
        // 毎フレームごとにクリア
        g_frame_stack.clear();
    }

    // 前に出てきた組み合わせを全パターンに並び替える。
    std::vector<std::vector<std::vector<PieceEdge>>> sort_list;
    sort_list = piecesAlignmentSequence(g_stacks);

    g_cleared_sort = sort_list;

    // 並び順が削除されたら同じ番号でfuncを繰り返す
    int count = 0;
    //int comb_size = (int)g_comb.size();

    // すべての組み合わせについて調べる
    for (int f=0; f<(int)g_cleared_sort.size(); f++)
    {
        for (int e = 0; e < (int)g_cleared_sort[f].size(); e++)
        {
            int check = clearOverlap(f,count);

            // もし削除されていなかったら次の番号で実行
            if (check == 0) count++;
        }
        count = 0;
    }

    printf("OK");
}
