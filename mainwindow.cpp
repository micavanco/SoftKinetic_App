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
    ui->analyseBox->setVisible(false);


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
    connect(&projectionprocessor, SIGNAL(monitorValuex(QString)), ui->coordinatesLabel1x, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorValuey(QString)), ui->coordinatesLabel1y, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorDepthValue(QString)), ui->milimeterValueLabel_2, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorValue2x(QString)), ui->coordinatesLabel2x, SLOT(setText(QString)));
    connect(&projectionprocessor, SIGNAL(monitorValue2y(QString)), ui->coordinatesLabel2y, SLOT(setText(QString)));
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

    object1ArrayX = nullptr;
    object1ArrayY = nullptr;
    object2ArrayX = nullptr;
    object2ArrayY = nullptr;

    object1ArrayX2 = nullptr;
    object1ArrayY2 = nullptr;
    object2ArrayX2 = nullptr;
    object2ArrayY2 = nullptr;

    series1Object1 = nullptr;
    series1Object2 = nullptr;
    series2Object1 = nullptr;
    series2Object2 = nullptr;
    chart1 = nullptr;
    chart2 = nullptr;

    ui->analysisDock->setVisible(false);
    this->setGeometry(300,200,800,700);

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
        ui->cameraviewpushButton_8->setEnabled(true);
        ui->cameraviewDock->setMinimumSize(0,0);
        ui->cameraviewDock->setMaximumSize(16777215,16777215);
    }else
    {
        ui->cameraviewDock->setMinimumSize(570,570);
    }
}

