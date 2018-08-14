#include "gui/displaywidget.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>

static int is_camera(const char* filename){
        FILE* f = fopen( filename, "r" );
        if (f == NULL) {
                return 0;
        }
        char buff[50] = {0};
        int ret = 0;
 
        if (fread(buff, 1, sizeof(buff)-1, f) <= 0) {
                fclose(f);
                return 0;
        }
        fclose( f );
        if (strstr(buff, "Camera") != NULL || strstr(buff, "UVC") != NULL || strstr(buff, "Webcam") != NULL) {
                return 1;
        }
        return 0;
}

DisplayWidget::DisplayWidget(QWidget *parent) : QWidget(parent)
{
    QStringList cameraOptions;

    int i;
    char filename[255];
    for (i=0; i<20; i++) {
        sprintf(filename, "/sys/class/video4linux/video%d/name", i);
        if (is_camera(filename) == 1) {
		cameraOptions << QString("%1").arg(i);
        }
    }
    if (cameraOptions.count()==0) {
        cameraOptions << "0" << "1" << "2" << "3" << "4" << "5" << "6";
    }

    QComboBox* cameraComboBox = new QComboBox;
    cameraComboBox->addItems(cameraOptions);

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    QPushButton *runButton = new QPushButton("run", this);
    QPushButton *fileSelector = new QPushButton("select file");
    horizontalLayout->addWidget(cameraComboBox);
    horizontalLayout->addWidget(fileSelector);
    horizontalLayout->addWidget(runButton);

    QVBoxLayout *layout = new QVBoxLayout;
    image_viewer_ = new ImageViewer(this);
    QRadioButton *sourceSelector = new QRadioButton("Stream from video camera_", 
		    				    this);

    sourceSelector->setDown(true);

    layout->addWidget(image_viewer_);
    layout->addLayout(horizontalLayout);
    layout->addWidget(sourceSelector);

    setLayout(layout);

    camera_ = new Camera();
    faceDector_ = new FaceDetector();

    //faceDector_->setProcessAll(false);

    faceDetectThread_.start();
    cameraThread_.start();

    camera_->moveToThread(&cameraThread_);
    faceDector_->moveToThread(&faceDetectThread_);

    // TODO: Add in slot to turn off camera_, or something
    image_viewer_->connect(faceDector_,
                           SIGNAL(image_signal(QImage)),
                           SLOT(set_image(QImage)));

    faceDector_->connect(camera_, SIGNAL(matReady(cv::Mat)), 
		         SLOT(processFrame(cv::Mat)));

    QObject::connect(runButton, SIGNAL(clicked()),
                     camera_, SLOT(runSlot()));

    QObject::connect(cameraComboBox, SIGNAL(currentIndexChanged(int)),
                     camera_, SLOT(cameraIndexSlot(int)));

    QObject::connect(fileSelector, SIGNAL(clicked()),
                     this,	SLOT(openFileDialog()));

    QObject::connect(sourceSelector, SIGNAL(toggled(bool)),
                     camera_, SLOT(usingVideoCameraSlot(bool)));

    QObject::connect(this, SIGNAL(videoFileNameSignal(QString)),
                     camera_, SLOT(videoFileNameSlot(QString)));

    faceDector_->connect(this, SIGNAL(facecascade_name_signal(QString)),
                     SLOT(facecascade_filename(QString)));
}

DisplayWidget::~DisplayWidget()
{
    faceDector_->~FaceDetector();
    camera_->~Camera();
    faceDetectThread_.quit();
    cameraThread_.quit();
    faceDetectThread_.wait();
    cameraThread_.wait();
}

void DisplayWidget::change_face_cascade_filename(QString filename)
{
    emit facecascade_name_signal(filename);
}

void DisplayWidget::openFileDialog()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Video"));
    emit videoFileNameSignal(filename);
}
