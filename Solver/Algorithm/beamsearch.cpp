#include "beamsearch.h"
#include "field.h"
#include "fit.h"
#include "Utils/evaluation.h"
#include "Utils/polygonconnector.h"
#include <mutex>
#include <thread>
#include "parallel.h"
#include <random>
#include "utilities.h"

//#define NO_PARALLEL

BeamSearch::BeamSearch()
{
    this->initialization();
}

void BeamSearch::initialization()
{
    cpu_num = std::thread::hardware_concurrency();
#ifndef NO_PARALLEL
    beam_width = 100;
#else
    beam_width = 100;
#endif
    variety_width = 20;
}

void BeamSearch::evaluateNextMove (std::vector<Evaluation> & evaluations,std::vector<procon::Field> const& field_vec)
{
    auto fieldSearch = [&](procon::Field const& field)
    {
        std::vector<Evaluation> evaluations;
        for (int j = 0;j < static_cast<int>(field.getElementaryPieces().size());j++){
            if (field.getIsPlaced().at(j)) continue;
            std::vector<Evaluation> eva = evaluateCombinationByAngle(field.getFrame(),field.getElementaryPieces().at(j));
            std::vector<Evaluation> eva_inverse = evaluateCombinationByAngle(field.getFrame(),field.getElementaryInversePieces().at(j));
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

    procon::Parallel parallel;

    int const field_vec_size = static_cast<int>(field_vec.size());

    auto evaluateRange = [&](int start_id,int end_id)
    {
        for (int j = start_id;j < end_id;j++) {
            std::vector<Evaluation> eva = fieldSearch(field_vec.at(j));
            for (auto & e : eva) {
                e.vector_id = j;
            }
            {
                //std::lock_guard<std::mutex> ld(parallel.mutex());と同じ
                MUTEX_LOCK(parallel);
                std::copy(eva.begin(),eva.end(),std::back_inserter(evaluations));
            }
        }
    };

#ifndef NO_PARALLEL
    /**cpuのスレッド数に合わせてvectorを分割し，それぞれスレッドに投げ込む**/
    parallel.generateThreads(evaluateRange,cpu_num,0,field_vec_size);
    /**スレッド終わるの待ち**/
    parallel.joinThreads();
#else
    evaluateRange(0,field_vec_size);
#endif

}

std::vector<procon::Field> BeamSearch::makeNextField (std::vector<Evaluation> const& evaluations,std::vector<procon::Field> const& field_vec)
{
    std::vector<procon::Field> next_field_vec;
    procon::Parallel parallel;

    auto makeField = [&](int start_id,int end_id){
        for (int j = start_id;j < end_id;j++) {
            int const vec_id = evaluations.at(j).vector_id;
            int const piece_id = evaluations.at(j).piece_id;
            std::array<Fit,2> const fits = evaluations.at(j).fits;
            procon::ExpandedPolygon const old_frame = field_vec.at(vec_id).getFrame();
            procon::ExpandedPolygon const old_piece =
            (evaluations.at(j).inverse_flag) ?
                field_vec.at(vec_id).getElementaryInversePieces().at(piece_id)
            :
                field_vec.at(vec_id).getElementaryPieces().at(piece_id)
            ;
            procon::ExpandedPolygon new_frame;

            bool hasJoinSuccess = PolygonConnector::joinPolygon(old_frame,old_piece,new_frame,fits);
            double const min_angle = field_vec.at(vec_id).getMinAngle();

            if (hasJoinSuccess  && !canPrune(new_frame,min_angle) ) {
                procon::Field new_field = field_vec.at(vec_id);
                new_field.sumTotalEvaluation(evaluations.at(j).evaluation);
                new_field.setFrame(new_frame);
                new_field.setIsPlaced(piece_id);

                {
                    MUTEX_LOCK(parallel);
                    next_field_vec.push_back(std::move(new_field));
                }
            }
        }
    };
    //過剰生産倍率
    //constexpr double size_weight = 1.2;
    //限界サイズ
    const int limit = static_cast<int>(evaluations.size());

#ifndef NO_PARALLEL
    //マルチスレッド
    int i;
    for (i = 1;static_cast<int>(next_field_vec.size()) < beam_width;i++) {
        if (i * beam_width > limit) {
            parallel.generateThreads(makeField,cpu_num,(i - 1) * beam_width,limit);
            parallel.joinThreads();
            return next_field_vec;
        } else {
            parallel.generateThreads(makeField,cpu_num,(i - 1) * beam_width,i * beam_width);
            parallel.joinThreads();
        }

        //重複しているfieldの除去
        removeDuplicateField(next_field_vec);
    }
#else
    makeField(0,limit);
#endif

#ifndef NO_PARALLEL
    if (static_cast<int>(next_field_vec.size()) > beam_width) next_field_vec.resize(beam_width);

    if (limit - (i - 1) *  beam_width > variety_width) {
        auto makeRandomVector = [&](int start,int end){
            std::vector<int> random_vec;
            std::random_device rd;
            std::mt19937 mt(rd());
            for (int i = start;i < end;i++){
                random_vec.emplace_back(i);
            }
            std::uniform_int_distribution<int> variety(start,end - 1);
            for (int i = end - start - 1;i >= 0;i--){
                std::swap(random_vec.at(variety(mt) - start),random_vec.at(i));
            }
            return random_vec;
        };

        std::vector<int> random_vec = std::move(makeRandomVector((i - 1) * beam_width,limit));
        int falut = 0;
        while (static_cast<int>(next_field_vec.size()) < beam_width + variety_width && falut + variety_width < (limit - ( (i - 1) * beam_width) )) {
            std::vector<std::thread> threads;
            for (int i = 0;i < variety_width;i++) {
                std::thread thread(makeField,random_vec.at(i + falut),random_vec.at(i + falut) + 1);
                threads.emplace_back(std::move(thread));
            }
            for (int i = 0;i < variety_width;i++) {
                threads.at(i).join();
            }
            falut += variety_width;
        }
    }

#endif
    if (static_cast<int>(next_field_vec.size()) > beam_width + variety_width) next_field_vec.resize(beam_width + variety_width);
    return next_field_vec;

}

bool BeamSearch::canPrune(procon::ExpandedPolygon const& next_frame ,double const& min_angle) {
    bool can_prune = false;
    for (auto const& angles : next_frame.getInnersSideAngle()) {
        for (auto const& angle : angles){
            if (angle < min_angle) {
                return true;
            }
        }
    }
    for (auto const& angles : next_frame.getInnersSideAngle()) {
        for (auto const& angle : angles) {
            auto isAngleExist = [&](double const& angle)
            {
                int _angle = static_cast<int>(angle) / exist_resolution;
                return this->angle_exist.at(_angle);
            };
            constexpr double to_rad = 180 / 3.1415926535;
            if (!isAngleExist(angle * to_rad)) {
                return true;
            }
        }

    }
    return  can_prune;
}

bool BeamSearch::removeDuplicateField(std::vector<procon::Field> & field_vec)
{
    if (field_vec.size() == 0) return false;
    auto poor_unique_move = [&](std::vector<procon::Field>::iterator begin,std::vector<procon::Field>::iterator end)
    {
        //*おおっと*線形*つらい*
        std::vector<procon::Field> ret_vector;
        ret_vector.reserve(static_cast<int>(end - begin));
        for(auto it_i = begin;it_i < end - 1;it_i++) {
            bool check_overlap = false;
            for (auto it_j = it_i + 1;it_j < end - 1;it_j++) {
                if (*it_i || *it_j) {
                    check_overlap = true;
                }
            }
            if (!check_overlap) ret_vector.emplace_back(*(it_i));
        }
        return std::move(ret_vector);
    };
    std::for_each(field_vec.begin(),field_vec.end(),[](procon::Field & field){field.calcFieldID();});
    field_vec = std::move(poor_unique_move(field_vec.begin(),field_vec.end()));
    return true;
}

void BeamSearch::run(procon::Field field)
{
    auto sortEvaLambda = [&](Evaluation const& a,Evaluation const& b)->bool
    {
        return a.evaluation > b.evaluation;
    };
    calcAngleFrequency(field);
    calcLengthFrequency(field);
    calcAngleExist(field);
    std::vector<procon::Field> field_vec;
    std::vector<Evaluation> evaluations;

    field.setFrame(field.getElementaryFrame());
    field_vec.push_back(field);
    procon::Field buckup_field;

    //ピースが全部置かれたら終了
    //このiは添字として使ってるわけではない（ただの回数ルーブ）
    for (int i = 0;i < static_cast<int>(field.getElementaryPieces().size());i++) {
        evaluations.clear();

        //最小角計算(後のpruningで使用)
        for (int j = 0;j < static_cast<int>(field_vec.size());j++) {
            field_vec.at(j).calcMinAngleSide();
        }

        buckup_field = field_vec.at(0);
        this->evaluateNextMove(evaluations,field_vec);
        this->evaluateHistoryInit(field_vec);
        for(Evaluation & evaluation: evaluations) {
#ifdef HYOKA_MODE
            evaluation.evaluation_normal = evaluation.evaluation;
            evaluation.evaluation_angle = alpha * this->evaluateUniqueAngle(evaluation,field_vec);
            evaluation.evaluation_length = beta * this->evaluateUniqueLength(evaluation,field_vec);
            evaluation.evaluation_history = gamma * this->evaluateHistory(evaluation,field_vec);
            evaluation.evaluation_frame = delta * this->evaluateFrame(evaluation,field_vec);
#endif
            //std::cout << "alpha" << std::endl;
            evaluation.evaluation += alpha * this->evaluateUniqueAngle(evaluation,field_vec);
            //std::cout << "beta" << std::endl;
            evaluation.evaluation += beta * this->evaluateUniqueLength(evaluation,field_vec);
            //std::cout << "gamma" << std::endl;
            evaluation.evaluation += gamma * this->evaluateHistory(evaluation,field_vec);
            //std::cout << "delta" << std::endl;
            evaluation.evaluation += delta * this->evaluateFrame(evaluation,field_vec);
        }

        if (evaluations.empty()){
            submitAnswer(buckup_field);
            return;
        }

        std::sort(evaluations.begin(),evaluations.end(),sortEvaLambda);
        field_vec = std::move(this->makeNextField(evaluations,field_vec));

        //return field_vec[0];
        //結合できるものがなければその１手前の最高評価地のフィールドを返す
        if(field_vec.empty()){
            submitAnswer(buckup_field);
            return;
        }

        // Output Answer
        int cnt = 0;
        for(auto& field: field_vec){
#ifdef HYOKA_MODE
            field.evaluation_normal = evaluations.at(cnt).evaluation_normal;
            field.evaluation_angle = evaluations.at(cnt).evaluation_angle;
            field.evaluation_length = evaluations.at(cnt).evaluation_length;
            field.evaluation_history = evaluations.at(cnt).evaluation_history;
            field.evaluation_frame = evaluations.at(cnt).evaluation_frame;
#endif
            field.evaluation_sum = evaluations.at(cnt).evaluation;
            DOCK->addAnswer(field);
            cnt++;
        }
        submitAnswer(field_vec.at(0));
    }
    return;
}