void MainWindow::on_analysisDock_visibilityChanged(bool visible)
{
    if(ui->analysisDock->isFloating())
    {
        ui->cameraviewpushButton_8->setEnabled(true);
        ui->analysisDock->setMinimumSize(0,0);
        ui->analysisDock->setMaximumSize(16777215,16777215);
    }else
    {
        ui->analysisDock->setMinimumSize(570,570);
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
    ui->analyseBox->setVisible(false);
    ui->analysisDock->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->cameraviewDock->setWidget(ui->tabWidget);
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
    ui->inVideo->setVisible(true);
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
    ui->analyseBox->setVisible(false);
    ui->analysisDock->setVisible(false);
    ui->tabWidget->setVisible(true);
    ui->cameraviewDock->setWidget(ui->tabWidget);
}

void MainWindow::on_analyseButton_pressed()
{
    ui->settingsBox->setVisible(false);
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
    ui->recordBox->setVisible(false);
    ui->line->setVisible(false);
    ui->graphicsView->setVisible(false);
    ui->graphicsView_2->setVisible(false);
    ui->objectLabel->setVisible(false);
    ui->objectLabel2->setVisible(false);
    ui->analyseBox->setVisible(true);
    ui->analysisDock->setVisible(true);
    this->tabifyDockWidget(ui->cameraviewDock, ui->analysisDock);
    ui->tabWidget->setVisible(false);
    ui->cameraviewDock->setWidget(chart2);
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

void MainWindow::on_cameraviewpushButton_8_pressed()
{
    if(ui->cameraviewDock->isHidden())
    {
        ui->cameraviewDock->show();
    }
    if(ui->analysisDock->isHidden())
    {
        ui->analysisDock->show();
    }
    ui->cameraviewDock->setFloating(false);
    ui->analysisDock->setFloating(false);
    ui->cameraviewpushButton_8->setEnabled(false);
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
    *streamOut << QString("%1").arg((int)ui->timeSpinBox->value()*100)+"\n";
    *streamOut << "Time        Obiekt 1      Obiekt 2\n";
    *streamOut << "[s]    |   [x] : [y]  |  [x] : [y]\n";

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
    *streamOut << QString("%1").arg(totalTime-time)+"|"+ui->coordinatesLabel1x->text()+":"+
                  ui->coordinatesLabel1y->text()+"|"+ui->coordinatesLabel2x->text()+":"+
                  ui->coordinatesLabel2y->text()+"\n";
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


void MainWindow::on_inTimeRadio_pressed()
{
    ui->openFile2->setEnabled(false);
}

void MainWindow::on_inSpaceRadio_pressed()
{
    ui->openFile2->setEnabled(false);
}

void MainWindow::on_in2filesRadio_pressed()
{
    ui->openFile2->setEnabled(true);
}

void MainWindow::on_openFile1_pressed()
{
    if(object1ArrayX != nullptr)
    {
        delete object1ArrayX;
        delete object1ArrayY;
        delete object2ArrayX;
        delete object2ArrayY;
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Otwórz plik 1"), "", "Text File (*.txt)");

    if(filename.isEmpty())return;

    file = new QFile(filename);
    file->open(QIODevice::ReadOnly | QIODevice::Text);

    streamOut = new QTextStream(file);
    fileName1 = streamOut->readLine();
    ui->openFileLabel1->setText("Załadowano plik:\n"+fileName1+".txt");
    array1Length = streamOut->readLine().toInt();
    if(array1Length == 0)
    {
        ui->openFileLabel1->setText("Nieprawidłowy plik.\nSpróbuj ponownie.");
        return;
    }
    streamOut->readLine();
    streamOut->readLine();
    QString line;
    object1ArrayX = new double [array1Length+1];
    object1ArrayY = new double [array1Length+1];
    object2ArrayX = new double [array1Length+1];
    object2ArrayY = new double [array1Length+1];
    for(int i=0; i < array1Length+1; i++)
    {
        line = streamOut->readLine();
        QStringList list = line.split("|");
        QStringList obiekt1 = list[1].split(":");
        QStringList obiekt2 = list[2].split(":");
        object1ArrayX[i] = obiekt1[0].toDouble();
        object1ArrayY[i] = obiekt1[1].toDouble();
        object2ArrayX[i] = obiekt2[0].toDouble();
        object2ArrayY[i] = obiekt2[1].toDouble();
    }


    file->close();
    delete streamOut;
    delete file;

    if((ui->inTimeRadio->isChecked() || ui->inSpaceRadio->isChecked()) && ui->openFileLabel2->text() == "")
        ui->analyseProcessButton->setEnabled(true);
    else if(ui->in2filesRadio->isChecked() && ui->openFileLabel2->text() != "" )
        ui->analyseProcessButton->setEnabled(true);
    else
        ui->analyseProcessButton->setEnabled(false);
}

void MainWindow::on_openFile2_pressed()
{
    if(object1ArrayX2 != nullptr)
    {
        delete object1ArrayX2;
        delete object1ArrayY2;
        delete object2ArrayX2;
        delete object2ArrayY2;
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Otwórz plik 2"), "", "Text File (*.txt)");

    if(filename.isEmpty())return;

    file = new QFile(filename);
    file->open(QIODevice::ReadOnly | QIODevice::Text);

    streamOut = new QTextStream(file);
    fileName2 = streamOut->readLine();
    ui->openFileLabel2->setText("Załadowano plik:\n"+fileName2+".txt");
    array2Length = streamOut->readLine().toInt();
    if(array2Length == 0)
    {
        ui->openFileLabel2->setText("Nieprawidłowy plik.\nSpróbuj ponownie.");
        return;
    }
    streamOut->readLine();
    streamOut->readLine();
    QString line;
    object1ArrayX2 = new double [array2Length+1];
    object1ArrayY2 = new double [array2Length+1];
    object2ArrayX2 = new double [array2Length+1];
    object2ArrayY2 = new double [array2Length+1];
    for(int i=0; i < array2Length+1; i++)
    {
        line = streamOut->readLine();
        QStringList list = line.split("|");
        QStringList obiekt1 = list[1].split(":");
        QStringList obiekt2 = list[2].split(":");
        object1ArrayX2[i] = obiekt1[0].toDouble();
        object1ArrayY2[i] = obiekt1[1].toDouble();
        object2ArrayX2[i] = obiekt2[0].toDouble();
        object2ArrayY2[i] = obiekt2[1].toDouble();
    }


    file->close();
    delete streamOut;
    delete file;
    if(ui->openFileLabel1->text() != "")
        ui->analyseProcessButton->setEnabled(true);
    else
        ui->analyseProcessButton->setEnabled(false);
}

void MainWindow::on_analyseProcessButton_pressed()
{
    if(chart1 != nullptr || chart2 != nullptr)
    {
        delete series1Object1;
        delete series1Object2;
        delete series2Object1;
        delete series2Object2;
        delete chart1;
        delete chart2;
    }
    if(ui->inTimeRadio->isChecked())
    {

        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        for(int i=0; i < array1Length+1; i++)
            series1Object1->append(i,object1ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series1Object2->append(i,object2ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series2Object1->append(i,object1ArrayX[i]);

        for(int i=0; i < array1Length+1; i++)
            series2Object2->append(i,object2ArrayX[i]);

        chart1 = new Chart(series1Object1, series1Object2, "Wykres położenia od czasu", "Czas [ms]", "Położenie Y [cm]", array1Length, 480, false);
        chart2 = new Chart(series2Object1, series2Object2, "Wykres położenia od czasu", "Czas [ms]", "Położenie X [cm]", array1Length, 640, false);
        ui->analysisDock->setWidget(chart1);
        ui->cameraviewDock->setWidget(chart2);

    }else if(ui->inSpaceRadio->isChecked())
    {
        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        for(int i=0; i < array1Length+1; i++)
            series1Object1->append(object1ArrayX[i],object1ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series1Object2->append(object2ArrayX[i],object2ArrayY[i]);


        chart1 = new Chart(series1Object1, series1Object2, "Wykres położenia w przestrzeni", "Położenie X [cm]", "Położenie Y [cm]", 640, 480, false);
        chart2 = nullptr;
        ui->analysisDock->setWidget(chart1);
    }else
    {
        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        for(int i=0; i < array1Length+1; i++)
            series1Object1->append(object1ArrayX[i],object1ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series1Object2->append(object2ArrayX[i],object2ArrayY[i]);

        for(int i=0; i < array2Length+1; i++)
            series2Object1->append(object1ArrayX2[i],object1ArrayY2[i]);

        for(int i=0; i < array2Length+1; i++)
            series2Object2->append(object2ArrayX2[i],object2ArrayY2[i]);

        chart1 = new Chart(series1Object1, series1Object2, "Wykres położenia w przestrzeni", "Położenie X [cm]", "Położenie Y [cm]", 640, 480, true
                           , series2Object1, series2Object2, fileName1, fileName2 );
        chart2 = nullptr;
        ui->analysisDock->setWidget(chart1);
    }
}
