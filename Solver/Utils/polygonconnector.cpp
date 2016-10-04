#include <array>
#include <cmath>
#include <tuple>

#include "polygonconnector.h"

#include "fit.h"
#include "utilities.h"
#include "polygonviewer.h"

//#define DEBUG_RING

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
polygon_t PolygonConnector::pushRingToPolygonT(Ring& ring, procon::ExpandedPolygon const& polygon, int inner_position)
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

    return new_raw_polygon;
}

//ポリゴンを合体する関数本体 !!!!!!polygon2 mast piece
bool PolygonConnector::joinPolygon(procon::ExpandedPolygon jointed_polygon, procon::ExpandedPolygon piece, procon::ExpandedPolygon& new_polygon, std::array<Fit,2> join_data)
{
#ifdef DEBUG_RING
    auto debugRing = [](Ring ring, int line){
        std::cout<<std::to_string(line)<<" : ";
        for (int i=0; i < static_cast<int>(ring.size()); i++) {
            double x = ring[i].x();
            double y = ring[i].y();
            std::cout<<"<"<<x<<","<<y<<">";
        }
        std::cout<<std::endl;
    };
#endif

    //結合情報
    Fit fit1 = join_data[0];
    Fit fit2 = join_data[1];

    //それぞれOuterとして持つ
    Ring ring1 = popRingByPolygon(jointed_polygon, jointed_polygon.getInnerSize() == 0 ? -1 : fit1.frame_inner_pos);
    Ring ring2 = popRingByPolygon(piece, -1);
    int size1 = ring1.size();
    int size2 = ring2.size();

#ifdef DEBUG_RING
    debugRing(ring1,__LINE__);
    debugRing(ring2,__LINE__);
#endif

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

#ifdef DEBUG_RING
    debugRing(ring1,__LINE__);
    debugRing(ring2,__LINE__);
#endif

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
                if(fit1.start_dot_or_line == Fit::Line && fit1.is_start_straight == true){
                    // Line is straight. Skip.
                    count++;
                }
            }else{
                count++;
            }
        }
        if (Type == 2) {
            x = ring2[count%size2].x();
            y = ring2[count%size2].y();
            if (count % size2 == (fit2.end_dot_or_line == Fit::Dot ? (((complete_matching_end_pos_2 - 1) % size2 + size2) % size2) : complete_matching_end_pos_2)) {
                if(fit1.end_dot_or_line == Fit::Line && fit1.is_end_straight == true){
                    // Line is straight. Skip.
                    break;
                }else{
                    Type=-1;
                }
            }else{
                count++;
            }
        }
        new_ring.push_back(point_t(x,y));
    } while (Type != -1);

#ifdef DEBUG_RING
    debugRing(new_ring,__LINE__);
