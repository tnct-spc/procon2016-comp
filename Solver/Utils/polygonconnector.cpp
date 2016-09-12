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
        for(int i=inner_size-1; i > 0; --i){ //not copy i==0
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
        for(int i=ring_size-1; i >= 0; --i){
            inner.push_back(ring[i]);
        }
        new_raw_polygon.inners().at(inner_position).clear();
        for(auto point : inner){
            new_raw_polygon.inners().at(inner_position).push_back(point);
        }
    }

    polygon.setPolygon(new_raw_polygon);
}

//ポリゴンを合体する関数本体
bool PolygonConnector::joinPolygon(procon::ExpandedPolygon Polygon1, procon::ExpandedPolygon Polygon2, procon::ExpandedPolygon& new_polygon, std::array<Fit,2> join_data)
{
    //結合情報
    Fit fit1 = join_data[0];
    Fit fit2 = join_data[1];

    //それぞれOuterとして持つ
    Ring ring1 = popRingByPolygon(Polygon1, Polygon1.getInnerSize() == 0 ? -1 : fit1.flame_inner_pos);
    Ring ring2 = popRingByPolygon(Polygon2, Polygon2.getInnerSize() == 0 ? -1 : fit2.flame_inner_pos);
    int size1 = ring1.size();
    int size2 = ring2.size();

    auto debugRing = [](Ring ring, int line){
        std::cout<<std::to_string(line)<<" : ";
        for (int i=0; i < static_cast<int>(ring.size()); i++) {
            double x = ring[i].x();
            double y = ring[i].y();
            std::cout<<"<"<<x<<","<<y<<">";
        }
        std::cout<<std::endl;
    };

    debugRing(ring1,__LINE__);
    debugRing(ring2,__LINE__);

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
    Ring turned_ring;
    for (auto point : ring2)
    {
        const double x = point.x() - ring2[0].x();
        const double y = point.y() - ring2[0].y();
        const double move_x = (x * cos(rotate_radian)) - (y * sin(rotate_radian));
        const double move_y = (x * sin(rotate_radian)) + (y * cos(rotate_radian));
        const double turn_x = move_x + ring2[0].x();
        const double turn_y = move_y + ring2[0].y();
        turned_ring.push_back(point_t(turn_x, turn_y));
    }
    ring2 = turned_ring;


    debugRing(ring1,__LINE__);
    debugRing(ring2,__LINE__);

    // 移動　結合後に一致する点とその次の点を用いて、ポリゴンのx,y移動を調べ、Polygon2を平行移動
    const int Join_point1 = (complete_matching_start_pos_1 + 1) % size1;
    const int Join_point2 = ((complete_matching_start_pos_2 - 1) % size2 + size2) % size2;
    const double move_x = ring1[Join_point1].x() - ring2[Join_point2].x();
    const double move_y = ring1[Join_point1].y() - ring2[Join_point2].y();
    Ring translated_ring;
    for (auto point : ring2)
    {
        translated_ring.push_back(point_t(point.x() + move_x, point.y() + move_y));
    }
    ring2 = translated_ring;

    // 重複チェック！
    bool conf=false;
    //TODO:未完成 if(hasConflict(ring1, ring2, fit1, fit2)) conf=true;

    debugRing(ring1,__LINE__);
    debugRing(ring2,__LINE__);

    // 結合　新しいRingに結合後の外周の角を入れる。
    // もし、結合端の辺の長さが等しくならない時はRing1,Ring2ともに端の角を入力。
    // ここで回転の誤差により角が一致しない場合がある。
    Ring new_ring;
    int count = complete_matching_end_pos_1 + 1;
    int Type = 1;

    double x,y;
    //TODO:ここバグでループしてそう。。。sorry
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

    debugRing(new_ring,__LINE__);

    //　ポリゴンにRingを出力しておしまい
    if(Polygon1.getInnerSize() != 0){
        pushRingToPolygon(new_ring, Polygon1, fit1.flame_inner_pos);
        Polygon1.setMultiIds(std::vector<int>{Polygon1.getId(), Polygon2.getId()});

        new_polygon = std::move(Polygon1);

    }else if(Polygon2.getInnerSize() != 0){
        pushRingToPolygon(new_ring, Polygon2, fit2.flame_inner_pos);
        Polygon2.setMultiIds(std::vector<int>{Polygon2.getId(), Polygon1.getId()});

        new_polygon = std::move(Polygon2);

    }else{
        new_polygon.setMultiIds(std::vector<int>{Polygon1.getId(), Polygon2.getId()});
        pushRingToPolygon(new_ring, new_polygon);
    }
    if(conf) return false;
    return true;
}

//重複を見つける。実装途中なのでとても汚い
bool PolygonConnector::hasConflict(Ring ring1, Ring ring2, Fit fit1, Fit fit2)
{
    int size1 = ring1.size();
    int size2 = ring2.size();
    //safe line
    int safe_start_pos_1 = fit1.start_dot_or_line == Fit::Dot ? decrement(fit1.start_id, size1) : fit1.start_id                  ;
    int safe_end_pos_1   = fit1.end_dot_or_line   == Fit::Dot ? increment(fit1.end_id, size1)   : increment(fit1.end_id, size1)  ;
    int safe_start_pos_2 = fit2.start_dot_or_line == Fit::Dot ? increment(fit2.start_id, size2) : increment(fit2.start_id, size2);
    int safe_end_pos_2   = fit2.end_dot_or_line   == Fit::Dot ? decrement(fit2.end_id, size2)   : fit2.end_id                    ;
    //↓ここおかしいのでいろいろ変える
    safe_end_pos_1 = increment(safe_end_pos_1, size1);
    safe_end_pos_2 = decrement(safe_end_pos_2, size2);

    bool ring1_safe_zone = true;
    bool ring1_yellow_zone = false;
    for(int i=0;i<size1;++i){
        if(ring1_yellow_zone){
            ring1_yellow_zone = false;
        }
        if((safe_start_pos_1+i) >= size1-1){
            ring1_yellow_zone = true;
        }
        if(i!=0 && (safe_start_pos_1+i)%size1 == safe_end_pos_1){
            ring1_safe_zone = false;
        }
        bg::model::segment<point_t> line1(ring1[(safe_start_pos_1+i)%size1],ring1[(safe_start_pos_1+i+1)%size1]);
        for(int j=0;j<size2;++j){
            //jump ring2's safe zone
            if(j==0 && ring1_safe_zone) j = safe_start_pos_2 - ( (safe_start_pos_2 >= safe_end_pos_2) ? safe_end_pos_2 : (safe_end_pos_2-size2) );
            if((safe_end_pos_2+j) >= size2-1 && ring1_safe_zone) break;
            if(j==0 && ring1_yellow_zone) j =                  safe_start_pos_2 - ( (safe_start_pos_2 >= safe_end_pos_2) ? safe_end_pos_2 : (safe_end_pos_2-size2) );
            bg::model::segment<point_t> line2(ring2[(safe_end_pos_2+j)%size2],ring2[(safe_end_pos_2+j+1)%size2]);
            if(static_cast<bool>(bg::intersects(line1, line2))){
                return true;
            }
        }
    }
    return false;
}