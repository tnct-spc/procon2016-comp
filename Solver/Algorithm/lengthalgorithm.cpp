#include "lengthalgorithm.h"

#include "utilities.h"
#include "field.h"

// PIを使うため
#define _USE_MATH_DEFINES
#include <math.h>

lengthalgorithm::lengthalgorithm()
{

}

void lengthalgorithm::run(procon::Field field)
{
    field.setFrame(field.getElementaryFrame());
    for (int p=0; p<9; p++)
    {
        g_pieces.push_back(field.getElementaryPieces().at(p));
    }

    for (int a=0; a<field.getElementaryFrame().getSize(); a++)
    {
        g_frame = field.getElementaryFrame();
        test();
    }
}

// rl : フレーム辺の残りの長さ
// pi : 破片番号。g_pieces[]のインデックス。
void lengthalgorithm::searchPairSide(double remaining_length, int watched_piece)
{

    // フレーム辺の長さに破片がぴったり合ったら表示して、再帰から抜ける。
    if (fabs(remaining_length) < 0.1)
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
    piece = g_pieces[watched_piece];
    piece.updatePolygon();

    // 配列からExpolygonを一つ取り出す
    for (int e = 0; e < piece.getSize(); e++)
    {
        // フレーム辺の残りの長さより破片の辺が短ければ入れてみる。
        double l = piece.getSideLength()[e];
        if (l <= remaining_length)
        {
            // この破片と辺をスタックに積む
            // 実際のピースの情報を使う際にはpiはピースのIDに変える
            // pi_id = piece.getId();
            g_comb.push_back(PieceEdge(piece.getId(), e));

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
lengthalgorithm::frame_edge_set_type lengthalgorithm::fitSide(double frame)
{
    // ピースの情報をグローバル化し、実行
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
std::vector<lengthalgorithm::frame_edge_set_type> lengthalgorithm::piecesAlignmentSequence(std::vector<lengthalgorithm::frame_edge_set_type> stacks)
{
    std::vector<frame_edge_set_type> sort_lists;
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

int lengthalgorithm::clearCorner(int frame,int com_num)
{
    int check = 0;

    procon::ExpandedPolygon Polygon = g_pieces[g_cleared_sort[frame][com_num][0].piece];
    g_frame.updatePolygon();
    Polygon.updatePolygon();
    double deg_corner = g_frame.getInnersSideAngle().back()[frame] * 180 / M_PI;
    double deg = Polygon.getSideAngle()[(g_cleared_sort[frame][com_num][0].edge + 1) % (int)Polygon.getSize()] * 180 / M_PI;
    if ((deg_corner - deg) < -1.0)
    {
        g_cleared_sort[frame].erase(g_cleared_sort[frame].begin() + com_num);
        check = 1;
        return check;
    }

    Polygon = g_pieces[g_cleared_sort[frame][com_num][(int)g_cleared_sort[frame][com_num].size() - 1].piece];
    g_frame.updatePolygon();
    Polygon.updatePolygon();
    deg_corner = g_frame.getInnersSideAngle().back()[(frame + 1) % (int)g_frame.getInnersSideAngle().back().size()];
    deg = Polygon.getSideAngle()[g_cleared_sort[frame][com_num][(int)g_cleared_sort[frame][com_num].size() - 1].edge] * 180 / M_PI;
    if ((deg_corner - deg) < -1.0)
    {
        g_cleared_sort[frame].erase(g_cleared_sort[frame].begin() + com_num);
        check = 1;
        return check;
    }

    return check;
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

int lengthalgorithm::clearEnd(int frame,int com_num)
{
    // その並び順が削除されたかを確認
    int check=0;

    for (int front_com = 0; front_com < (int)g_cleared_sort[(frame + 1) % g_frame.getInnersSideAngle().back().size()].size(); front_com++)
    {
        procon::ExpandedPolygon Polygon1 = g_pieces[g_cleared_sort[frame][com_num][g_cleared_sort[frame][com_num].size() - 1].piece];
        procon::ExpandedPolygon Polygon2 = g_pieces[g_cleared_sort[(frame + 1) % (int)g_cleared_sort.size()][front_com][0].piece];
        Polygon1.updatePolygon();
        Polygon2.updatePolygon();
        double deg1 = Polygon1.getSideAngle()[g_cleared_sort[frame][com_num][g_cleared_sort[frame][com_num].size() - 1].edge] * 180 / M_PI;
        double deg2 = Polygon2.getSideAngle()[(g_cleared_sort[(frame + 1) % (int)g_cleared_sort.size()][front_com][0].edge + 1) % Polygon2.getSize()] * 180 / M_PI;
        double deg_frame = g_frame.getInnersSideAngle().back()[(frame + 1) % (int)g_cleared_sort.size()] * 180 / M_PI;
        if (deg_frame - deg1 - deg2 >= -0.1)
        {
            return check;
        }

        int frame1_piece = g_cleared_sort[frame][com_num][g_cleared_sort[frame][com_num].size() - 1].piece;
        int frame2_piece = g_cleared_sort[(frame + 1) % (int)g_cleared_sort.size()][front_com][0].piece;
        if (frame1_piece == frame2_piece)
        {
            return check;
        }
    }
    g_cleared_sort[frame].erase(g_cleared_sort[frame].begin() + com_num);
    check = 1;
    return check;
}

void lengthalgorithm::test()
{
   // 組み合わせが全て入る配列
    std::vector<frame_edge_set_type> stacks;

    // フレームの長さぴったりのピースと辺の組み合わせを探す。
    for (int f=0; f<(int)g_frame.getInnersSideAngle().back().size(); f++)
    {
        double frame_length = g_frame.getInnersSideLength().back()[f];
        stacks.push_back(fitSide(frame_length));
    }

    stacks.erase(stacks.begin() + 1);

    // 前に出てきた組み合わせを全パターンに並び替える。
    std::vector<frame_edge_set_type> sort_list;
    sort_list = piecesAlignmentSequence(stacks);

    g_cleared_sort = sort_list;

    // 並び順が削除されたら同じ番号でfuncを繰り返す
    int count = 0;

    for (int f=0; f<(int)g_cleared_sort.size(); f++)
    {
        for (int e = 0; e < (int)g_cleared_sort[f].size(); e++)
        {
            int check = clearCorner(f,count);

            // もし削除されていなかったら次の番号で実行
            if (check == 0) count++;
        }
        count = 0;
    }

      printf("OK");

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

    // すべての組み合わせについて調べる
    for (int f=0; f<(int)g_cleared_sort.size(); f++)
    {
        for (int e = 0; e < (int)g_cleared_sort[f].size(); e++)
        {
            if (g_cleared_sort[(f + 1) % (int)g_frame.getInnersSideAngle().back().size()].size() == 0) break;

            int check = clearEnd(f,count);

            // もし削除されていなかったら次の番号で実行
            if (check == 0) count++;
        }
        count = 0;
    }

    printf("OK");
}
