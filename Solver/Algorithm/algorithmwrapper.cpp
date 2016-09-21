#include "algorithmwrapper.h"
#include "utilities.h"

#include <math.h>

#define PI 3.141592

procon::Field AlgorithmWrapper::run(procon::Field field)
{
    return field;
}

int AlgorithmWrapper::searchSameLength(procon::ExpandedPolygon polygon1, procon::ExpandedPolygon polygon2, std::vector<std::array<Fit,2>> &result)
{
    /*許容誤差指定*/
    constexpr double length_error = 0.05; // 単位CM
    constexpr double angle_error = 0.017; //単位rad 0.017rad=1°
    double comped_first = 0;
    double comping_first = 0;
    int Eva = 0;
    int maxEva = 0;
    std::array<Fit,2> fits;

    for (int i = 0; i < polygon1.getSize(); ++i) { //角度のみ探索
        comped_first = polygon1.getSideAngle()[i];
        for (int j = 0; j < polygon2.getSize(); ++j) {
            comping_first = polygon2.getSideAngle()[j];
            //if (procon::nearlyEqual(comped_first,comping_first,angle_error)) {
            if((M_PI * 2) - angle_error * 2 < (comping_first + comped_first) && (comping_first + comped_first) < (M_PI * 2) + angle_error * 2){
                Eva++;

                int start_polygon1 = i;
                int start_polygon2 = j;
                Fit::DotORLine dot_or_line = AlgorithmWrapper::findEnd(polygon2, polygon1, start_polygon2, start_polygon1,length_error, angle_error, Eva);
                fits.at(0).end_dot_or_line = dot_or_line;
                fits.at(0).end_id=start_polygon1;
                fits.at(1).end_dot_or_line = dot_or_line;
                fits.at(1).end_id=start_polygon2;

                //重複していたら追加しない
                bool isDuplicate = false;
                for(auto accept_fits : result){
                    if(fits[0].end_id == accept_fits[0].end_id &&
                       fits[1].end_id == accept_fits[1].end_id){
                        isDuplicate = true;
                        break;
                    }
                }

                if(!isDuplicate){
                    start_polygon1 = i;
                    start_polygon2 = j;
                    dot_or_line = AlgorithmWrapper::findEnd(polygon1, polygon2, start_polygon1, start_polygon2,length_error, angle_error, Eva);
                    fits.at(0).start_dot_or_line = dot_or_line;
                    fits.at(0).start_id=start_polygon1;
                    fits.at(1).start_dot_or_line = dot_or_line;
                    fits.at(1).start_id=start_polygon2;
                    if (Eva >= 1){
                        result.push_back(fits);
                        if (Eva > maxEva){
                            maxEva = Eva;
                        }
                    }
                }
                Eva = 0;
            }
        }
    }
    return maxEva;
}

Fit::DotORLine AlgorithmWrapper::findEnd(procon::ExpandedPolygon polygon1, procon::ExpandedPolygon polygon2,int &comp1,int &comp2, double length_error, double angle_error, int &Eva)
{
    double comped;
    double comping;
    for (int r=0; r<polygon1.getSize(); r++){
        //decrement comp1
        comp1--;
        if (comp1 < 0) {
            comp1=polygon1.getSize()-1;
        }
        //compare LENGTH comp1 <=> comp2
        comped=polygon1.getSideLength()[comp1];
        comping=polygon2.getSideLength()[comp2];
        if (comped - (length_error*2) < comping && comping < comped + (length_error*2)){
            //increment Eva
            Eva++;
        } else {
            //increment comp1
            if (comp1 == polygon1.getSize()-1){
                comp1=-1;
            }
            comp1++;
            //return
            return Fit::Dot;

        }

        //increment comp2
        comp2++;
        if (comp2 > polygon2.getSize()-1){
            comp2=0;
        }
        //compare ANGLE comp1 <=> comp2
        comped=polygon1.getSideAngle()[comp1];
        comping=polygon2.getSideAngle()[comp2];
        if ((M_PI * 2) - angle_error * 2 < (comping + comped) && (comping + comped) < (M_PI * 2) + angle_error * 2){
            //increment Eva
            Eva++;
        } else {
            //decrement comp2
            if (comp2 == 0){
                comp2=polygon2.getSize();
            }
            comp2--;
            //return
            return Fit::Line;
        }
    }
}

// 頂点配列を指定してオブジェクトを作成
Piece::Piece(int _n, const Point *_p)
{
    n = _n;
    p.resize(_n);
    for (int i = 0; i < _n; i++) p[i] = _p[i];
}

// 入力データ：破片の一覧
std::vector<Piece> g_pieces;

// func()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
std::vector<PieceEdge> g_comb;

// 組み合わせを保存する配列
std::vector<std::vector<PieceEdge>> g_stack;

// rl : フレーム辺の残りの長さ
// pi : 破片番号。g_pieces[]のインデックス。
void fitSide(double rl, int pi)
{
    // フレーム辺の長さに破片がぴったり合ったら表示して、再帰から抜ける。
    if (fabs(rl) < 0.001)
    {
        // 破片とその辺の組み合わせを保存
        g_stack.push_back(g_comb);
        return;
    }

    // すべての破片の組み合わせを試してたら再帰から抜ける。
    if ((int)g_pieces.size() <= pi)
    {
        return;
    }

    // 破片の各辺を入れて再帰する
    for (int e = 0; e < g_pieces[pi].n; e++)
    {
        // フレーム辺の残りの長さより破片の辺が短ければ入れてみる。
        double l = g_pieces[pi].getEdgeLength(e);
        if (l <= rl)
        {
            // この破片と辺をスタックに積む
            g_comb.push_back(PieceEdge(pi, e));
            // 次の破片へ再帰
            fitSide(rl - l, pi + 1);
            // スタックから取り除く。
            g_comb.pop_back();
        }
    }

    // この破片は入れずに、次の破片へ再帰する。
    fitSide(rl, pi + 1);
}

void test()
{
    // テストデータをセットアップ
    Point p0[] = { { 0, 0 }, { 0, 4 }, { 2, 0 } };
    Point p1[] = { { 2, 0 }, { 1, 2 }, { 3, 2 }, { 3, 0 } };
    Point p2[] = { { 1, 2 }, { 0, 4 }, { 4, 4 }, { 5, 0 }, { 3, 0 }, { 3, 2 }, };
    Point p3[] = { { 5, 0 }, { 4, 4 }, { 6, 4 }, { 6, 0 } };
    g_pieces.push_back(Piece(3, p0));
    g_pieces.push_back(Piece(4, p1));
    g_pieces.push_back(Piece(6, p2));
    g_pieces.push_back(Piece(4, p3));

    // テストするフレーム辺の長さを指定して処理を実行
    fitSide(4.0, 0);

    // すべての組み合わせを表示
    for (int s = 0; s < (int)g_stack.size(); s++)
    {
        for (int p = 0; p < (int)g_stack[s].size(); p++)
        {
            printf("piece %d-%d\n", g_stack[s][p].pi, g_stack[s][p].e);
        }
        printf("\n");
    }

    std::vector<std::vector<PieceEdge>> box;
    box = g_stack;

    printf("OK");
}

AlgorithmWrapper::AlgorithmWrapper()
{
    test();
}
