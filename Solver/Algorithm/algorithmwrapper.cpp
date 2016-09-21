#include "algorithmwrapper.h"
#include "utilities.h"
#include "field.h"

#include <math.h>

#define PI 3.141592

AlgorithmWrapper::AlgorithmWrapper()
{
    test();
}

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
std::vector<procon::ExpandedPolygon> g_pieces;

// func()再帰関数で、フレーム辺に入れた破片と辺の組み合わせを記録するスタック
std::vector<PieceEdge> g_comb;

// 組み合わせを保存する配列
std::vector<std::vector<PieceEdge>> g_stack;

// rl : フレーム辺の残りの長さ
// pi : 破片番号。g_pieces[]のインデックス。
void searchPairSide(double rl, int pi)
{
    // フレーム辺の長さに破片がぴったり合ったら表示して、再帰から抜ける。
    if (fabs(rl) < 0.001)
    {
        // 破片とその辺の組み合わせを保存
        g_stack.push_back(g_comb);
        return;
    }

    // すべての破片の組み合わせを試してたら再帰から抜ける。
    if (g_pieces.size() <= pi)
    {
        return;
    }

    // 破片の各辺を入れて再帰する
    procon::ExpandedPolygon piece;
    piece.updatePolygon();
    piece = g_pieces[pi];
    for (int e = 0; e < piece.getSize(); e++)
    {
        // フレーム辺の残りの長さより破片の辺が短ければ入れてみる。
        double l = piece.getSideLength()[e];
        if (l <= rl)
        {
            // この破片と辺をスタックに積む
            g_comb.push_back(PieceEdge(pi, e));
            // 次の破片へ再帰
            searchPairSide(rl - l, pi + 1);
            // スタックから取り除く。
            g_comb.pop_back();
        }
    }

    // この破片は入れずに、次の破片へ再帰する。
    searchPairSide(rl, pi + 1);
}

std::vector<std::vector<PieceEdge>> fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces)
{
    // テストするフレーム辺の長さを指定して処理を実行
    g_pieces = pieces;
    searchPairSide(frame,0);

    std::vector<std::vector<PieceEdge>> box;
    box = g_stack;

    return box;
}

void test()
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

    std::vector<std::vector<PieceEdge>> stacks;
    stacks = fitSide(4.0,pieces);

    printf("OK");
}
