#include "hazama.h"
#include "ui_hazama.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QFileDialog>

#include "polygonexpansion.h"
#include "polygonset.h"
#include "field.h"
#include "polygonio.h"
#include "displayanswer.h"
#include "solver.h"

Hazama::Hazama(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Hazama)
{
    ui->setupUi(this);
    connect(ui->RunButton, &QPushButton::clicked, this, &Hazama::clickedRunButton);
}

Hazama::~Hazama()
{
    delete ui;
}

void Hazama::init()
{

}

void Hazama::run()
{
    std::cout << "Run" << std::endl;

    /*Get puzzle data*/
    procon::Field PDATA;
    cv::Mat src;
    if(ui->useWebCamera->isChecked() || ui->useScaner->isChecked()){
        //get Image
        if(ui->useWebCamera->isChecked()){
           src = capture();
        }else{
            int state = system("sh ./../../procon2016-comp/scanimage.sh");
            if(state ^= 0){
                std::cout << "failed. scanimage.sh error." << std::endl;
                return;
            }
            std::string path = "./../../procon2016-comp/CompImage/dpi300test.png";
            src = cv::imread(path);
        }
        //display picture
        cv::namedWindow("picture",CV_WINDOW_AUTOSIZE);
        cv::imshow("picture",src);

        /*Image Recognition*/
        //PDATA = ...
    }else if(ui->useFileData->isChecked()){
        std::string path = QFileDialog::getOpenFileName(this).toStdString();
        PDATA = procon::PolygonIO::importPolygon(path);
    }else{
        return;
    }
    /*Solve puzzle*/
    Solver solver;
    procon::Field field = solver.run(PDATA);

    /*Display answer*/
    Display.setField(field);

    std::cout<<"finish"<<std::endl;
}

cv::Mat Hazama::capture()
{
    cv::VideoCapture cap(0);//デバイスのオープン
    //cap.open(0);//こっちでも良い．

    if(!cap.isOpened()){
        std::cerr << "Can't open camera!" << std::endl;
    }

    cv::Mat src;

    cap >> src;

    return src;

    //===convert cv::Mat to QImage===//
    /*
    cv::Mat dst;
    cv::cvtColor(src, dst, CV_RGB2BGR);
    QImage tmp((uchar *)dst.data, dst.cols, dst.rows, dst.step, QImage::Format_RGB888);
    QImage viewImage = tmp.copy();
    */
    //===end===//
}

void Hazama::clickedRunButton()
{
    run();
}
