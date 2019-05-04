#include "gui/displaywidget.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev.h>
#include <QMessageBox>
#include <QApplication>

DisplayWidget::DisplayWidget(QWidget *parent) : QWidget(parent)
{
    QStringList cameraOptions;

    QStringList previewDevices;
    if (QDir("/sys/class/video4linux/v4l-subdev2/device/video4linux/video1").exists() 
        || QDir("/sys/class/video4linux/v4l-subdev5/device/video4linux/video1").exists()) {
        previewDevices.append("/dev/video1");
    }
    if (QDir("/sys/class/video4linux/v4l-subdev2/device/video4linux/video5").exists() 
        || QDir("/sys/class/video4linux/v4l-subdev5/device/video4linux/video5").exists()) {
        previewDevices.append("/dev/video5");
    }

    cameraOptions << QString("rkisp device=%1 io-mode=1 ! video/x-raw,format=NV12,width=640,height=480,framerate=30/1 ! videoconvert ! appsink")
        .arg(previewDevices.at(0));

    QComboBox* cameraComboBox = new QComboBox;
    cameraComboBox->addItems(cameraOptions);

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    QPushButton *runButton = new QPushButton("run", this);
    horizontalLayout->addWidget(cameraComboBox);
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

    QObject::connect(sourceSelector, SIGNAL(toggled(bool)),
                     camera_, SLOT(usingVideoCameraSlot(bool)));

    faceDector_->connect(this, SIGNAL(facecascade_name_signal(QString)),
                     SLOT(facecascade_filename(QString)));

    if (previewDevices.count() == 0) {
        QMessageBox::critical(this,
                                "Video Error",
                                "Make sure you have a mipi camera connected and entered correct camera parameter!");
    }
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
