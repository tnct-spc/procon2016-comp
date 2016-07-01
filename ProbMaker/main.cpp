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
    v.generateVoronoiDiagram(50);
    v.printVoronoiDiagram();
    std::vector<polygon_t> pol  = v.cvToBoost();
    std::for_each(pol.begin(),pol.end(),[](polygon_t &a){std::cout << bg::dsv(a) << std::endl;});
    return a.exec();
}
