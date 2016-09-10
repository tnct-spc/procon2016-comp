#include "singlepolygondisplay.h"
#include "ui_singlepolygondisplay.h"

#include <QPainter>
#include <QPen>

SinglePolygonDisplay::SinglePolygonDisplay(QWidget *parent = 0) :
    QWidget(parent),
    ui(new Ui::SinglePolygonDisplay)
{
    ui->setupUi(this);
}

std::unique_ptr<SinglePolygonDisplay> SinglePolygonDisplay::create(procon::ExpandedPolygon& polygon, std::string wname_, int scale)
{
    std::unique_ptr<SinglePolygonDisplay> instance(new SinglePolygonDisplay());
    instance->polygon = polygon;
    instance->scale = scale;
    QString wname = QString::fromStdString(wname_);
    instance->setWindowTitle(wname);
    return std::move(instance);
}

SinglePolygonDisplay::~SinglePolygonDisplay()
{
    delete ui;
}

void SinglePolygonDisplay::paintEvent(QPaintEvent *)
{
    int window_height = this->height();
    int window_width = this->width();
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 3));

    auto drawPolygon = [&](std::vector<QPointF> points){
        static const double margin = 10;
        const int size = points.size();
        const double x_max = (std::max_element(points.begin(), points.end(), [](QPointF a, QPointF b){return a.x() < b.x();}))->x();
        const double y_max = (std::max_element(points.begin(), points.end(), [](QPointF a, QPointF b){return a.y() < b.y();}))->y();
        const double x_min = (std::min_element(points.begin(), points.end(), [](QPointF a, QPointF b){return a.x() < b.x();}))->x();
        const double y_min = (std::min_element(points.begin(), points.end(), [](QPointF a, QPointF b){return a.y() < b.y();}))->y();
        const double width  = x_max - x_min;
        const double height = y_max - y_min;
        const double max = scale != -1 ? scale : width < height ? height : width;
        const double y = window_height - margin;
        const double x = window_height - margin;
        const double y_margin = margin/2;
        const double x_margin = (window_width-window_height)/2 + margin/2;
        const double x_offset = - (x_min + (x_max - x_min)/2);
        const double y_offset = - (y_min + (y_max - y_min)/2);
        std::cout<<"xmaxmin"<<x_max<<","<<x_min<<"wid"<<width<<"hei"<<height<<"xOffset"<<x_offset<<"yOffset"<<y_offset<<"max"<<max<<"x,y"<<x<<y<<std::endl;
        QPointF* draw_point = new QPointF[points.size()];
        for(int i=0;i<size;i++){
            draw_point[i].setX(((points.at(i).x() + x_offset)*x/max)+x/2+x_margin);
            draw_point[i].setY(((points.at(i).y() + y_offset)*y/max)+y/2+y_margin);
        }
        painter.drawPolygon(draw_point,size);
        delete[] draw_point;
    };

    //draw piece
    int piece_size = polygon.getPolygon().outer().size()-1;
    std::vector<QPointF> points;
    for(int i=0;i<piece_size;i++){
        points.push_back(QPointF(polygon.getPolygon().outer()[i].x(),polygon.getPolygon().outer()[i].y()));
    }
    painter.setBrush(QBrush(QColor("#0f5ca0")));
    drawPolygon(points);

}