#endif

    // Divide polygon to dividedFrameRings
    std::vector<Ring> dividedFrameRings;
    std::function<void(Ring)> divideFrameRing = [&divideFrameRing,&dividedFrameRings](Ring new_ring){
        int new_ring_size = new_ring.size();

        // Search Connection
        std::tuple<bool,bool,int,int> field_divide_data = searchFieldConnection(new_ring);

        if(std::get<0>(field_divide_data) == true){

            // Debug
            Fit::DotORLine start_dot_or_line = std::get<1>(field_divide_data)? Fit::Dot : Fit::Line;
            int start_id = std::get<3>(field_divide_data);
            int end_id = std::get<2>(field_divide_data);
            std::cout<<"Divide!"<<start_dot_or_line<<","<<start_id<<","<<end_id<<std::endl;

            /* Divide and generate twice ring(=polygon) */
            Ring new_left_ring;
            Ring new_right_ring;
            int seek_pos_id = -1;

            // Generate Left Ring
            seek_pos_id = start_id;
            while(seek_pos_id == (start_dot_or_line == Fit::Dot ? end_id : Utilities::inc(end_id,new_ring_size))){
                new_left_ring.push_back(new_ring.at(seek_pos_id));
                Utilities::inc(seek_pos_id,new_ring_size);
            }
            new_left_ring.push_back(new_ring.at(start_id));

            // Generate Right Ring
            seek_pos_id = Utilities::inc(end_id,new_ring_size);
            new_right_ring.push_back(new_ring.at(start_id));
            while(seek_pos_id == (start_dot_or_line == Fit::Dot ? start_id : start_id)){
                new_right_ring.push_back(new_ring.at(seek_pos_id));
                Utilities::inc(seek_pos_id,new_ring_size);
            }

            dividedFrameRings.push_back(new_left_ring);
            dividedFrameRings.push_back(new_right_ring);

            divideFrameRing(new_left_ring);
            divideFrameRing(new_right_ring);

        }else{
            std::cout<<"Don't need devide."<<std::endl;
        }

    };
    divideFrameRing(new_ring);

    // 重複チェック！
    //if(hasConflict(ring1, ring2, fit1, fit2)){
    //    return false;
    //}

    //　ポリゴンにRingを出力しておしまい
    //TODO
    if(jointed_polygon.getInnerSize() != 0){ //frame-piece
        polygon_t new_raw_polygon = pushRingToPolygonT(new_ring, jointed_polygon, fit1.frame_inner_pos);
        jointed_polygon.setMultiIds(std::vector<int>{jointed_polygon.getId(), piece.getId()});
        new_polygon = std::move(jointed_polygon);
        new_polygon.pushNewJointedPolygon(new_raw_polygon, piece);
    }else{ //piece-piece
        throw "Not supported!";
        new_polygon.setMultiIds(std::vector<int>{jointed_polygon.getId(), piece.getId()});
        polygon_t new_raw_polygon = pushRingToPolygonT(new_ring, new_polygon);
        //new_polygon.pushNewJointedPolygon(jointed_polygon, join_data);
        //new_polygon.pushNewJointedPolygon(piece, join_data);
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

std::tuple<bool,bool,int,int> PolygonConnector::searchFieldConnection(std::vector<point_t> inner)
{

    struct Line{
        double a;
        double b;
        double c;
    };

    //calc length point to point
    auto calcPointToPointDistance = [](point_t start,point_t end)->double{
        return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
    };

    //calc length point to line
    auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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
            line.c = -((line.b * start.y()) + (line.a * start.x()));

            /*
            const double a = y1 - y2;
            const double b = x2 - x1;
            const double c = (-b * y1) + (-a * x1);
            */

            return line;
        };

        auto calcPointToLineDistance = [](point_t point,Line line){

            //line.c = 0;

            const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
            const double bunbo = line.a * line.a + line.b * line.b;

            const double distance = bunshi * bunshi / bunbo;

            return distance;
        };



        if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

            //std::cout << "out of hani" << std::endl;

            return 100.0;

        }

        if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

            //std::cout << "out of hani" << std::endl;

            return 100.0;
        }

        Line line = calcLineABC(a,b);

        const double distance = calcPointToLineDistance(c,line);

        return distance;

    };

    auto checkFirstPointHasNearPoint = [](unsigned int i,double gosaaaaaaaaa,procon::Field fieeld){

        auto calcPointToPointDistance = [](point_t start,point_t end)->double{
            return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
        };


        for(unsigned int k = 1; k < ( fieeld.getFrame().getPolygon().inners().at(i).size() ) - 1; k++){

            const double distance_1 = calcPointToPointDistance(fieeld.getFrame().getPolygon().inners().at(i).at(0)
                                                               ,fieeld.getFrame().getPolygon().inners().at(i).at(k));

            if(distance_1 < gosaaaaaaaaa * gosaaaaaaaaa){

                return std::tuple<bool,int>{true,k};

            }


        }

        return std::tuple<bool,int>(false,0);

    };

    auto checkFirstLineHasNearPoint = [](unsigned int i,double gosaaaaaaaaa,procon::Field fieeld){

        auto calcPointToPointDistance = [](point_t start,point_t end)->double{
            return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
        };

        //calc length point to line1
        auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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

                //line.c = 0;

                const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
                const double bunbo = line.a * line.a + line.b * line.b;

                const double distance = bunshi * bunshi / bunbo;

                return distance;
            };



            if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;

            }

            if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;
            }

            Line line = calcLineABC(a,b);

            const double distance = calcPointToLineDistance(c,line);

            return distance;

        };

        const unsigned int inner_size = fieeld.getFrame().getPolygon().inners().at(i).size();

        Fit fit_buf;

        for(unsigned int k = 2; k < inner_size -1; k++ ){

            const double distance_2 = calcLineToDistance(fieeld.getFrame().getPolygon().inners().at(i).at(0)
                                                         ,fieeld.getFrame().getPolygon().inners().at(i).at(1)
                                                         ,fieeld.getFrame().getPolygon().inners().at(i).at(k));

            if(distance_2 < gosaaaaaaaaa * gosaaaaaaaaa){

                return std::tuple<bool,int,int> {true, 0, k };

            }

        }

        return std::tuple<bool,int,int> {false, 0 ,0};

    };

    auto checkPointHasNearPoint = [](unsigned int i,unsigned int k,double gosaaaaaaaaa,procon::Field fieeld){

        //calc length point to point
        auto calcPointToPointDistance = [](point_t start,point_t end)->double{
            return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
        };

        //calc length point to line
        auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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

                //line.c = 0;

                const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
                const double bunbo = line.a * line.a + line.b * line.b;

                const double distance = bunshi * bunshi / bunbo;

                return distance;
            };



            if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;

            }

            if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;
            }

            Line line = calcLineABC(a,b);

            const double distance = calcPointToLineDistance(c,line);

            return distance;

        };


        const unsigned int inner_size = fieeld.getFrame().getPolygon().inners().at(i).size();

        for(unsigned int l = k + 1; l < inner_size - 1; l++ ){

            const double distanceee = calcPointToPointDistance(fieeld.getFrame().getPolygon().inners().at(i).at(k)
                                                               ,fieeld.getFrame().getPolygon().inners().at(i).at(l));

            if(distanceee < gosaaaaaaaaa * gosaaaaaaaaa){

                return std::tuple<bool,int,int> {true, k, l};

            }
        }

        return std::tuple<bool,int,int> {false,0,0};
    };

    auto checkPointHasNearLine = [](unsigned int i,unsigned int k,double gosaaaaaaaaa,procon::Field fieeld){

        //calc length point to point
        auto calcPointToPointDistance = [](point_t start,point_t end)->double{
            return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
        };

        //calc length point to line
        auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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
                line.c = -((line.b * start.y()) + (line.a * start.x()));

                /*
                const double a = y1 - y2;
                const double b = x2 - x1;
                const double c = (-b * y1) + (-a * x1);
                */

                return line;
            };

            auto calcPointToLineDistance = [](point_t point,Line line){


                const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
                const double bunbo = line.a * line.a + line.b * line.b;

                const double distance = bunshi * bunshi / bunbo;

                return distance;
            };



            if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;

            }

            if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

                //std::cout << "out of hani" << std::endl;

                return 100.0;
            }

            Line line = calcLineABC(a,b);

            const double distance = calcPointToLineDistance(c,line);

            return distance;

        };


        const unsigned int inner_size = fieeld.getFrame().getPolygon().inners().at(i).size();

        for(unsigned int l = 0; l < inner_size - 1; l++){

            if(l == k){
                l = l + 2;
            }
            if(k > (inner_size - 2)){
                break;
            }

            if(l > (inner_size - 2)){
                break;
            }

            const double distanceeeee = calcLineToDistance(fieeld.getFrame().getPolygon().inners().at(i).at(k)
                                                           ,fieeld.getFrame().getPolygon().inners().at(i).at(k + 1)
                                                           ,fieeld.getFrame().getPolygon().inners().at(i).at(l));


            if(distanceeeee < ( gosaaaaaaaaa * gosaaaaaaaaa ) ){

                return std::tuple<bool,int,int>{true,k,l};

            }
        }

        return std::tuple<bool,int,int>{false,0,0};

    };

    auto goHasNearPiecePoint = [](unsigned int i,unsigned int k,double gosaaaaaaaaa,std::vector<point_t> inner){

        auto checkPointHasNearPoint = [](unsigned int i,unsigned int k,double gosaaaaaaaaa,std::vector<point_t> inner){

            //calc length point to point
            auto calcPointToPointDistance = [](point_t start,point_t end)->double{
                return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
            };

            //calc length point to line
            auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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

                    //line.c = 0;

                    const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
                    const double bunbo = line.a * line.a + line.b * line.b;

                    const double distance = bunshi * bunshi / bunbo;

                    return distance;
                };



                if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

                    //std::cout << "out of hani" << std::endl;

                    return 100.0;

                }

                if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

                    //std::cout << "out of hani" << std::endl;

                    return 100.0;
                }

                Line line = calcLineABC(a,b);

                const double distance = calcPointToLineDistance(c,line);

                return distance;

            };


            const unsigned int inner_size = inner.size();

            for(unsigned int l = k + 1; l < inner_size - 1; l++ ){

                const double distanceee = calcPointToPointDistance(inner.at(k)
                                                                   ,inner.at(l));

                if(distanceee < gosaaaaaaaaa * gosaaaaaaaaa){

                    return std::tuple<bool,int,int> {true, k, l};

                }
            }

            return std::tuple<bool,int,int> {false,0,0};
        };

        auto checkPointHasNearLine = [](unsigned int i,unsigned int k,double gosaaaaaaaaa,std::vector<point_t> inner){

            //calc length point to point
            auto calcPointToPointDistance = [](point_t start,point_t end)->double{
                return (start.x() - end.x()) * (start.x() - end.x()) + (start.y() - end.y()) * (start.y() - end.y());
            };

            //calc length point to line
            auto calcLineToDistance = [](point_t a,point_t b,point_t c)->double{

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
                    line.c = -((line.b * start.y()) + (line.a * start.x()));

                    /*
                    const double a = y1 - y2;
                    const double b = x2 - x1;
                    const double c = (-b * y1) + (-a * x1);
                    */

                    return line;
                };

                auto calcPointToLineDistance = [](point_t point,Line line){


                    const double bunshi = line.a * point.x() + line.b * point.y() + line.c;
                    const double bunbo = line.a * line.a + line.b * line.b;

                    const double distance = bunshi * bunshi / bunbo;

                    return distance;
                };



                if(Dot(calcVec(a,b),calcVec(a,c)) < 0.0){

                    //std::cout << "out of hani" << std::endl;

                    return 100.0;

                }

                if(Dot(calcVec(b,a),calcVec(b,c)) < 0.0){

                    //std::cout << "out of hani" << std::endl;

                    return 100.0;
                }

                Line line = calcLineABC(a,b);

                const double distance = calcPointToLineDistance(c,line);

                return distance;

            };


            const unsigned int inner_size = inner.size();

            for(unsigned int l = 0; l < inner_size - 1; l++){

                if(l == k){
                    l = l + 2;
                }
                if(k > (inner_size - 2)){
                    break;
                }

                if(l > (inner_size - 2)){
                    break;
                }

                const double distanceeeee = calcLineToDistance(inner.at(k)
                                                               ,inner.at(k + 1)
                                                               ,inner.at(l));


                if(distanceeeee < ( gosaaaaaaaaa * gosaaaaaaaaa ) ){

                    return std::tuple<bool,int,int>{true,k,l};

                }
            }

            return std::tuple<bool,int,int>{false,0,0};

        };

        const unsigned int inner_size = inner.size();

        for(unsigned int l = k; l < inner_size; ++l){

            std::tuple<bool,int,int> result_1 = checkPointHasNearPoint(i,l,gosaaaaaaaaa,inner);

            if(std::get<0>(result_1)){

                return std::tuple<bool,bool,int,int> {std::get<0>(result_1),true,std::get<1>(result_1),std::get<2>(result_1)};

                break;

            }
        }


        for(unsigned int l = k; l < inner_size; ++l){

            std::tuple<bool,int,int> result_2 = checkPointHasNearLine(i,l,gosaaaaaaaaa,inner);

            if(std::get<0>(result_2)){

                return std::tuple<bool,bool,int,int> {std::get<0>(result_2),false,std::get<1>(result_2),std::get<2>(result_2)};

                break;

            }

        }

        return std::tuple<bool,bool,int,int> {false,false,0,0};

    };


    constexpr double gosaaaaaaaaaaaaaaaa = 0.1;

    /*
    for(unsigned int i = 0; i < field.getFrame().getPolygon().inners().size(); i++){

        /*
        std::tuple<bool,int> result = checkFirstPointHasNearPoint(i,gosaaaaaaaaaaaaaaaa,field);

        std::cout << "weeei"<<std::get<0>(result) << std::endl;
        std::cout << "weeei"<<std::get<1>(result) << std::endl;

        std::tuple<bool,int,int> ressslut = checkFirstLineHasNearPoint(i,gosaaaaaaaaaaaaaaaa,field);

        std::cout << "weeeeeei" << std::get<0>(ressslut) << std::endl;

        for(int a = 0; a < field.getFrame().getPolygon().inners().at(i).size(); a++){

            std::tuple<bool,int,int> reeeesullltttt = checkPointHasNearPoint(i,a,gosaaaaaaaaaaaaaaaa,field);
            std::cout << "true:false" << std::get<0>(reeeesullltttt) << std::endl;
            std::cout << "pointnum" << std::get<1>(reeeesullltttt) << std::endl;
            std::cout << "dor" << std::get<2>(reeeesullltttt) << std::endl;

        }

        for(int a = 0; a < field.getFrame().getPolygon().inners().at(i).size(); a++){

            std::tuple<bool,int,int> reeeesullltttt = checkPointHasNearLine(i,a,gosaaaaaaaaaaaaaaaa,field);
            std::cout << "true:false" << std::get<0>(reeeesullltttt) << std::endl;
            std::cout << "pointnum" << std::get<1>(reeeesullltttt) << std::endl;
            std::cout << "dor" << std::get<2>(reeeesullltttt) << std::endl;

        }

        std::cout << "weiriieirwire" << calcLineToDistance(point_t(15,15),point_t(0,15),point_t(5,0)) << std::endl;

        std::tuple<bool,bool,int,int> resultttttttt = goHasNearPiecePoint(i,1,gosaaaaaaaaaaaaaaaa,field);

        std::cout << "startnumber" << std::get<0>(resultttttttt) << std::endl;
        std::cout << "startnumber" << std::get<1>(resultttttttt) << std::endl;
        std::cout << "startnumber" << std::get<2>(resultttttttt) << std::endl;
        std::cout << "startnumber" << std::get<3>(resultttttttt) << std::endl;
        /

        std::tuple<bool,bool,int,int> resulttttt = goHasNearPiecePoint(i,0,gosaaaaaaaaaaaaaaaa,field);

        return resulttttt;
    }
    *//*
    std::tuple<bool,bool,int,int> resultttttttt = goHasNearPiecePoint(0,0,gosaaaaaaaaaaaaaaaa,inner);

    std::cout << "startnumber" << std::get<0>(resultttttttt) << std::endl;
    std::cout << "startnumber" << std::get<1>(resultttttttt) << std::endl;
    std::cout << "startnumber" << std::get<2>(resultttttttt) << std::endl;
    std::cout << "startnumber" << std::get<3>(resultttttttt) << std::endl;

    */
    return goHasNearPiecePoint(0,0,gosaaaaaaaaaaaaaaaa,inner);
}
