#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MovementAnalyzer)
{
    ui->setupUi(this);

    connect(&processor,
                SIGNAL(inDisplay(QPixmap)),
                ui->inVideo,
                SLOT(setPixmap(QPixmap)));
    connect(&depthprocessor,
                SIGNAL(inDisplay(QPixmap)),
                ui->inVideo,
                SLOT(setPixmap(QPixmap)));

    connect(&processor, SIGNAL(CameraOn(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&processor, SIGNAL(CameraOff(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&depthprocessor, SIGNAL(CameraOn(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&depthprocessor, SIGNAL(CameraOff(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(this, SIGNAL(mouseOnScreen()), &depthprocessor, SLOT(onGettingDepth()));
    connect(this, SIGNAL(mouseOffScreen()), &depthprocessor, SLOT(offGettingDepth()));
    connect(&depthprocessor, SIGNAL(newValue(QString)), ui->milimeterValueLabel, SLOT(setText(QString)));

    ui->cameraSettingsBox->setVisible(false);
    ui->settingsBox->setVisible(false);
    ui->inVideo->installEventFilter(this);


    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->setDragMode(QGraphicsView::RubberBandDrag);

    connect(&projectionprocessor,
      SIGNAL(newFrame(QPixmap)),
      this,
      SLOT(onNewFrame(QPixmap)));

    connect(&projectionprocessor,
      SIGNAL(newFrame2(QPixmap)),
      this,
      SLOT(onNewFrame2(QPixmap)));

    connect(&projectionprocessor,
      SIGNAL(newFrame3(QPixmap)),
      ui->inVideo,
      SLOT(setPixmap(QPixmap)));

    connect(&projectionprocessor, SIGNAL(CameraOn(QString)), ui->camstatuslabel_2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(CameraOff(QString)), ui->camstatuslabel_2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorValue(QString)), ui->coordinatesLabel_2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorDepthValue(QString)), ui->milimeterValueLabel_2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorValue2(QString)), ui->coordinatesLabel_3, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorDepthValue2(QString)), ui->milimeterValueLabel_3, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(newTitle(QString)), ui->objectLabel, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(newTitle2(QString)), ui->objectLabel2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(checkIfRecord()), this, SLOT(onNewTitle()));


    connect(ui->graphicsView,
    SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),
    this,
    SLOT(onRubberBandChanged(QRect,QPointF,QPointF)));

    connect(ui->graphicsView_2,
    SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),
    this,
    SLOT(onRubberBandChanged2(QRect,QPointF,QPointF)));

    ui->graphicsView->fitInView(&pixmap,Qt::IgnoreAspectRatio);
    ui->graphicsView->scene()->addItem(&pixmap);

    ui->graphicsView_2->fitInView(&pixmap2,Qt::IgnoreAspectRatio);
    ui->graphicsView_2->scene()->addItem(&pixmap2);


}

MainWindow::~MainWindow()
{
    if(ui->testButton->isEnabled())
    {
        if(ui->normalradioButton->isChecked())
        {
            processor.requestInterruption();
            processor.wait();
        }
        else
        {
            depthprocessor.requestInterruption();
            depthprocessor.wait();
        }
    }else if(ui->recordButton->isEnabled() || ui->openpushButton_2->isEnabled())
    {
        projectionprocessor.requestInterruption();
        projectionprocessor.wait();
    }
    delete ui;
}

void MainWindow::on_openpushButton_pressed()
{
    if(ui->normalradioButton->isChecked())
        processor.start();
    else
    {
        DepthSense::FrameFormat frameFormat;
        DepthSense::DepthNode::CameraMode cameraMode;
        int colorType;
        if(ui->frameFormatComboBox->currentText() == "QQVGA (160x120)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_QQVGA;
        else if(ui->frameFormatComboBox->currentText() == "QCIF (176x144)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_QCIF;
        else if(ui->frameFormatComboBox->currentText() == "HQVGA (240x160)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_HQVGA;
        else if(ui->frameFormatComboBox->currentText() == "QVGA (320x240)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_QVGA;
        else if(ui->frameFormatComboBox->currentText() == "CIF (352x288)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_CIF;
        else if(ui->frameFormatComboBox->currentText() == "HVGA (480x320)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_HVGA;
        else if(ui->frameFormatComboBox->currentText() == "VGA (640x480)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_VGA;
        else if(ui->frameFormatComboBox->currentText() == "WXGA_H (1280x720)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_WXGA_H;
        else if(ui->frameFormatComboBox->currentText() == "DS311 (320x120)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_DS311;
        else if(ui->frameFormatComboBox->currentText() == "XGA (1024x768)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_XGA;
        else if(ui->frameFormatComboBox->currentText() == "SVGA (800x600)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_SVGA;
        else if(ui->frameFormatComboBox->currentText() == "OVVGA (636x480)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_OVVGA;
        else if(ui->frameFormatComboBox->currentText() == "WHVGA (640x240)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_WHVGA;
        else if(ui->frameFormatComboBox->currentText() == "nHD (640x360)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_NHD;
        else if(ui->frameFormatComboBox->currentText() == "StereoLR (320x480)")
            frameFormat=DepthSense::FrameFormat::FRAME_FORMAT_STEREOLR;

        if(ui->cameraModComboBox->currentText() == "Bliski Zasięg")
            cameraMode=DepthSense::DepthNode::CameraMode::CAMERA_MODE_CLOSE_MODE;
        else
            cameraMode=DepthSense::DepthNode::CameraMode::CAMERA_MODE_LONG_RANGE;

        if(ui->colorComboBox->currentText() == "Jesień")
            colorType=0;
        else if(ui->colorComboBox->currentText() == "Kość")
            colorType=1;
        else if(ui->colorComboBox->currentText() == "Odrzut Rakiety")
            colorType=2;
        else if(ui->colorComboBox->currentText() == "Zima")
            colorType=3;
        else if(ui->colorComboBox->currentText() == "Tęcza")
            colorType=4;
        else if(ui->colorComboBox->currentText() == "Ocean")
            colorType=5;
        else if(ui->colorComboBox->currentText() == "Lato")
            colorType=6;
        else if(ui->colorComboBox->currentText() == "Wiosna")
            colorType=7;
        else if(ui->colorComboBox->currentText() == "Chłód")
            colorType=8;
        else if(ui->colorComboBox->currentText() == "HSV")
            colorType=9;
        else if(ui->colorComboBox->currentText() == "Różowy")
            colorType=10;
        else if(ui->colorComboBox->currentText() == "Gorący")
            colorType=11;
        depthprocessor.setConfiguration(ui->fpsspinBox->value(), frameFormat, cameraMode, colorType);
        depthprocessor.start();
    }
    ui->normalradioButton->setEnabled(false);
    ui->depthradioButton->setEnabled(false);
    ui->openpushButton->setDisabled(true);
    ui->closepushButton_2->setEnabled(true);
    ui->recordButton->setEnabled(false);
    ui->analyseButton->setEnabled(false);
}

void MainWindow::on_closepushButton_2_pressed()
{
    if(ui->normalradioButton->isChecked())
    {
        processor.requestInterruption();
        processor.wait();
    }
    else
    {
        depthprocessor.requestInterruption();
        depthprocessor.wait();
    }

    ui->normalradioButton->setEnabled(true);
    ui->depthradioButton->setEnabled(true);
    ui->openpushButton->setDisabled(false);
    ui->closepushButton_2->setEnabled(false);
    ui->recordButton->setEnabled(true);
    ui->analyseButton->setEnabled(true);
}

void MainWindow::on_cameraviewpushButton_pressed()
{
    if(ui->cameraviewDock->isHidden())
        ui->cameraviewDock->show();
    ui->cameraviewDock->setFloating(false);
    ui->cameraviewpushButton->setEnabled(false);
}


void MainWindow::on_cameraviewDock_visibilityChanged(bool visible)
{
    if(!visible || ui->cameraviewDock->isFloating())
    {
        ui->cameraviewpushButton->setEnabled(true);
        ui->cameraviewpushButton_2->setEnabled(true);
        ui->cameraviewDock->setMinimumSize(0,0);
        ui->cameraviewDock->setMaximumSize(16777215,16777215);
    }else
    {
        ui->cameraviewDock->setMinimumSize(570,570);
    }
}




void MainWindow::on_normalradioButton_pressed()
{
    if(!ui->normalradioButton->isChecked())
        ui->normalradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(false);
}

void MainWindow::on_normalradioButton_clicked(bool checked)
{
    if(!checked)
        ui->normalradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(false);
}

void MainWindow::on_depthradioButton_clicked(bool checked)
{
    if(!checked)
        ui->depthradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(true);
}

void MainWindow::on_depthradioButton_pressed()
{
    if(!ui->depthradioButton->isChecked())
        ui->depthradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(true);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QMouseEvent *mouseEvent = reinterpret_cast<QMouseEvent*>(event);
    if(qobject_cast<QLabel*>(obj)==ui->inVideo && mouseEvent->type() == QMouseEvent::MouseMove)
    {
        mouseOnScreen();
        int x=mouseEvent->pos().x()*320.0/(double)ui->inVideo->width();
        int y=mouseEvent->pos().y()*240.0/(double)ui->inVideo->height();
        depthprocessor.setDepthValue(x,y);
        ui->coordinatesLabel->setText(QString("%1, %2").arg(x).arg(y));

    }else if(qobject_cast<QLabel*>(obj)==ui->inVideo && mouseEvent->type() == QMouseEvent::MouseButtonPress){


            int x=mouseEvent->pos().x()*320.0/(double)ui->inVideo->width();
            int y=mouseEvent->pos().y()*240.0/(double)ui->inVideo->height();

            depthprocessor.setDepthValue(x,y);

            ui->coordinatesLabel->setText(QString("%1, %2").arg(x).arg(y));
            mouseOffScreen();
    }
    return false;
}


void MainWindow::on_testButton_pressed()
{
    ui->settingsBox->setVisible(true);
    ui->recordBox->setVisible(false);
    ui->line->setVisible(false);
    ui->graphicsView->setVisible(false);
    ui->graphicsView_2->setVisible(false);
    ui->objectLabel->setVisible(false);
    ui->objectLabel2->setVisible(false);
    ui->cameraviewpushButton->setEnabled(false);
    ui->cameraviewpushButton_2->setEnabled(false);
    ui->inVideo->setVisible(true);
    ui->redSlider1->setVisible(false);
    ui->redColorLabel1->setVisible(false);
    ui->redSlider2->setVisible(false);
    ui->redColorLabel2->setVisible(false);
    ui->redSlider3->setVisible(false);
    ui->redColorLabel3->setVisible(false);
    ui->blueSlider1->setVisible(false);
    ui->blueColorLabel1->setVisible(false);
    ui->blueSlider2->setVisible(false);
    ui->blueColorLabel2->setVisible(false);
    ui->blueSlider3->setVisible(false);
    ui->blueColorLabel3->setVisible(false);
}

void MainWindow::on_recordButton_pressed()
{
    ui->settingsBox->setVisible(false);
    ui->recordBox->setVisible(true);
    ui->line->setVisible(true);
    ui->graphicsView->setVisible(true);
    ui->graphicsView_2->setVisible(true);
    ui->objectLabel->setVisible(true);
    ui->objectLabel2->setVisible(true);
    ui->cameraSettingsBox->setVisible(false);
    ui->cameraviewpushButton->setEnabled(false);
    ui->cameraviewpushButton_2->setEnabled(false);
    ui->inVideo->setVisible(false);
    ui->redSlider1->setVisible(true);
    ui->redColorLabel1->setVisible(true);
    ui->redSlider2->setVisible(true);
    ui->redColorLabel2->setVisible(true);
    ui->redSlider3->setVisible(true);
    ui->redColorLabel3->setVisible(true);
    ui->blueSlider1->setVisible(true);
    ui->blueColorLabel1->setVisible(true);
    ui->blueSlider2->setVisible(true);
    ui->blueColorLabel2->setVisible(true);
    ui->blueSlider3->setVisible(true);
    ui->blueColorLabel3->setVisible(true);
}

void MainWindow::on_openpushButton_2_pressed()
{
    ui->recordmoveButton->setEnabled(false);
    ui->closepushButton_3->setEnabled(true);
    ui->openpushButton_2->setEnabled(false);
    ui->testButton->setEnabled(false);
    ui->analyseButton->setEnabled(false);
    ui->objectLabel->setText(QString("Zaznacz obiekt 1..."));
    ui->objectLabel2->setText(QString("Zaznacz obiekt 2..."));
    projectionprocessor.start();

}

void MainWindow::on_closepushButton_3_pressed()
{
    ui->recordmoveButton->setEnabled(false);
    ui->testButton->setEnabled(true);
    ui->analyseButton->setEnabled(true);
    ui->closepushButton_3->setEnabled(false);
    ui->openpushButton_2->setEnabled(true);
    ui->objectLabel->setText(QString(""));
    ui->objectLabel2->setText(QString(""));

    projectionprocessor.requestInterruption();
    projectionprocessor.wait();
}

void MainWindow::on_cameraviewpushButton_2_pressed()
{
    if(ui->cameraviewDock->isHidden())
        ui->cameraviewDock->show();
    ui->cameraviewDock->setFloating(false);
    ui->cameraviewpushButton_2->setEnabled(false);
}


void MainWindow::onRubberBandChanged(QRect rect,
  QPointF frScn,
  QPointF toScn)
  {
    projectionprocessor.setTrackRect(rect);
  }

void MainWindow::onRubberBandChanged2(QRect rect,
  QPointF frScn,
  QPointF toScn)
  {
    projectionprocessor.setTrackRect2(rect);
  }

void MainWindow::onNewFrame(QPixmap newFrm)
{
    newFrm.scaled(640,480, Qt::IgnoreAspectRatio);
    pixmap.setPixmap(newFrm);
    ui->graphicsView->fitInView(&pixmap,Qt::IgnoreAspectRatio);
}

void MainWindow::onNewFrame2(QPixmap newFrm)
{
    newFrm.scaled(640,480, Qt::IgnoreAspectRatio);
    pixmap2.setPixmap(newFrm);
    ui->graphicsView_2->fitInView(&pixmap2,Qt::IgnoreAspectRatio);
}

void MainWindow::onNewTitle()
{
    if(ui->objectLabel->text()[0]=="O" && ui->objectLabel2->text()[0]=="O")
        ui->recordmoveButton->setEnabled(true);
}


void MainWindow::on_recordmoveButton_pressed()
{
    fileTitle = QTime::currentTime().toString("hh_mm_ss");
    file = new QFile("../MovementAnalyzer/output/"+fileTitle+".txt");
    file->open(QIODevice::ReadWrite | QIODevice::Text);

    streamOut = new QTextStream(file);
    *streamOut << QTime::currentTime().toString("hh:mm:ss")+"\n";
    *streamOut << "Time [s]    Obiekt 1       Obiekt 2\n";

    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    time = ui->timeSpinBox->value();
    totalTime = time;
    ui->timeLabel->setText(QString("%1").arg(time));
    timer->start(10);
    ui->timeSpinBox->setEnabled(false);
    ui->closepushButton_3->setEnabled(false);
    ui->recordmoveButton->setEnabled(false);
}

void MainWindow::onTimerTimeout()
{
    *streamOut << QString("%1").arg(totalTime-time)+"  -  "+ui->coordinatesLabel_2->text()+"  |  "+ui->coordinatesLabel_2->text()+"\n";
    time -= 0.01;
    ui->timeLabel->setText(QString("%1").arg(time));
    if(time <= 0.0)
    {
        timer->stop();
        file->flush();
        file->close();
        delete file;
        delete streamOut;
        ui->timeSpinBox->setEnabled(true);
        ui->closepushButton_3->setEnabled(true);
        ui->recordmoveButton->setEnabled(true);
        ui->timeLabel->setText("Zapisano do "+fileTitle+".txt");
    }
}

void MainWindow::on_redSlider1_valueChanged(int value)
{
    ui->redColorLabel1->setText(QString("%1").arg(value));
    projectionprocessor.setHueRed(value);
}

void MainWindow::on_redSlider2_valueChanged(int value)
{
    ui->redColorLabel2->setText(QString("%1").arg(value));
    projectionprocessor.setSaturationRed(value);
}

void MainWindow::on_redSlider3_valueChanged(int value)
{
    ui->redColorLabel3->setText(QString("%1").arg(value));
    projectionprocessor.setValueRed(value);
}

void MainWindow::on_blueSlider1_valueChanged(int value)
{
    ui->blueColorLabel1->setText(QString("%1").arg(value));
    projectionprocessor.setValueBlue(value);
}

void MainWindow::on_blueSlider2_valueChanged(int value)
{
    ui->blueColorLabel2->setText(QString("%1").arg(value));
    projectionprocessor.setSaturationBlue(value);
}

void MainWindow::on_blueSlider3_valueChanged(int value)
{
    ui->blueColorLabel3->setText(QString("%1").arg(value));
    projectionprocessor.setHueBlue(value);
}
