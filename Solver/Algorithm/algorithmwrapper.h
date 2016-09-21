#ifndef ALGORITHMWRAPPER_H
#define ALGORITHMWRAPPER_H

#include "field.h"
#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "Utils/fit.h"
#include "polygonviewer.h"

class AlgorithmWrapper
{
public:

    AlgorithmWrapper();

    virtual procon::Field run(procon::Field field);

    Fit fit1,fit2;

    int searchSameLength(procon::ExpandedPolygon polygon1 ,procon::ExpandedPolygon polygon2, std::vector<std::array<Fit,2>> &result);
    Fit::DotORLine findEnd(procon::ExpandedPolygon polygon1, procon::ExpandedPolygon polygon2,int &comp1,int &comp2, double length_error, double angle_error, int &Eva);

    typedef struct PieceAssesment{
        //評価値
        int EvaluationValue;
        //評価したpolygon
        procon::ExpandedPolygon Polygon;

        bool operator<(const PieceAssesment& Pieceassesment) const {
            return EvaluationValue < Pieceassesment.EvaluationValue;
        }

    } PieceAssesment;

    typedef struct {

        int piece_id;
        int side_id;
        double length_capacity;

    } SidePair;

};

// 当てはまるピースのIDをいれるクラス
class PieceEdge
{
public:
    // 空のコンストラクタ
    PieceEdge() {}
    // 破片と辺番号を指定してオブジェクトを作成
    PieceEdge(int _pi, int _e) :
        pi(_pi),
        e(_e)
    {}

public:
    int pi;
    int e;
};

void searchPairSide(double rl, int pi);

std::vector<std::vector<PieceEdge>> fitSide(double frame, std::vector<procon::ExpandedPolygon> pieces);

void test();

#endif // ALGORITHMWRAPPER_H
