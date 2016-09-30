#include <array>
#include <cmath>

#include "polygonconnector.h"

#include "fit.h"
#include "utilities.h"

PolygonConnector::PolygonConnector()
{

}

//ピースならouterを、フレームなら指定のinner(反転する)を返す（最後の点は消す）
Ring PolygonConnector::popRingByPolygon(procon::ExpandedPolygon& polygon, int inner_position)
{
    if(inner_position == -1){
        Ring ring = polygon.getPolygon().outer();
        ring.pop_back();
        return ring;
    }else{
        Ring inner = polygon.getPolygon().inners().at(inner_position);
        Ring outer;
        int inner_size = inner.size();
        for(int i=0; i < inner_size-1; ++i){ //not copy last
            outer.push_back(inner[i]);
        }
        return outer;
    }
}

//ピースならouterを、フレームなら指定のinner(反転させる)とringを置き換える（最後の点を追加する）
void PolygonConnector::pushRingToPolygon(Ring& ring, procon::ExpandedPolygon& polygon, int inner_position)
{
    ring.push_back(*ring.begin());

    polygon_t new_raw_polygon = polygon.getPolygon();

    if(inner_position == -1){
        new_raw_polygon.outer().clear();
        for(auto point : ring){
            new_raw_polygon.outer().push_back(point);
        }
    }else{
        Ring inner;
        int ring_size = ring.size();
        for(int i=0; i < ring_size; ++i){
            inner.push_back(ring[i]);
        }
        new_raw_polygon.inners().at(inner_position).clear();
        for(auto point : inner){
            new_raw_polygon.inners().at(inner_position).push_back(point);
        }
    }

    polygon.setPolygon(new_raw_polygon);
}

