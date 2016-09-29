#include "chokudaisearch.h"
#include "timecounter.h"
#include "parallel.h"
#include "Utils/polygonconnector.h"
#include <thread>

ChokudaiSearch::ChokudaiSearch()
{

}

void ChokudaiSearch::initialization()
{
    beam_width = 1;
    cpu_num = std::thread::hardware_concurrency();
}

std::vector<NeoField> ChokudaiSearch::evaluateNextMove(NeoField const& field)
{
    auto fieldSearch = [&](procon::Field const& field)
    {
        std::vector<Evaluation> evaluations;
        for (int j = 0;j < static_cast<int>(field.getElementaryPieces().size());j++){
            if (field.getIsPlaced().at(j)) continue;
            std::vector<Evaluation> eva = evaluateCombinationByLength(field.getFlame(),field.getElementaryPieces().at(j));
            std::vector<Evaluation> eva_inverse = evaluateCombinationByLength(field.getFlame(),field.getElementaryInversePieces().at(j));
            for (auto & e : eva){
                e.piece_id = j;
            }
            for (auto & e : eva_inverse) {
                e.piece_id = j;
                e.inverse_flag = true;
            }
            std::copy(eva.begin(),eva.end(),std::back_inserter(evaluations));
            std::copy(eva_inverse.begin(),eva_inverse.end(),std::back_inserter(evaluations));
        }
        return evaluations;
    };

    std::vector<Evaluation> tmp_eva = fieldSearch(field.field);
    std::vector<NeoField> ret_field;
    for (auto eva : tmp_eva) {
        NeoField neo_field;
        neo_field.field = field.field;
        neo_field.eva = std::move(eva);
        ret_field.emplace_back(neo_field);
    }
    return std::move(ret_field);
}

std::vector<NeoField> ChokudaiSearch::makeNextField(std::vector<NeoField> const& fields)
{
    std::vector<NeoField> next_fields;
    procon::Parallel parallel;

    auto makeField = [&](int start_id,int end_id){
        for (int j = start_id;j < end_id;j++) {
            int const piece_id = fields.at(j).eva.piece_id;
            std::array<Fit,2> const fits = fields.at(j).eva.fits;
            procon::ExpandedPolygon const old_frame = fields.at(j).field.getFlame();
            procon::ExpandedPolygon const old_piece =
            (fields.at(j).eva.inverse_flag) ?
                fields.at(j).field.getElementaryInversePieces().at(piece_id)
            :
                fields.at(j).field.getElementaryPieces().at(piece_id)
            ;
            procon::ExpandedPolygon new_frame;

            bool hasJoinSuccess = PolygonConnector::joinPolygon(old_frame,old_piece,new_frame,fits);
            double const min_angle = fields.at(j).field.getMinAngle();

            if (hasJoinSuccess  && !canPrune(new_frame,min_angle) ) {
                NeoField new_field = fields.at(j);
                new_field.field.setFlame(new_frame);
                new_field.field.setIsPlaced(piece_id);

                {
                    MUTEX_LOCK(parallel);
                    next_fields.emplace_back(new_field);
                }
            }
        }
    };

    //int const width = beam_width < static_cast<int>(fields.size()) ? beam_width : static_cast<int>(fields.size());
    int const width = static_cast<int>(fields.size());
    /**cpuのスレッド数に合わせてvectorを分割し，それぞれスレッドに投げ込む**/
    //parallel.generateThreads(makeField,cpu_num,0,width);

    /**スレッド終わるの待ち**/
    //parallel.joinThreads();
    makeField(0,width);
    return std::move(next_fields);

}

procon::Field ChokudaiSearch::run(procon::Field field)
{
    auto sortEvaLambda = [&](NeoField const& a,NeoField const& b)->bool
    {
        return a.eva.evaluation > b.eva.evaluation;
    };


    //一度生み出されたフィールドを保存する配列
    std::vector<std::vector<NeoField>> neo_fields;
    std::vector<std::vector<NeoField>> next_neo_fields;
    neo_fields.resize(51);
    next_neo_fields.resize(51);
    field.setFlame(field.getElementaryFlame());
    //beamwidth幅ビームサーチ
    //返り値は最適解
    //キャプチャ:next_neo_fields，beam_width
    auto beamSearch = [&](NeoField const& field)
    {
        std::vector<NeoField> field_vec;
        field_vec.emplace_back(field);

        //ピースが全部置かれたら終了
        //i は階層
        for (int i = 0;i < static_cast<int>(field.field.getElementaryPieces().size());i++){

            if (neo_fields.at(i).size() < beam_width) continue;

            //評価
            field_vec = this->evaluateNextMove(neo_fields.at(i).at(beam_width - 1));

            //それより先がなければその1手前の最高評価値のフィールドを返す
            if (field_vec.empty());

            //評価値が高い順にソート
            std::sort(field_vec.begin(),field_vec.end(),sortEvaLambda);

            //作成したNeoFieldをすべて保存 いっぱい->makeNextField->ビーム幅
            //次回の評価に使用
            std::copy(field_vec.begin(),field_vec.end(),std::back_inserter(neo_fields.at(i + 1)));

            //結合
            field_vec = std::move(makeNextField(field_vec));

            //結合できるものがなければその１手前の最高評価地のフィールドを返す
            if(field_vec.empty()) return;

            //評価値が高い順にソート
            std::sort(field_vec.begin(),field_vec.end(),sortEvaLambda);

            for (int j = 0;j < static_cast<int>(field_vec.size());j++) {
                neo_fields.at(i + 1).at(j) = std::move(field_vec.at(j));
            }

            field_vec.clear();
        }
        //return field_vec.at(0);
    };

    NeoField tmp;
    tmp.field = field;
    neo_fields.at(0).push_back(tmp);

    procon::TimeCounter time_counter;
    time_counter.startTimer();
    constexpr double wait_time = 114514; //114s

    //一定時間を超えるまで
    while(time_counter.getElapsedTime() < wait_time) {
        beamSearch(tmp);
        beam_width++;
    }
    std::cout << beam_width << std::endl;
    for (int i = 0;i < neo_fields.size();i++) {
        if (neo_fields.at(i).empty()) {
            std::cout << neo_fields.at(i - 1).at(0).field.getFieldScore() << std::endl;
            return neo_fields.at(i - 1).at(0).field;
        }
    }
    return field;
}

