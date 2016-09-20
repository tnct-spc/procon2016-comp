#include "polygonconnector.h"

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

//重複を見つける。実装途中なのでとても汚い akkasita
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
        bg::model::segment<point_t> line1(ring1[ring1_pos],ring1[(ring1_pos+1)%size1]);

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
                    (ring1_yello_start_zone && !ring2_orange_end_zone && (ring2_yello_start_zone || ring2_white_zone || ring2_yellow_end_zone || ring2_orange_end_zone || ring2_red_zone))
              ){
                //make ring
                bg::model::segment<point_t> line2(ring2[ring2_pos],ring2[((ring2_pos - 1)%size2+size2)%size2]);

                //check conflict
                if(static_cast<bool>(bg::intersects(line1, line2))){
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