//ポリゴンを合体する関数本体 !!!!!!polygon2 mast piece
bool PolygonConnector::joinPolygon(procon::ExpandedPolygon joined_polygon, procon::ExpandedPolygon piece, procon::ExpandedPolygon& new_polygon, std::array<Fit,2> join_data)
{
    auto debugRing = [](Ring ring, int line){
        std::cout<<std::to_string(line)<<" : ";
        for (int i=0; i < static_cast<int>(ring.size()); i++) {
            double x = ring[i].x();
            double y = ring[i].y();
            std::cout<<"<"<<x<<","<<y<<">";
        }
        std::cout<<std::endl;
    };

    //結合情報
    Fit fit1 = join_data[0];
    Fit fit2 = join_data[1];

    //それぞれOuterとして持つ
    Ring ring1 = popRingByPolygon(joined_polygon, joined_polygon.getInnerSize() == 0 ? -1 : fit1.flame_inner_pos);
    Ring ring2 = popRingByPolygon(piece, -1);
    int size1 = ring1.size();
    int size2 = ring2.size();

    //debugRing(ring1,__LINE__);
    //debugRing(ring2,__LINE__);

    //結合後に座標が一致する始点及び終点を取得
    const int complete_matching_start_pos_1 = fit1.start_dot_or_line == Fit::Dot ? fit1.start_id : fit1.start_id                  ;
    const int complete_matching_end_pos_1   = fit1.end_dot_or_line   == Fit::Dot ? fit1.end_id   : increment(fit1.end_id, size1)  ;
    const int complete_matching_start_pos_2 = fit2.start_dot_or_line == Fit::Dot ? fit2.start_id : increment(fit2.start_id, size2);
    const int complete_matching_end_pos_2   = fit2.end_dot_or_line   == Fit::Dot ? fit2.end_id   : fit2.end_id                    ;

    // 回転　Ring2を回転させる。このとき誤差が生じる。
    const double x1 = ring1[complete_matching_start_pos_1].x() - ring1[increment(complete_matching_start_pos_1, size1)].x();
    const double y1 = ring1[complete_matching_start_pos_1].y() - ring1[increment(complete_matching_start_pos_1, size1)].y();
    const double x2 = ring2[complete_matching_start_pos_2].x() - ring2[decrement(complete_matching_start_pos_2, size2)].x();
    const double y2 = ring2[complete_matching_start_pos_2].y() - ring2[decrement(complete_matching_start_pos_2, size2)].y();
    const double degree2 = atan2(y2, x2);
    const double degree1 = atan2(y1, x1);
    const double rotate_radian = (degree1 - degree2);
    piece.rotatePolygon(-rotate_radian*(360/(M_PI*2))); //rotate piece
    ring2 = popRingByPolygon(piece,-1); //update ring2

    //debugRing(ring1,__LINE__);
    //debugRing(ring2,__LINE__);

    // 移動　結合後に一致する点とその次の点を用いて、ポリゴンのx,y移動を調べ、Polygon2を平行移動
    const int Join_point1 = complete_matching_start_pos_1;
    const int Join_point2 = complete_matching_start_pos_2;
    const double move_x = ring1[Join_point1].x() - ring2[Join_point2].x();
    const double move_y = ring1[Join_point1].y() - ring2[Join_point2].y();
    piece.translatePolygon(move_x, move_y); //translate piece
    ring2 = popRingByPolygon(piece,-1); //update ring2

    // 重複チェック！
    if(hasConflict(ring1, ring2, fit1, fit2)){
        return false;
    }

    //debugRing(ring1,__LINE__);
    //debugRing(ring2,__LINE__);

    // 結合　新しいRingに結合後の外周の角を入れる。
    // もし、結合端の辺の長さが等しくならない時はRing1,Ring2ともに端の角を入力。
    // ここで回転の誤差により角が一致しない場合がある。
    Ring new_ring;
    int count = complete_matching_end_pos_1 + 1;
    int Type = 1;

    double x,y;
    do{
        if (Type == 1) {
            x = ring1[count%size1].x();
            y = ring1[count%size1].y();
            if (count % size1 == complete_matching_start_pos_1){
                Type = 2;
                if (fit1.start_dot_or_line == Fit::Dot) { //dot_or_lineはどちらのポリゴンでも同じですね…仕様が変だ
                    count = complete_matching_start_pos_2 + 1;
                } else {
                    count = complete_matching_start_pos_2;
                }
            }else{
                count++;
            }
        }
        if (Type == 2) {
            x = ring2[count%size2].x();
            y = ring2[count%size2].y();
            if (count % size2 == (fit2.end_dot_or_line == Fit::Dot ? (((complete_matching_end_pos_2 - 1) % size2 + size2) % size2) : complete_matching_end_pos_2)) {
                Type = -1;
            }else{
                count++;
            }
        }
        new_ring.push_back(point_t(x,y));
    } while (Type != -1);

    //debugRing(new_ring,__LINE__);

    //　ポリゴンにRingを出力しておしまい
    if(joined_polygon.getInnerSize() != 0){ //flame-piece
        pushRingToPolygon(new_ring, joined_polygon, fit1.flame_inner_pos);
        joined_polygon.setMultiIds(std::vector<int>{joined_polygon.getId(), piece.getId()});
        new_polygon = std::move(joined_polygon);
        new_polygon.jointed_pieces.push_back(piece);
    }else{ //piece-piece
        new_polygon.setMultiIds(std::vector<int>{joined_polygon.getId(), piece.getId()});
        pushRingToPolygon(new_ring, new_polygon);
        new_polygon.jointed_pieces.push_back(joined_polygon);
        new_polygon.jointed_pieces.push_back(piece);
    }

    return true;
}

