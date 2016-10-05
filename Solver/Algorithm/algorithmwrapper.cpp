#include "algorithmwrapper.h"

#include "utilities.h"
#include "Utils/polygonconnector.h"
#include "polygonviewer.h"
#include "utilities.h"
#include "Evaluation/searchsamelength.h"
#include <QTimer>

void AlgorithmWrapper::init()
{
    DOCK = std::make_shared<AnswerDock>();
    DOCK->showMinimized();
}

void AlgorithmWrapper::submitAnswer(procon::Field field)
{
    emit throwAnswer(field);
    // Wait 1msec
    QEventLoop loop;
    QTimer::singleShot(500, &loop, SLOT(quit()));
    loop.exec();
}

void AlgorithmWrapper::run(procon::Field field)
{
    field.setFrame(field.getElementaryFrame());
    submitAnswer(field);
    return;
}

void AlgorithmWrapper::calcAngleFrequency(procon::Field field)
{
    angle_frequency.resize(360 / angle_resolution);
    constexpr double to_deg = 180 / 3.1415926535;
    auto pieces = field.getElementaryPieces();
    for (auto piece : pieces) {
        auto angles = piece.getSideAngle();
        for (auto angle : angles) {
            int num = static_cast<int>(angle * to_deg / angle_resolution);
            angle_frequency.at(num) += 1;
        }
    }
    for (auto& angle : angle_frequency) {
        if (angle == 0) {
            angle = 1;
        }
    }

    for (auto& angle : angle_frequency) {
        double real_max = *(std::max_element(angle_frequency.begin(),angle_frequency.end()));
        double real_min = *(std::min_element(angle_frequency.begin(),angle_frequency.end()));
        auto linerFunction = [&](double x)->double
        {
            return ((angle_ideal_max - angle_ideal_min) / (real_min - real_max)) * (x - real_max) + angle_ideal_min;
        };
        auto exponentialFunction = [&](double x)->double
        {
            return std::pow(angle_base,-angle_alpha * x) + angle_beta;
        };

        //angle = linerFunction(angle);
        angle = exponentialFunction(angle);
    }
}

void AlgorithmWrapper::calcLengthFrequency(procon::Field field)
{
    length_frequency.resize(360 / length_resolution);
    auto & pieces = field.getElementaryPieces();
    for (auto& piece : pieces) {
        auto lengths = piece.getSideLength();
        for (auto& length : lengths) {
            int num = static_cast<int>(length / length_resolution);
            length_frequency.at(num) += 1;
        }
    }
    for (auto &length : length_frequency) {
        if (length == 0) length = 1;
        auto exponentialFunction = [&](double x)->double
        {
            return std::pow(length_base,-length_alpha * x) + length_beta;
        };
        length = exponentialFunction(length);
    }
    true;
}

std::vector<Evaluation> AlgorithmWrapper::evaluateCombinationByAngle(procon::ExpandedPolygon const& frame, procon::ExpandedPolygon const& piece)
{
    double frame_first = 0;
    double piece_first = 0;
    std::vector<Evaluation> evaluations;

    for (unsigned int k = 0; k < frame.getPolygon().inners().size(); ++k){
        const int inner_size = static_cast<int>(frame.getPolygon().inners().at(k).size() - 1);
        for (int i = 0; i < inner_size; ++i) {
            frame_first = frame.getInnersSideAngle().at(k).at(i);
            for (int j = 0; j < piece.getSize(); ++j) {
                piece_first = piece.getSideAngle()[j];

                if(Utilities::nearlyEqual(frame_first,piece_first,angle_error)){
                    std::array<Fit,2> fits;
                    fits.at(0).start_id = i;
                    fits.at(0).end_id = i;
                    fits.at(0).frame_inner_pos = k;
                    fits.at(0).start_dot_or_line = Fit::Dot;
                    fits.at(0).end_dot_or_line = Fit::Dot;
                    fits.at(1).start_id = j;
                    fits.at(1).end_id = j;
                    fits.at(1).start_dot_or_line = Fit::Dot;
                    fits.at(1).end_dot_or_line = Fit::Dot;
                    Evaluation eva;
                    eva.frame_id = k;
                    eva.fits = fits;
                    eva.evaluation = bg::area(piece.getPolygon()) * angle_frequency.at((int)piece_first / angle_resolution);
                    evaluations.push_back(eva);
                }
            }
        }
    }
    return evaluations;
}


std::vector<Evaluation> AlgorithmWrapper::evaluateCombinationByLength(procon::ExpandedPolygon const& frame, procon::ExpandedPolygon const& piece)
{
    std::vector<Evaluation> evaluations;

    std::vector<std::array<Fit,2>> fits;
    std::vector<int> eva_point = SearchSameLength::evaluateMatching(frame, piece, fits);

    for(unsigned int fit_number=0; fit_number < fits.size(); ++fit_number){
        if(eva_point[fit_number] > 0){
            Evaluation eva;
            eva.frame_id = fits[fit_number].at(0).frame_inner_pos;
            eva.fits = fits[fit_number];
            eva.evaluation = eva_point[fit_number];
            evaluations.push_back(eva);
        }
    }

    return evaluations;
}

double AlgorithmWrapper::evaluateUniqueAngle(Evaluation const& evaluation,std::vector<procon::Field> const& field_vec)
{
    double max = this->angle_beta;
    //at(1)なのはピース側のため
    double const& start_id =
        evaluation.fits.at(1).start_dot_or_line == Fit::Line
        ? evaluation.fits.at(1).start_id + 1
        : evaluation.fits.at(1).start_id;
    double const& end_id = evaluation.fits.at(1).end_id;
    procon::ExpandedPolygon const& piece = field_vec.at(evaluation.vector_id).getElementaryPieces().at(evaluation.piece_id);
    constexpr double to_deg = 180 / 3.141592653589;
    for(int i = start_id; i < end_id + 1; i++){
        const double angle = piece.getSideAngle().at(i);
        double tmp = this->angle_frequency.at(static_cast<int>((angle / angle_resolution) * to_deg));
        if (tmp > max) max = tmp;
    }
    return max;
}

double AlgorithmWrapper::evaluateUniqueLength(Evaluation const& evaluation,std::vector<procon::Field> const& field_vec)
{

}

AlgorithmWrapper::AlgorithmWrapper()
{
}
