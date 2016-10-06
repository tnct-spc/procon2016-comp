#include "lengthalgorithm.h"

#include "utilities.h"
#include "field.h"
#include <QEventLoop>

// PIを使うため
#define _USE_MATH_DEFINES
#include <math.h>

lengthalgorithm::lengthalgorithm()
{

}

void lengthalgorithm::bomPush(procon::Field& field, int frame_inner_pos, int frame_number, std::vector<PieceEdge> edges)
{
    int frame_pos = frame_number;
    for(auto& edge : edges){
        procon::ExpandedPolygon new_frame;
        PolygonConnector::joinEdge(new_frame, field.getFrame(), frame_inner_pos, frame_pos, field.getElementaryPieces().at(edge.piece), edge.edge);
        field.setFrame(new_frame);

        frame_pos = (new_frame.getPolygon().inners().at(frame_inner_pos).size()-1) - 1;
    }
}

void lengthalgorithm::run(procon::Field field)
{
    // ピースとフレームを配列に入れる
    for (int p=0; p<(int)field.getElementaryPieces().size(); p++)
    {
        g_pieces.push_back(field.getElementaryPieces().at(p));
    }

    /*for (int p=0; p<5; p++)
    {
        g_pieces.push_back(field.getElementaryInversePieces().at(p));
    }*/


    field.setFrame(field.getElementaryFrame());
    for (int a=0; a<field.getElementaryFrame().getSize(); a++)
    {
        // 領域ごとにできるようにする
        g_frame = field.getElementaryFrame();
        procon::Field result_field = test(field);
        std::cout<<"get result"<<std::endl;
        submitAnswer(result_field);
        QEventLoop whileloop;
        whileloop.exec();
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
    if ((int)g_pieces.size() / 2 <= watched_piece)
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

// ピースの組み合わせをすべてのパターンで並び替える。
// focus : 並び替える組み合わせの配列での番号
void lengthalgorithm::sortPieces(int focus)
{
    int stack_size = (int)g_sorted_pieces.size();

    // もし、最後まで調べ終わったら戻る
    if (stack_size <= focus)
    {
        this->g_frame_sort.push_back(g_sorted_pieces);
        return;
    }

    // 破片を並び替えながら再帰していく
    for (int i = focus; i < stack_size; i++)
    {
        std::swap(g_sorted_pieces[focus], g_sorted_pieces[i]);
        sortPieces(focus + 1);
        std::swap(g_sorted_pieces[focus], g_sorted_pieces[i]);
    }
}

/*void lengthalgorithm::addInversePieces(int frame, int com_num)
{
    for (int p=0; p<(int)g_cleared_sort[f][com_num].size(); p++)
    {

    }
}*/

// 端のピースがフレームと重ならないかを確認
// frame : フレーム番号
// com_num : 組み合わせ番号
void lengthalgorithm::clearCorner(int frame,int com_num)
{
    // 左側のピースを確認
    PieceEdge focus = g_cleared_sort[frame][com_num][0];
    procon::ExpandedPolygon Polygon = g_pieces[focus.piece];
    g_frame.updatePolygon();
    Polygon.updatePolygon();
    double deg_corner = g_frame.getInnersSideAngle().back()[frame] * 180 / M_PI;
    double deg = Polygon.getSideAngle()[(focus.edge + 1) % Polygon.getSize()] * 180 / M_PI;
    if ((deg_corner - deg) < -1.0)
    {
        return;
    }

    // 右側のピースを確認
    focus = g_cleared_sort[frame][com_num][(int)g_cleared_sort[frame][com_num].size() - 1];
    Polygon = g_pieces[focus.piece];
    Polygon.updatePolygon();
    deg_corner = g_frame.getInnersSideAngle().back()[(frame + 1) % (int)g_cleared_sort.size()] * 180 / M_PI;
    deg = Polygon.getSideAngle()[focus.edge] * 180 / M_PI;
     if ((deg_corner - deg) < -1.0)
    {
        return;
    }

    g_frame_ok.push_back(g_cleared_sort[frame][com_num]);
    return;
}

// 隣同士の破片が重なる場合リストから削除
// com_num : 組み合わせ番号
void lengthalgorithm::clearOverlap(int frame,int com_num)
{
    // ピースが隣り合っているところを調べる
    for (int p = 0; p < (int)g_cleared_sort[frame][com_num].size() - 1; p++)
    {
        // 隣り合った左右の破片の隣り合う角を調べ180度を越さないか確かめる
        PieceEdge id1 = g_cleared_sort[frame][com_num][p];
        PieceEdge id2 = g_cleared_sort[frame][com_num][p + 1];
        procon::ExpandedPolygon Polygon1 = g_pieces[id1.piece];
        procon::ExpandedPolygon Polygon2 = g_pieces[id2.piece];
        Polygon1.updatePolygon();
        Polygon2.updatePolygon();
        double deg1 = Polygon1.getSideAngle()[id1.edge] * 180 / M_PI;
        double deg2 = Polygon2.getSideAngle()[(id2.edge + 1) % Polygon2.getSize()] * 180 / M_PI;

        // 破片同士が重なっていたら削除
        if ((180 - deg1 - deg2) < -1.0)
        {
            return;
        }
    }

    g_frame_ok.push_back(g_cleared_sort[frame][com_num]);
    return;
}

void lengthalgorithm::clearEnd(int frame,int com_num)
{
    // 組み合わせの左端のピースに中も注目
    PieceEdge comp_id = g_cleared_sort[frame][com_num][g_cleared_sort[frame][com_num].size() - 1];
    int comp_frame_piece = comp_id.piece;
    procon::ExpandedPolygon comp_Polygon = g_pieces[comp_frame_piece];
    comp_Polygon.updatePolygon();
    double comp_deg = comp_Polygon.getSideAngle()[comp_id.edge] * 180 / M_PI;
    int next_frame = (frame + 1) % (int)g_cleared_sort.size();
    
    // 端に来るピースの端が入るか確認
    double deg_frame = g_frame.getInnersSideAngle().back()[next_frame] * 180 / M_PI;

    // 右隣のフレームの組み合わせと確認
    for (int back_com = 0; back_com < (int)g_cleared_sort[next_frame].size(); back_com++)
    {
        PieceEdge back_id = g_cleared_sort[next_frame][back_com][0];

        if (comp_frame_piece == back_id.piece)
        {
            g_frame_ok.push_back(g_cleared_sort[frame][com_num]);
            return;
        }

        procon::ExpandedPolygon back_Polygon = g_pieces[back_id.piece];
        back_Polygon.updatePolygon();
        double back_deg = back_Polygon.getSideAngle()[(back_id.edge + 1) % back_Polygon.getSize()] * 180 / M_PI;
        if (deg_frame - comp_deg - back_deg >= -0.1)
        {
            g_frame_ok.push_back(g_cleared_sort[frame][com_num]);
            return;
        }
    }

    return;
}

procon::Field lengthalgorithm::test(procon::Field field)
{
    // フレームの長さぴったりのピースと辺の組み合わせを探す。
    for (int f=0; f<(int)g_frame.getInnersSideAngle().back().size(); f++)
    {
        double frame_length = g_frame.getInnersSideLength().back()[f];
        g_frame_stack.clear();
        searchPairSide(frame_length,0);
        g_stacks.push_back(g_frame_stack);
    }

    g_stacks.erase(g_stacks.begin() + 1);

    // 前に出てきた組み合わせを全パターンに並び替える。
    for (int f=0; f<(int)g_stacks.size(); f++)
    {
        for (int c=0; c<(int)g_stacks[f].size(); c++)
        {
            g_sorted_pieces = g_stacks[f][c];
            sortPieces(0);
        }
        g_cleared_sort.push_back(g_frame_sort);

        // 毎フレームごとに中身をクリア
        g_frame_sort.clear();
    }

    for (int f=0; f<(int)g_cleared_sort.size(); f++)
    {
        for (int c = 0; c < (int)g_cleared_sort[f].size(); c++)
        {
            clearCorner(f,c);
        }
        g_cleared_sort[f].clear();
        g_cleared_sort[f] = g_frame_ok;
        g_frame_ok.clear();
    }

     printf("OK");

    // すべての組み合わせについて調べる
    for (int f=0; f<(int)g_cleared_sort.size(); f++)
    {
        for (int c = 0; c < (int)g_cleared_sort[f].size(); c++)
        {
            clearOverlap(f,c);
        }
        g_cleared_sort[f].clear();
        g_cleared_sort[f] = g_frame_ok;
        g_frame_ok.clear();
    }

     printf("OK");

    // すべての組み合わせについて調べる
    int frame_num = (int)g_cleared_sort.size();
    for (int f=0; f<(int)frame_num; f++)
    {
        for (int c = 0; c < (int)g_cleared_sort[f].size(); c++)
        {
            if (g_cleared_sort[(f + 1) % frame_num].size() == 0) break;

            clearEnd(f,c);
        }
        g_cleared_sort[f].clear();
        g_cleared_sort[f] = g_frame_ok;
        g_frame_ok.clear();
    }

    printf("OK");

    procon::Field new_field = field; //omosou
    int frame_inner_pos = 0;
    int frame_pos = 0;
    bomPush(new_field, frame_inner_pos, frame_pos, g_cleared_sort.at(frame_pos).at(0));
    return new_field;
}