//重複を見つける。
bool PolygonConnector::hasConflict(Ring ring1, Ring ring2, Fit fit1, Fit fit2)
{
    int size1 = ring1.size();
    int size2 = ring2.size();
    //結合後に座標が一致する始点及び終点を取得
    const int cmstart1 = fit1.start_dot_or_line == Fit::Dot ? fit1.start_id : fit1.start_id                  ;
    const int cmend1   = fit1.end_dot_or_line   == Fit::Dot ? fit1.end_id   : increment(fit1.end_id, size1)  ;
    const int cmstart2 = fit2.start_dot_or_line == Fit::Dot ? fit2.start_id : increment(fit2.start_id, size2);
    const int cmend2   = fit2.end_dot_or_line   == Fit::Dot ? fit2.end_id   : fit2.end_id                    ;

    bool ring1_yello_start_zone = false;
    bool ring1_orange_start_zone = false;
    bool ring1_red_zone = false;
    bool ring1_orange_end_zone = true;
    bool ring1_yellow_end_zone = false;
    bool ring1_white_zone = false;

    int ring1_pos = cmend1; //orange end zone
    if((ring1_pos+2)%size1 == cmstart1 && fit1.start_dot_or_line == Fit::Dot){
        ring1_orange_end_zone = false;
        ring1_yello_start_zone = true;
    }else if((ring1_pos+1)%size1 == cmstart1 && fit1.start_dot_or_line == Fit::Line){
        ring1_orange_end_zone = false;
        ring1_orange_start_zone = true;
    }

    for(int i=0;i<size1;++i){

        //make ring
        point_t line1_start = ring1[ring1_pos];
        point_t line1_end = ring1[(ring1_pos+1)%size1];


        bool ring2_yello_start_zone = false;
        bool ring2_orange_start_zone = false;
        bool ring2_red_zone = false;
        bool ring2_orange_end_zone = true;
        bool ring2_yellow_end_zone = false;
        bool ring2_white_zone = false;

        int ring2_pos = cmend2; //orange end zone
        if(((ring2_pos-2)%size2+size2)%size2 == cmstart2 && fit2.start_dot_or_line == Fit::Dot){
            ring2_orange_end_zone = false;
            ring2_yello_start_zone = true;
        }else if(((ring2_pos-1)%size2+size2)%size2 == cmstart2 && fit2.start_dot_or_line == Fit::Line){
            ring2_orange_end_zone = false;
            ring2_orange_start_zone = true;
        }

        for(int j=0;j<size2;++j){

            //skip check
            if(     ring1_white_zone ||
                    (ring1_yellow_end_zone && !ring2_orange_end_zone && (ring2_yellow_end_zone || ring2_white_zone || ring2_red_zone || ring2_orange_start_zone || ring2_yello_start_zone)) ||
                    (ring1_orange_end_zone && !ring2_red_zone && !ring2_yellow_end_zone && !ring2_orange_end_zone && (ring2_white_zone || (ring2_orange_start_zone && (cmstart1!=cmend1 || fit2.start_dot_or_line == Fit::Line)) || ring2_yello_start_zone)) ||
                    (ring1_red_zone && !ring2_red_zone && !ring2_orange_end_zone && !ring2_orange_start_zone && (ring2_white_zone || ring2_yellow_end_zone || ring2_yellow_end_zone)) ||
                    (ring1_orange_start_zone && !ring2_red_zone && !ring2_orange_start_zone && !ring2_yello_start_zone && (ring2_white_zone || ring2_yellow_end_zone || (ring2_orange_end_zone && (cmstart1 != cmend1 || fit2.start_dot_or_line == Fit::Line)))) ||
                    (ring1_yello_start_zone && !ring2_orange_start_zone && (ring2_yello_start_zone || ring2_white_zone || ring2_yellow_end_zone || ring2_orange_end_zone || ring2_red_zone))
              ){
                //make ring
                point_t line2_start = ring2[ring2_pos];
                point_t line2_end = ring2[((ring2_pos - 1)%size2+size2)%size2];

                //check conflict
                if(static_cast<bool>(Utilities::cross_check(line1_start, line1_end, line2_start, line2_end))){
                    return true;
                }
            }

            //dec
            ring2_pos--;
            if(ring2_pos == -1) ring2_pos = size2 - 1;

            //toggle
            if(ring2_orange_end_zone){
                ring2_orange_end_zone = false;
                if(fit2.end_dot_or_line == Fit::Dot){
                    ring2_yellow_end_zone = true;
                }else{
                    ring2_white_zone = true;
                }
            }else if(ring2_yellow_end_zone && (fit2.start_dot_or_line == Fit::Dot ? ring2_yello_start_zone == false : ring2_orange_start_zone == false)){
                ring2_yellow_end_zone = false;
                ring2_white_zone = true;
            }
            if(((ring2_pos-2)%size2+size2)%size2 == cmstart2 && fit2.start_dot_or_line == Fit::Dot){
                ring2_white_zone = false;
                ring2_yello_start_zone = true;
            }else if(((ring2_pos-1)%size2+size2)%size2 == cmstart2 && fit2.start_dot_or_line == Fit::Line){
                ring2_white_zone = false;
                ring2_orange_start_zone = true;
            }else if(ring2_yello_start_zone){
                ring2_yellow_end_zone = false;
                ring2_yello_start_zone = false;
                ring2_orange_start_zone = true;
            }else if(ring2_orange_start_zone){
                ring2_yellow_end_zone = false;
                ring2_orange_start_zone = false;
                ring2_red_zone = true;
            }
        }
        //inc
        ring1_pos++;
        if(ring1_pos == size1) ring1_pos = 0;

        //toggle
        if(ring1_orange_end_zone){
            ring1_orange_end_zone = false;
            if(fit1.end_dot_or_line == Fit::Dot){
                ring1_yellow_end_zone = true;
            }else{
                ring1_white_zone = true;
            }
        }else if(ring1_yellow_end_zone && (fit1.start_dot_or_line == Fit::Dot ? ring1_yello_start_zone == false : ring1_orange_start_zone == false)){
            ring1_yellow_end_zone = false;
            ring1_white_zone = true;
        }
        if((ring1_pos+2)%size1 == cmstart1 && fit1.start_dot_or_line == Fit::Dot){
            ring1_white_zone = false;
            ring1_yello_start_zone = true;
        }else if((ring1_pos+1)%size1 == cmstart1 && fit1.start_dot_or_line == Fit::Line){
            ring1_white_zone = false;
            ring1_orange_start_zone = true;
        }else if(ring1_yello_start_zone){
            ring1_yellow_end_zone = false;
            ring1_yello_start_zone = false;
            ring1_orange_start_zone = true;
        }else if(ring1_orange_start_zone){
            ring1_yellow_end_zone = false;
            ring1_orange_start_zone = false;
            ring1_red_zone = true;
        }
    }
    return false;
}

