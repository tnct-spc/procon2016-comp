#include "probmaker.h"
#include "voronoidiagrammaker.h"
#include "fieldmaker.h"
#include "field.h"
#include "displayanswer.h"
#include <QApplication>
#include <boost/foreach.hpp>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*
    ProbMaker w;
    w.show();
    w.run();
    */
    VoronoiDiagramMaker v;
    v.generateVoronoiDiagram(50);
    std::vector<polygon_t> pol  = v.cvToBoost();
    FieldMaker f;
    f.makeField(pol,0);
    Field field = f.getField();
    auto hoge = field.pieceSize();
    std::cout << hoge << std::endl;
    DisplayAnswer d;
    d.setField(field);
    a.exec();
}
