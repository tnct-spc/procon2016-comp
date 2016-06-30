#include "probmaker.h"
#include "voronoidiagrammaker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*
    ProbMaker w;
    w.show();
    w.run();
    */
    VoronoiDiagramMaker v;
    v.generateVoronoiDiagram();
    v.printVoronoiDiagram();
    return a.exec();
}