Fit PolygonConnector::searchFieldConnection(procon::Field field)
{
    bool succeeded = false;

    struct FlameSlope{
        std::array<double,2> start_point_number;
        std::array<double,2> end_point_number;
        double field_slope;
        double field_y_intercept;
    };

    struct point{
        //１つめの点のinners.at().at(ココ)
        unsigned int flame_point_number_1;
        unsigned int flame_point_number_2;
        unsigned int flame_inner_position;

        point_t flame_point;
        point_t piece_point;
    };


    struct PointLine{

        unsigned int inner_position;

        unsigned int line_start_point_number;
        unsigned int line_end_point_number;

        unsigned int point_number;

        //point_t point;
    };


    struct Line{
        double a;
        double b;
        double c;
    };


    auto calcPointToPointDistance = [](point_t start,point_t end){
        return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
    };





    auto CalcLineToDistance = [](point_t a,point_t b,point_t c)->double{

        struct Vec2d{
            double x;
            double y;
        };

        //naiseki
        auto Dot = [](Vec2d a,Vec2d b)->double{
            return a.x * b.x + a.y + b.y;
        };

        //gaiseki
        auto Cross = [](Vec2d a,Vec2d b)->double{
            return a.x * b.y - a.y * b.x;
        };

        auto calcVec = [](point_t a,point_t b)->Vec2d{
            Vec2d vec;
            vec.x = b.x() - a.x();
            vec.y = b.y() - a.y();

            return vec;
        };

        auto calcLineABC = [](point_t start,point_t end)->Line{

            Line line;

            line.a = start.y() - end.y();
            line.b = end.x() - start.x();
            line.c = (line.b * start.y()) + (line.a * start.x());

            /*
            const double a = y1 - y2;
            const double b = x2 - x1;
            const double c = (-b * y1) + (-a * x1);
            */

            return line;
        };

        auto calcPointToLineDistance = [](point_t point,Line line){

            line.c = 0;

            const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
            const double bunbo = line.a * line.a + line.b * line.b;

            const double distance = bunshi * bunshi / bunbo;

            return distance;
        };

        if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

            std::cout << "out of hani" << std::endl;

        }

        if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

            std::cout << "out of hani" << std::endl;

        }

        Line line = calcLineABC(a,b);

        const double distance = calcPointToLineDistance(c,line);

        return distance;

    };


    auto goOkerutokoromade = [](procon::Field field,unsigned int searching_point_main_number,unsigned int ){



    }

    constexpr double gosaaaaaaaaa = 0.1;

    bool can_connect = false;

    bool FirstPointHasNearPoint = false;




    for(unsigned int i = 0; i < field.getFlame().getPolygon().inners().size(); i++){

        Fit fit_buf;

        unsigned int point_number_at_0 = 0;

        //check line 1 and point 1 have near point
        for(unsigned int k = 1; k < field.getFlame().getPolygon().inners().at(i).size() - 1; k++){

            const double distance = calcPointToPointDistance(field.getFlame().getPolygon().inners().at(i).at(0)
                                                             ,field.getFlame().getPolygon().inners().at(i).at(k));



            //distance equals real-distance squared
            if(distance < gosaaaaaaaaa * gosaaaaaaaaa){

                FirstPointHasNearPoint = true;

                point_number_at_0 = k;

                break;

            }
        }


        if(FirstPointHasNearPoint){


            //0番目の点が接していた点の番号
            unsigned int searching_main_point_number = 1;
            unsigned int searching_sub_point_numebr = point_number_at_0;

            double distance = 0;


            searching_main_point_number = field.getFlame().getPolygon().inners().at(i).size() - 1;

            while(1){

                distance = calcPointToPointDistance(field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number)
                                                    ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_numebr));

                if(!(distance < gosaaaaaaaaa * gosaaaaaaaaa)){

                    //1,行き過ぎてるので戻す
                    ++searching_main_point_number;
                    ++searching_sub_point_numebr;

                    break;

                }

                searching_main_point_number = searching_main_point_number - 1;
                searching_sub_point_numebr = searching_sub_point_numebr - 1;

            }

            const double distance1 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number - 1)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_numebr - 1));

            const double distance2 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_numebr)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_numebr - 1)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number - 1));


            if(distance1 < gosaaaaaaaaa * gosaaaaaaaaa){

                fit_buf.flame_inner_pos = i;
                fit_buf.start_dot_or_line = Fit::Line;
                fit_buf.start_id = searching_main_point_number - 1;

                can_connect = true;

                return fit_buf;

            }else if(distance2 < gosaaaaaaaaa * gosaaaaaaaaa){

                fit_buf.flame_inner_pos = i;
                fit_buf.start_dot_or_line = Fit::Line;
                fit_buf.start_id = searching_main_point_number - 1;

                can_connect = true;

                return fit_buf;

            }else{

                fit_buf.flame_inner_pos = i;
                fit_buf.start_dot_or_line = Fit::Dot;
                fit_buf.start_id = searching_main_point_number;

                can_connect = true;

                return fit_buf;

            }


        }else{

            unsigned int first_line_number;

            bool FirstPointLineHasNearPoint = false;

            for(int k = 1; k < field.getFlame().getPolygon().inners().at(i).size() - 1; k++){

                const double distanceLine = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(0),field.getFlame().getPolygon().inners().at(i).at(1),field.getFlame().getPolygon().inners().at(i).at(k));

                if(distanceLine < gosaaaaaaaaa * gosaaaaaaaaa){

                    FirstPointLineHasNearPoint = true;



                    first_line_number = k;

                    fit_buf.start_id = 0;
                    fit_buf.start_dot_or_line = Fit::Line;
                    fit_buf.flame_inner_pos = i;

                    break;

                }

            }

            if(FirstPointLineHasNearPoint){

                unsigned int searching_main_point_number = 1;
                unsigned int searching_sub_point_number ;

                double distance = 0;


                while(1){

                    distance = calcPointToPointDistance(field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_number));

                    if(!(distance < gosaaaaaaaaa * gosaaaaaaaaa)){

                        //1,行き過ぎてるので戻す
                        --searching_main_point_number;
                        --searching_sub_point_number;

                         break;

                    }

                    searching_main_point_number = searching_main_point_number + 1;
                    searching_sub_point_number = searching_sub_point_number + 1;

                }

                const double distance1 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number + 1)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_number + 1));

                const double distance2 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_number)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_sub_point_number + 1)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_main_point_number + 1));


                if(distance1 < gosaaaaaaaaa * gosaaaaaaaaa){

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Line;
                    fit_buf.end_id = searching_main_point_number;

                    return fit_buf;

                }else if(distance2 < gosaaaaaaaaa * gosaaaaaaaaa){

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Line;
                    fit_buf.end_id = searching_main_point_number;

                    return fit_buf;

                }else{

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Dot;
                    fit_buf.end_id = searching_main_point_number;

                    return fit_buf;
                }

            }else{

                unsigned int searching_point_main_number;
                unsigned int searching_point_sub_number;

                [&](){
                for(unsigned int k = 1; k < field.getFlame().getPolygon().inners().at(i).size(); k++){

                    for(unsigned int l = k + 1; l < field.getFlame().getPolygon().inners().at(i).size(); l++){

                        const double distance1 = calcPointToPointDistance(field.getFlame().getPolygon().inners().at(i).at(k)
                                                                          ,field.getFlame().getPolygon().inners().at(i).at(l));

                        if(distance1 < gosaaaaaaaaa * gosaaaaaaaaa){

                            fit_buf.start_dot_or_line = Fit::Dot;
                            fit_buf.start_id = k;
                            fit_buf.flame_inner_pos = i;

                            searching_point_main_number = k;
                            searching_point_sub_number = l;


                            return;
                        }


                        const double distance2 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(k)
                                                                   ,field.getFlame().getPolygon().inners().at(i).at(k+1)
                                                                   ,field.getFlame().getPolygon().inners().at(i).at(l));

                        if(distance2 < gosaaaaaaaaa * gosaaaaaaaaa){

                            fit_buf.start_dot_or_line = Fit::Dot;
                            fit_buf.start_id = k;
                            fit_buf.flame_inner_pos = i;

                            searching_point_main_number = k;
                            searching_point_sub_number = l;


                            return;

                        }



                    }


                }
                }();

                double distance = 0;

                while(1){

                    distance = calcPointToPointDistance(field.getFlame().getPolygon().inners().at(i).at(searching_point_main_number)
                                                        ,field.getFlame().getPolygon().inners().at(i).at(searching_point_sub_number));

                    if(!(distance < gosaaaaaaaaa * gosaaaaaaaaa)){

                        //1,行き過ぎてるので戻す
                        --searching_point_main_number;
                        --searching_point_sub_number;

                         break;

                    }

                    searching_point_main_number = searching_point_main_number + 1;
                    searching_point_sub_number = searching_point_sub_number + 1;

                }

                const double distance1 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_point_main_number)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_point_main_number + 1)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_point_sub_number + 1));

                const double distance2 = CalcLineToDistance(field.getFlame().getPolygon().inners().at(i).at(searching_point_sub_number)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_point_sub_number + 1)
                                                            ,field.getFlame().getPolygon().inners().at(i).at(searching_point_main_number + 1));


                if(distance1 < gosaaaaaaaaa * gosaaaaaaaaa){

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Line;
                    fit_buf.end_id = searching_point_main_number;

                    can_connect = true;

                    return fit_buf;


                }else if(distance2 < gosaaaaaaaaa * gosaaaaaaaaa){

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Line;
                    fit_buf.end_id = searching_point_main_number;

                    can_connect = true;

                    return fit_buf;


                }else{

                    fit_buf.flame_inner_pos = i;
                    fit_buf.end_dot_or_line = Fit::Dot;
                    fit_buf.end_id = searching_point_main_number;

                    can_connect = true;

                    return fit_buf;
                }

            }

        }

    }

    if(can_connect){
        std::cout << "yabai" << std::endl;
    }else{
        std::cout << "koredosuru?" << std::endl;
    }



    /*
    std::vector<point> pointlist;
    std::vector<std::vector<bool>> hasNearPoint(10);
    //std::vector<bool> hasNearPoint_buf;

    point point_buffer;

    /*
    for(unsigned int i = 0; i < field.getFlame().getPolygon().inners().size(); i++){
        for(unsigned int k = 0; k < field.getFlame().getPolygon().inners().at(i).size(); k++){

            const double flame_x_1 = field.getFlame().getPolygon().inners().at(i).at(k).x();
            const double flame_y_1 = field.getFlame().getPolygon().inners().at(i).at(k).y();

            for(unsigned int l = k + 1; l < field.getFlame().getPolygon().inners().at(i).size() - 1; l++){

                const double flame_x_2 = field.getFlame().getPolygon().inners().at(i).at(l).x();
                const double flame_y_2 = field.getFlame().getPolygon().inners().at(i).at(l).y();

                const double distance = (flame_x_1 - flame_x_2) * (flame_x_1 - flame_x_2) + (flame_y_1 - flame_y_2) * (flame_y_1 - flame_y_2);

                //許容誤差aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
                const double gosaaaaaaaaaaaaaaa = 0.1;

                if(distance < gosaaaaaaaaaaaaaaa * gosaaaaaaaaaaaaaaa){


                    point_buffer.flame_inner_position = i;
                    point_buffer.flame_point_number_1 = k;
                    point_buffer.flame_point_number_2 = l;
                    point_buffer.flame_point = point_t(flame_x_1,flame_y_1);
                    point_buffer.piece_point = point_t(flame_x_2,flame_y_2);

                    //put
                    pointlist.push_back(point_buffer);

                    hasNearPoint.at(i).push_back(true);

                }else{

                    hasNearPoint.at(i).push_back(false);

                }
            }
        }
    }
    /*
    
    //ay+bx+c=0のa,b,cを記録するLine構造体
    std::vector<Line> lines;

    Line line_buf;
        
    //linesにデータを入れる
    for(unsigned int i = 0; i < field.getFlame().getPolygon().inners().size(); i++){
        for(unsigned int k = 0; k < field.getFlame().getPolygon().inners().at(i).size() - 1; k++){


            const double x1 = field.getFlame().getPolygon().inners().at(i).at(k).x();
            const double y1 = field.getFlame().getPolygon().inners().at(i).at(k).y();
            const double x2 = field.getFlame().getPolygon().inners().at(i).at(k + 1).y();
            const double y2 = field.getFlame().getPolygon().inners().at(i).at(k + 1).y();

            const double a = y1 - y2;
            const double b = x2 - x1;
            const double c = (-b * y1) + (-a * x1);

            //put start and end information
            line_buf.line_inner_position = i;
            line_buf.line_start_point_number = k;
            line_buf.line_end_point_number = k + 1;

            //put calc result
            line_buf.a = a;
            line_buf.b = b;
            line_buf.c = c;


            lines.push_back(line_buf);

        }
    }
    
    //PointLine構造体:近いポイントとラインの点番号を記録
    std::vector<PointLine> PointLineList;
    std::vector<std::vector<bool>> hasNearPointLine(10);

    point_t point_buf;
    PointLine PointLine_Buf;
    
    //この一つ前で作ったLinesをつかって、点と直線の距離公式を使い距離を求めて近いやつをピックアップ
    for(unsigned int i = 0; i < field.getFlame().getPolygon().inners().size(); i++){
        for(unsigned int k = 0; k < field.getFlame().getPolygon().inners().at(i).size(); k++){

            const double x = field.getFlame().getPolygon().inners().at(i).at(k).x();
            const double y = field.getFlame().getPolygon().inners().at(i).at(k).y();

            for(auto line : lines){

                //同じinnerをサーチしている時だけtrue
                //elseではfalseをpush_backする

                if(line.line_inner_position == i){

                    const double bunshi = line.a * x + line.b * y + line.c;
                    const double bunbo = line.a * line.a + line.b * line.b;

                    const double distance = std::abs(bunshi) / std::sqrt(bunbo);

                    const double gosaaaaaaaaaaaaaaaaaaaaaaa = 0.1;

                    if(distance < gosaaaaaaaaaaaaaaaaaaaaaaa){


                        point_buf = point_t(x,y);

                        //put line information
                        PointLine_Buf.line_start_point_number = line.line_start_point_number;
                        PointLine_Buf.line_end_point_number = line.line_end_point_number;
                        PointLine_Buf.inner_position = line.line_inner_position;

                        //put point information
                        PointLine_Buf.point_number = k;

                        //put point
                        PointLine_Buf.point = point_buf;

                        PointLineList.push_back(PointLine_Buf);

                        //true:has near point
                        hasNearPointLine.at(i).push_back(true);

                    }else{

                        //false:has no near point
                        hasNearPointLine.at(i).push_back(false);

                    }

                }else{

                    //innerが違うのでfalseを入れて次へ
                    hasNearPointLine.at(i).push_back(false);

                }
            }
        }
    }


    ///Check has Near Point or Line

    //has NearPoint or Line

    for(unsigned int i = 0; i < hasNearPoint.size(); i++){
        for(unsigned int  l = 0; l < hasNearPoint.at(i).size(); l++){

            if(hasNearPoint.at(i).at(l) == true){

                succeeded = true;

                break;

            }
        }
    }

    for(unsigned int i = 0; i < hasNearPointLine.size(); i++){
        for(unsigned int  l = 0; l < hasNearPointLine.at(i).size(); l++){

            if(hasNearPointLine.at(i).at(l) == true){

                succeeded = true;

                break;

            }
        }
    }

    //do not have near point or line
    if(succeeded == false){

        succeeded = false;

        Fit empty_fit;

        return empty_fit;

    }




    
    //持っているデータ
    //if yout want to use this data,please comment out ... in this function

    //PointLine:near line and point
    //pointlist:near point and point
    

    Fit fit_buf;

    unsigned int searching_field_inner_number = 0;
    unsigned int searching_field_point_number = 0;

    //繋がる部分の始点と終点をサーチする部分
    //goto多用だけどこっちのほうが見やすいと思う

    while(searching_field_inner_number < field.getFlame().getPolygon().inners().size()){
        while(searching_field_point_number < field.getFlame().getPolygon().inners().at(searching_field_inner_number).size()){

            //nextLoop:;

            /*
            if(searching_field_point_number == 0){
                if(hasNearPoint.at(searching_field_inner_number).at(searching_field_point_number)){
                    //start is point

                    //put
                    fit_buf.start_dot_or_line = Fit::Dot;
                    fit_buf.start_id = searching_field_point_number;
                    fit_buf.flame_inner_pos = searching_field_inner_number;

                    //goto point search
                    goto PointSearch;
                }
            }
            *//*

            if(hasNearPoint.at(searching_field_inner_number).at(searching_field_point_number)){
                //start is point

                //put
                fit_buf.start_dot_or_line = Fit::Dot;
                fit_buf.start_id = searching_field_point_number;
                fit_buf.flame_inner_pos = searching_field_inner_number;

                //goto point continue search
                goto PointSearch;

            }else{
                if(hasNearPointLine.at(searching_field_inner_number).at(searching_field_point_number)){
                    //start is line

                    fit_buf.start_dot_or_line = Fit::Line;
                    fit_buf.start_id = searching_field_point_number;
                    fit_buf.flame_inner_pos = searching_field_inner_number;

                    goto PointSearch;

                }else{

                    //search next ++
                    searching_field_point_number++;

                    //goto nextLoop;

                }
            }


            while(1){

                PointSearch:;

                //接してる限りカウント
                if(!hasNearPoint.at(searching_field_inner_number).at(searching_field_point_number)){

                    //1進んでしまってるのでtrueだったところに戻す
                    searching_field_point_number--;

                    //goto 終点サーチ
                    goto endSearch;
                }
                searching_field_point_number++;
            }

            //最後に接してるのが点か線か確かめる
            endSearch:;

            if(hasNearPointLine.at(searching_field_inner_number).at(searching_field_point_number)){
                //end is line

                fit_buf.start_dot_or_line = Fit::Line;
                fit_buf.start_id = searching_field_point_number;
                fit_buf.flame_inner_pos = searching_field_inner_number;


                //goto next search ++

                searching_field_point_number++;

                //return result
                return fit_buf;

                //goto nextLoop;

            }else{
                //end is point

                fit_buf.start_dot_or_line = Fit::Dot;
                fit_buf.start_id = searching_field_point_number;
                fit_buf.flame_inner_pos = searching_field_inner_number;


                //goto next loop ++

                searching_field_point_number++;

                //return result
                return fit_buf;

                //goto nextLoop;

            }

        }
        searching_field_inner_number++;
    }


    //to delete warning
    Fit emptyFit;
    succeeded = false;

    return emptyFit;

    */

}
