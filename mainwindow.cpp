#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MovementAnalyzer)
{
    ui->setupUi(this);// przypisanie graficznego interfejsu uzytkownika do tej klasy

    // połączenie sygnałów przesyłąjących obraz z klas do elementów GUI
    connect(&processor,
                SIGNAL(inDisplay(QPixmap)),
                ui->inVideo,
                SLOT(setPixmap(QPixmap)));
    connect(&depthprocessor,
                SIGNAL(inDisplay(QPixmap)),
                ui->inVideo,
                SLOT(setPixmap(QPixmap)));
    // połączenie sygnałów informujących o: Wł/Wył kamery, położeniu kursora oraz wartości odległości od punktu
    connect(&processor, SIGNAL(CameraOn(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&processor, SIGNAL(CameraOff(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&depthprocessor, SIGNAL(CameraOn(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(&depthprocessor, SIGNAL(CameraOff(QString)), ui->camstatuslabel, SLOT(setText(QString)));
    connect(this, SIGNAL(mouseOnScreen()), &depthprocessor, SLOT(onGettingDepth()));
    connect(this, SIGNAL(mouseOffScreen()), &depthprocessor, SLOT(offGettingDepth()));
    connect(&depthprocessor, SIGNAL(newValue(QString)), ui->milimeterValueLabel, SLOT(setText(QString)));

    // ukrywanie elementów GUI oraz instalacja filtra zdarzeń do okna z obrazem z kamery
    ui->cameraSettingsBox->setVisible(false);
    ui->settingsBox->setVisible(false);
    ui->inVideo->installEventFilter(this);
    ui->analyseBox->setVisible(false);
    ui->additionalOptionsBox->setEnabled(false);

    // utworzenie nowego obiektu sceny i przypisanie go do okna z obrazem z kamery oraz ustawienie możliwości przeciągania i zaznaczania
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->setDragMode(QGraphicsView::RubberBandDrag);

    // połączenie sygnałów z obrazem z kamery do slotów wyświetlających w GUI
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
    // połączenie sygnałów informujących o: Wł/Wył kamery, położeniu kursora, wartości odległości od punktu oraz czy zostało włączone nagrywanie
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

    // połaczenie sygnału o przeciąganiu wcisniętego przycisku myszy po oknie z obrazem z kamery
    connect(ui->graphicsView,
    SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),
    this,
    SLOT(onRubberBandChanged(QRect,QPointF,QPointF)));

    connect(ui->graphicsView_2,
    SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),
    this,
    SLOT(onRubberBandChanged2(QRect,QPointF,QPointF)));
    // ustawienie trybu dopasowania obrazu do okna zawierającego oraz dodanie mapy bitowej do sceny
    ui->graphicsView->fitInView(&pixmap,Qt::IgnoreAspectRatio);
    ui->graphicsView->scene()->addItem(&pixmap);

    ui->graphicsView_2->fitInView(&pixmap2,Qt::IgnoreAspectRatio);
    ui->graphicsView_2->scene()->addItem(&pixmap2);
    // przypisanie do wskaźników wartości nullptr aby nie wskazywały na niezaalokowany obszar
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
    m_fftChart = nullptr;
    // ukrycie okna z wykresami
    ui->analysisDock->setVisible(false);
    this->setGeometry(300,200,800,700); // ustawienie rozmiaru okna całej aplikacji

}
// destruktor wywołujący prośbę zakończenie równoległych watków i odczekanie na ich zakończenie
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
    delete ui;// usunięcie zaimplementowanego graficznego interfejsu użytkownika
}
// metoda zdarzenia naciśnięcia przycisku włączenia kamery w trybie testowania
void MainWindow::on_openpushButton_pressed()
{
    // jeżeli ustawiony jest tryb pracy normalnej to uruchom wątek obsługujący transmisję danych z kamery
    if(ui->normalradioButton->isChecked())
        processor.start();
    else
    {   // sprawdzenie ustawionych parametrów pracy kamery takich jak kolejno: rozdzielczość, zasięg oraz mapa kolorów
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
        // wywołanie metody ustawiającej parametry kamery w trybie głębi wraz zczytanymi wczesniej parametrami
        depthprocessor.setConfiguration(ui->fpsspinBox->value(), frameFormat, cameraMode, colorType);
        // wywołanie wątku do transmisji obrazu
        depthprocessor.start();
    }
    // ustawienie przycisków na nieaktywne oraz przycisku do zatrzymania transmisji na aktywny
    ui->normalradioButton->setEnabled(false);
    ui->depthradioButton->setEnabled(false);
    ui->openpushButton->setDisabled(true);
    ui->closepushButton_2->setEnabled(true);
    ui->recordButton->setEnabled(false);
    ui->analyseButton->setEnabled(false);
}
// metoda obsługujaca zdarzenia nacisnięcia przycisku do zatrzymania transmisji obrazu z kamery
void MainWindow::on_closepushButton_2_pressed()
{// wywołanie prośby zakończenie równoległych watków i odczekanie na ich zakończenie
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
    // ustawienie przycisków na aktywne oraz przycisku do zatrzymania transmisji na nieaktywny
    ui->normalradioButton->setEnabled(true);
    ui->depthradioButton->setEnabled(true);
    ui->openpushButton->setDisabled(false);
    ui->closepushButton_2->setEnabled(false);
    ui->recordButton->setEnabled(true);
    ui->analyseButton->setEnabled(true);
}
// metoda obsługujaca zdarzenia nacisnięcia przycisku przywracającego okno z obrazem z kamery do okna głównego programu
void MainWindow::on_cameraviewpushButton_pressed()
{
    // sprawdzenie czy okno z obrazem z kamery zostało zamknięte
    if(ui->cameraviewDock->isHidden())
        ui->cameraviewDock->show();
    // ustawienie klawisza jako niekatywny oraz przywrócenie okna
    ui->cameraviewDock->setFloating(false);
    ui->cameraviewpushButton->setEnabled(false);
}

// metoda obsługujaca zdarzenia zmiany widoczności okna z obrazem z kamery
void MainWindow::on_cameraviewDock_visibilityChanged(bool visible)
{// jeżeli okno nie jest widoczne lub jest poza głównym oknem aplikacji to ustaw mozliwość dowolnej zmiany jego rozmiaru
    // oraz ustaw przyciski przywrócenia jako aktywne. Jeżeli nie to ustaw pierwotną wartość minimalną rozmiaru
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
// metoda obsługujaca zdarzenia zmiany widoczności okna z wykresami
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

// metoda obsługujaca zdarzenia naciśnięcia przycisku opcji pracy normalnej kamery
void MainWindow::on_normalradioButton_pressed()
{
    if(!ui->normalradioButton->isChecked())
        ui->normalradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(false);
}
// metoda obsługujaca zdarzenia kliknięcia przycisku opcji pracy normalnej kamery
void MainWindow::on_normalradioButton_clicked(bool checked)
{
    if(!checked)
        ui->normalradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(false);
}
// metoda obsługujaca zdarzenia kliknięcia przycisku opcji pracy głębi kamery
void MainWindow::on_depthradioButton_clicked(bool checked)
{
    if(!checked)
        ui->depthradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(true);
}
// metoda obsługujaca zdarzenia naciśnięcia przycisku opcji pracy głębi kamery
void MainWindow::on_depthradioButton_pressed()
{
    if(!ui->depthradioButton->isChecked())
        ui->depthradioButton->setChecked(true);
    ui->cameraSettingsBox->setVisible(true);
}
// metoda przechwytująca wszystkie zdarzenia występujace w głównym oknie aplikacji
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QMouseEvent *mouseEvent = reinterpret_cast<QMouseEvent*>(event);// rzutowanie obiektu zdarzenia na zdarzenia zwiazane z myszą
    // jeżeli zdarzenie dotyczyło okna z obrazem z kamery oraz zdarzenia poruszania myszą
    if(qobject_cast<QLabel*>(obj)==ui->inVideo && mouseEvent->type() == QMouseEvent::MouseMove)
    {
        mouseOnScreen();
        // odczytaj położenie kursora oraz przekaż do klasy przechwytującej obraz z kamery
        int x=mouseEvent->pos().x()*320.0/(double)ui->inVideo->width();
        int y=mouseEvent->pos().y()*240.0/(double)ui->inVideo->height();
        depthprocessor.setDepthValue(x,y);
        ui->coordinatesLabel->setText(QString("%1, %2").arg(x).arg(y));
    // jeżeli zdarzenie dotyczyło okna z obrazem z kamery oraz zdarzenia nacisniecia przycisku myszy
    }else if(qobject_cast<QLabel*>(obj)==ui->inVideo && mouseEvent->type() == QMouseEvent::MouseButtonPress){


            int x=mouseEvent->pos().x()*320.0/(double)ui->inVideo->width();
            int y=mouseEvent->pos().y()*240.0/(double)ui->inVideo->height();

            depthprocessor.setDepthValue(x,y);

            ui->coordinatesLabel->setText(QString("%1, %2").arg(x).arg(y));
            mouseOffScreen();
    }
    return false;
}

// metoda obsługujaca zdarzenia nacisnięcia przycisku wyświetlającego okna i panele związane z testowanie kamery
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
    ui->tabWidget->removeTab(1);
    ui->tabWidget->removeTab(0);
}
// metoda obsługujaca zdarzenia nacisnięcia przycisku wyświetlającego okna i panele związane z rejestracją obiektów
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
    ui->tabWidget->insertTab(0, ui->tab_23, "Obiekt 1");
    ui->tabWidget->insertTab(1, ui->tab_24, "Obiekt 2");
}
// metoda obsługujaca zdarzenia nacisnięcia przycisku wyświetlającego okna i panele związane z rysowaniem wykresów
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
// metoda zdarzenia naciśnięcia przycisku włączenia kamery w trybie rejestracji obiektów
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
// metoda zdarzenia naciśnięcia przycisku wyłączenia kamery w trybie rejestracji obiektów
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
// metoda zdarzenia naciśnięcia przycisku przywrócenia okna z obrazem z kamery do głównego okna aplikacji
void MainWindow::on_cameraviewpushButton_2_pressed()
{
    if(ui->cameraviewDock->isHidden())
        ui->cameraviewDock->show();
    ui->cameraviewDock->setFloating(false);
    ui->cameraviewpushButton_2->setEnabled(false);
}
// metoda zdarzenia naciśnięcia przycisku przywrócenia okna z wykresami do głównego okna aplikacji
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

// metoda zdarzenia przeciągania wcisniętego przycisku myszy po oknie z obrazem z kamery
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
// slot przyjmujący mapę bitową, będącą obrazem z kamery oraz wyświetlająca ten obraz w GUI
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
// slot sprawdzający czy użytkownik zaznaczył obiekt na dwóch obrazach, jeżeli tak to aktywuje przycisk do rejestracji położenia tych obiektów
void MainWindow::onNewTitle()
{
    if(ui->objectLabel->text()[0]=="O" && ui->objectLabel2->text()[0]=="O")
        ui->recordmoveButton->setEnabled(true);
}

// metoda zdarzenia naciśnięcia przycisku rejestracji położenia obiektów
void MainWindow::on_recordmoveButton_pressed()
{
    // pobierz aktualny czas oraz utwórz nowy plik tekstowy z datą w nazwie
    fileTitle = QTime::currentTime().toString("hh_mm_ss");
    file = new QFile("output/"+fileTitle+".txt");
    file->open(QIODevice::ReadWrite | QIODevice::Text);
    // utwórz obiekt do transmisji danych do pliku oraz prześlij dwa wiersze
    streamOut = new QTextStream(file);
    *streamOut << QTime::currentTime().toString("hh:mm:ss")+"\n";
    *streamOut << QString("%1").arg((int)ui->timeSpinBox->value()*10)+"\n";
    *streamOut << "Time        Obiekt 1      Obiekt 2\n";
    *streamOut << "[s]    |   [x] : [y]  |  [x] : [y]\n";
    // utwórz nowy obiekt zliczający czas oraz połącz go z metodą wywoływaną po każdym upływie 100 ms
    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    // połączenie sygnału osiągnięcia maksymalnego czasu przez licznik z metodą
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    time = ui->timeSpinBox->value();
    totalTime = time;
    ui->timeLabel->setText(QString("%1").arg(time));
    // zacznij zliczać czas
    timer->start(100);
    ui->timeSpinBox->setEnabled(false);
    ui->closepushButton_3->setEnabled(false);
    ui->recordmoveButton->setEnabled(false);
    timeCount = time*10+1;
}
// metoda wywolywana co 100 ms czasu ustawionego na liczniku QTimer
void MainWindow::onTimerTimeout()
{
    // zapisz do pliku aktualne położenie obiektów
    *streamOut << QString("%1").arg(totalTime-time)+"|"+ui->coordinatesLabel1x->text()+":"+
                  ui->coordinatesLabel1y->text()+"|"+ui->coordinatesLabel2x->text()+":"+
                  ui->coordinatesLabel2y->text()+"\n";
    time -= 0.1;
    timeCount--;
    ui->timeLabel->setText(QString("%1").arg(time));
    if(timeCount == 0)// jeżeli zliczany czas wynosi zero to zatrzymaj licznik i transmisję danych do pliku oraz usuń związane z tym obiekty
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
// metoda zdarzenia zmiany położenia suwaków w oknie z obrazem z kamery w trybie rejestracji obiektów
// wartość okreslająca nasycenie Hue obiektu 1
void MainWindow::on_redSlider1_valueChanged(int value)
{
    ui->redColorLabel1->setText(QString("%1").arg(value));
    projectionprocessor.setHueRed(value);
}
// wartość okreslająca natężenie Saturation obiektu 1
void MainWindow::on_redSlider2_valueChanged(int value)
{
    ui->redColorLabel2->setText(QString("%1").arg(value));
    projectionprocessor.setSaturationRed(value);
}
// wartość okreslająca jasność Value obiektu 1
void MainWindow::on_redSlider3_valueChanged(int value)
{
    ui->redColorLabel3->setText(QString("%1").arg(value));
    projectionprocessor.setValueRed(value);
}
// wartość okreslająca nasycenie Hue obiektu 2
void MainWindow::on_blueSlider1_valueChanged(int value)
{
    ui->blueColorLabel1->setText(QString("%1").arg(value));
    projectionprocessor.setValueBlue(value);
}
// wartość okreslająca natężenie Saturation obiektu 2
void MainWindow::on_blueSlider2_valueChanged(int value)
{
    ui->blueColorLabel2->setText(QString("%1").arg(value));
    projectionprocessor.setSaturationBlue(value);
}
// wartość okreslająca jasność Value obiektu 2
void MainWindow::on_blueSlider3_valueChanged(int value)
{
    ui->blueColorLabel3->setText(QString("%1").arg(value));
    projectionprocessor.setHueBlue(value);
}

// metoda obsługujaca zdarzenia naciśnięcia przycisku opcji trybu analizy przebiegów
// opcja analizy położenia od czasu
void MainWindow::on_inTimeRadio_pressed()
{
    ui->openFile2->setEnabled(false);
    ui->additionalOptionsBox->setEnabled(false);
}
// opcja analizy przestrzeni
void MainWindow::on_inSpaceRadio_pressed()
{
    ui->openFile2->setEnabled(false);
    ui->additionalOptionsBox->setEnabled(false);
}
// opcja analizy dwóch plików
void MainWindow::on_in2filesRadio_pressed()
{
    ui->openFile2->setEnabled(true);
    ui->additionalOptionsBox->setEnabled(true);
    if(ui->openFileLabel2->text() == "")
        ui->analyseProcessButton->setEnabled(false);
}
// metoda obsługujaca zdarzenia naciśnięcia przycisku wczytania pliku pierwszego
void MainWindow::on_openFile1_pressed()
{

    // wywolanie okna dialogowego do wyboru pliku i przypisanie jego nazwy do zmiennej
    QString filename = QFileDialog::getOpenFileName(this,
    tr("Otwórz plik 1"), "", "Text File (*.txt)");

    if(filename.isEmpty())return;// jeżeli nazwwa jest pusta to zakończ wczytywanie pliku
    // jeżeli były już wcześniej wczytane jakieś dane to je usuń
    if(object1ArrayX != nullptr)
    {
        delete [] object1ArrayX;
        delete [] object1ArrayY;
        delete [] object2ArrayX;
        delete [] object2ArrayY;
    }
    // otworzenie pliku
    file = new QFile(filename);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    // rozpoczęcie transmisji danych z pliku
    streamOut = new QTextStream(file);
    fileName1 = streamOut->readLine();
    ui->openFileLabel1->setText("Załadowano plik:\n"+fileName1+".txt");// ustawienie informacji w GUI o załadowanym pliku
    array1Length = streamOut->readLine().toInt();
    if(array1Length == 0)// jeżeli pierwszy wiersz nie zawiera liczby, bądź tą liczbą jest zero to zakończ wczytywanie pliku
    {
        ui->openFileLabel1->setText("Nieprawidłowy plik.\nSpróbuj ponownie.");
        return;
    }
    // wczytaj kolejne dwa wiersze
    streamOut->readLine();
    streamOut->readLine();
    QString line;
    // utworzenie dynamiczne tablic
    object1ArrayX = new double [array1Length+1];
    object1ArrayY = new double [array1Length+1];
    object2ArrayX = new double [array1Length+1];
    object2ArrayY = new double [array1Length+1];
    for(int i=0; i < array1Length+1; i++)
    {
        // wczytanie całej linii wiersza oraz odseparowania danych od siebie i zapis do tablicy
        line = streamOut->readLine();
        QStringList list = line.split("|");
        QStringList obiekt1 = list[1].split(":");
        QStringList obiekt2 = list[2].split(":");
        object1ArrayX[i] = obiekt1[0].toDouble();
        object1ArrayY[i] = obiekt1[1].toDouble();
        object2ArrayX[i] = obiekt2[0].toDouble();
        object2ArrayY[i] = obiekt2[1].toDouble();
    }

    // zamknięcie transmisji danych z pliku oraz usunięcie obiektów z tym związanych
    file->close();
    delete streamOut;
    delete file;
    // sprawdzenie które przyciski opcji są zaznaczone, czy można aktywować przycisk do generowania wykresów
    if(ui->inTimeRadio->isChecked() || ui->inSpaceRadio->isChecked())
        ui->analyseProcessButton->setEnabled(true);
    else if(ui->in2filesRadio->isChecked() && ui->openFileLabel2->text() != "" )
        ui->analyseProcessButton->setEnabled(true);
    else
        ui->analyseProcessButton->setEnabled(false);
}
// metoda obsługujaca zdarzenia naciśnięcia przycisku wczytania pliku drugiego
void MainWindow::on_openFile2_pressed()
{

    // wywolanie okna dialogowego do wyboru pliku i przypisanie jego nazwy do zmiennej
    QString filename = QFileDialog::getOpenFileName(this, tr("Otwórz plik 2"), "", "Text File (*.txt)");

    if(filename.isEmpty())return;// jeżeli nazwwa jest pusta to zakończ wczytywanie pliku
    // jeżeli były już wcześniej wczytane jakieś dane to je usuń
    if(object1ArrayX2 != nullptr)
    {
        delete [] object1ArrayX2;
        delete [] object1ArrayY2;
        delete [] object2ArrayX2;
        delete [] object2ArrayY2;
    }
    // otworzenie pliku
    file = new QFile(filename);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    // rozpoczęcie transmisji danych z pliku
    streamOut = new QTextStream(file);
    fileName2 = streamOut->readLine();
    ui->openFileLabel2->setText("Załadowano plik:\n"+fileName2+".txt");// ustawienie informacji w GUI o załadowanym pliku
    array2Length = streamOut->readLine().toInt();
    if(array2Length == 0)// jeżeli pierwszy wiersz nie zawiera liczby, bądź tą liczbą jest zero to zakończ wczytywanie pliku
    {
        ui->openFileLabel2->setText("Nieprawidłowy plik.\nSpróbuj ponownie.");
        return;
    }
    // wczytaj kolejne dwa wiersze
    streamOut->readLine();
    streamOut->readLine();
    QString line;
    // utworzenie dynamiczne tablic
    object1ArrayX2 = new double [array2Length+1];
    object1ArrayY2 = new double [array2Length+1];
    object2ArrayX2 = new double [array2Length+1];
    object2ArrayY2 = new double [array2Length+1];
    for(int i=0; i < array2Length+1; i++)
    {
        // wczytanie całej linii wiersza oraz odseparowania danych od siebie i zapis do tablicy
        line = streamOut->readLine();
        QStringList list = line.split("|");
        QStringList obiekt1 = list[1].split(":");
        QStringList obiekt2 = list[2].split(":");
        object1ArrayX2[i] = obiekt1[0].toDouble();
        object1ArrayY2[i] = obiekt1[1].toDouble();
        object2ArrayX2[i] = obiekt2[0].toDouble();
        object2ArrayY2[i] = obiekt2[1].toDouble();
    }

    // zamknięcie transmisji danych z pliku oraz usunięcie obiektów z tym związanych
    file->close();
    delete streamOut;
    delete file;
    // sprawdzenie czy plik numer 1 został wczytane, jeżeli tak to aktywuj przycisk generowania wykresów
    if(ui->openFileLabel1->text() != "")
        ui->analyseProcessButton->setEnabled(true);
    else
        ui->analyseProcessButton->setEnabled(false);
}
// metoda obsługujaca zdarzenia naciśnięcia przycisku generowania wykresów
void MainWindow::on_analyseProcessButton_pressed()
{
    // jeżeli były już wcześniej generowane wykresy to je usuń oraz usuń ich serie danych
    if(chart1 != nullptr || chart2 != nullptr)
    {
        delete series1Object1;
        delete series1Object2;
        delete series2Object1;
        delete series2Object2;
        delete chart1;
        delete chart2;
        delete m_fftChart;
    }
    if(ui->inTimeRadio->isChecked())// jeżeli zaznacozny jest przycisk opcji do analizy w przestrzeni i czasie
    {
        // utwórz nowe obiekty serii danych liniowych
        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        // usuń poprzednie informacje z okna informacyjnego
        ui->resultsLabel->setText("");
        ui->resultsLabel2->setText("");
        // wpisz dane z tablic przebiegów do serii danych kolejnych obiektów dla osi X i Y od czasu
        for(int i=0; i < array1Length+1; i++)
            series1Object1->append((double)i/10,object1ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series1Object2->append((double)i/10,object2ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series2Object1->append((double)i/10,object1ArrayX[i]);

        for(int i=0; i < array1Length+1; i++)
            series2Object2->append((double)i/10,object2ArrayX[i]);
        // utwórz nowe obiekty wykresu
        chart1 = new Chart(series1Object1, series1Object2, "Wykres położenia od czasu", "Czas [s]", "Położenie Y ", array1Length/10, 480, false);
        chart2 = new Chart(series2Object1, series2Object2, "Wykres położenia od czasu", "Czas [s]", "Położenie X ", array1Length/10, 640, false);
        m_fftChart = nullptr;
        // ustaw obiekt wykresu jako widżet okna modularnego
        ui->analysisDock->setWidget(chart1);
        ui->cameraviewDock->setWidget(chart2);
        // wywołanie metod zwracających pole powierzchni przebiegów oraz wyświetlenie informacji
        ui->resultsLabel->setText(QString("<span style=\"color:#f40659;\">Pole 1 oś Y: <b>%1</b></span>"
                                          "<br/><span style=\"color:#0033cc;\">Pole 2 oś Y: <b>%2</b></span>"
                                          "<br/><span style=\"color:#f40659;\">Pole 1 oś X: <b>%3</b></span>"
                                          "<br/><span style=\"color:#0033cc;\">Pole 2 oś X: <b>%4</b></span>")
                                   .arg(chart1->getSurfaceInfo1()).arg(chart1->getSurfaceInfo2())
                                  .arg(chart2->getSurfaceInfo1()).arg(chart2->getSurfaceInfo2()));

    }else if(ui->inSpaceRadio->isChecked())// jeżeli zaznaczony jest przycisk opcji do analizy w przestrzeni osi X i Y
    {
        // utwórz nowe obiekty serii danych liniowych
        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        // usuń poprzednie informacje z okna informacyjnego
        ui->resultsLabel->setText("");
        ui->resultsLabel2->setText("");
        // wpisz dane z tablic przebiegów do serii danych kolejnych obiektów dla osi X i Y
        for(int i=0; i < array1Length+1; i++)
            series1Object1->append(object1ArrayX[i],object1ArrayY[i]);

        for(int i=0; i < array1Length+1; i++)
            series1Object2->append(object2ArrayX[i],object2ArrayY[i]);

        // utwórz nowe obiekty wykresu
        chart1 = new Chart(series1Object1, series1Object2, "Wykres położenia w przestrzeni", "Położenie X ", "Położenie Y ", 640, 480, false, true);
        chart2 = nullptr;
        m_fftChart = nullptr;
        // ustaw obiekt wykresu jako widżet okna modularnego
        ui->analysisDock->setWidget(chart1);
    }else// jeżeli zaznacozny jest przycisk opcji do analizy dwóch plików
    {
        // utwórz nowe obiekty serii danych liniowych
        series1Object1 = new QLineSeries();
        series1Object2 = new QLineSeries();
        series2Object1 = new QLineSeries();
        series2Object2 = new QLineSeries();

        // sprawdzenie która z serii danych ma mniejszy zakres tablicy
        int arrayLength;
        // sprawdź który z przebiegów z dwóch plików ma krótszy zakres pomiarowy i przypisz ten zakres do zmiennej
        if(array2Length>array1Length)
            arrayLength = array1Length;
        else
            arrayLength = array2Length;
        // ponieważ nie korzystamy z wykresu dwa, więc trzeba przypisać jego wskaźnik na nullptr
        chart2 = nullptr;
        // sprawdzenie który z przycisków opcji dotyczącego analizy FFT oraz numeru obiektu i badanej osi został zaznaczony
        // w zależności od tego załaduj serię danych oraz utwórz obiekty wykresów na podstawie tych danych
        if(ui->obj1YRadio->isChecked())
        {
            for(int i=0; i < array1Length+1; i++)
                series1Object1->append((double)i/10,object1ArrayY[i]);

            for(int i=0; i < array2Length+1; i++)
                series2Object1->append((double)i/10,object1ArrayY2[i]);

            m_fftChart = new FftChart(object1ArrayY, object1ArrayY2, array1Length, array2Length, "Wykres FFT", fileName1, fileName2, "1", "Amplituda położenia Y");
            chart1 = new Chart(series1Object1, series2Object1, "Wykres położenia od czasu", "Czas [s]", "Położenie Y ", arrayLength/10, 480
                               , false, false, "1 z pliku "+fileName1, "1 z pliku "+fileName2);
        }
        else if(ui->obj2YRadio->isChecked())
        {
            for(int i=0; i < array1Length+1; i++)
                series1Object2->append((double)i/10,object2ArrayY[i]);

            for(int i=0; i < array2Length+1; i++)
                series2Object2->append((double)i/10,object2ArrayY2[i]);

            m_fftChart = new FftChart(object2ArrayY, object2ArrayY2, array1Length, array2Length, "Wykres FFT", fileName1, fileName2, "2", "Amplituda położenia Y");
            chart1 = new Chart(series1Object2, series2Object2, "Wykres położenia od czasu", "Czas [s]", "Położenie Y ", arrayLength/10, 480
                               , false, false, "2 z pliku "+fileName1, "2 z pliku "+fileName2);
        }
        else if(ui->obj1XRadio->isChecked())
        {
            for(int i=0; i < array1Length+1; i++)
                series1Object1->append((double)i/10,object1ArrayX[i]);

            for(int i=0; i < array2Length+1; i++)
                series2Object1->append((double)i/10,object1ArrayX2[i]);

            m_fftChart = new FftChart(object1ArrayX, object1ArrayX2, array1Length, array2Length, "Wykres FFT", fileName1, fileName2, "1", "Amplituda położenia X");
            chart1 = new Chart(series1Object1, series2Object1, "Wykres położenia od czasu", "Czas [s]", "Położenie X ", arrayLength/10, 640
                               , false, false, "1 z pliku "+fileName1, "1 z pliku "+fileName2);
        }
        else
        {
            for(int i=0; i < array1Length+1; i++)
                series1Object2->append((double)i/10,object2ArrayX[i]);

            for(int i=0; i < array2Length+1; i++)
                series2Object2->append((double)i/10,object2ArrayX2[i]);

             m_fftChart = new FftChart(object2ArrayX, object2ArrayX2, array1Length, array2Length, "Wykres FFT", fileName1, fileName2, "2", "Amplituda położenia X");
             chart1 = new Chart(series1Object2, series2Object2, "Wykres położenia od czasu", "Czas [s]", "Położenie X ", arrayLength/10, 640
                                , false, false, "2 z pliku "+fileName1, "2 z pliku "+fileName2);
        }

        // wywołanie metod zwracających pole powierzchni przebiegów oraz wyświetlenie informacji
        ui->resultsLabel2->setText(QString("<span style=\"color:#f40659;\">Pole obiekt 1: <b>%1</b></span>"
                                           "<br/><span style=\"color:#0033cc;\">Pole obiekt 2: <b>%2</b></span>")
                                   .arg(chart1->getSurfaceInfo1()).arg(chart1->getSurfaceInfo2()));
        ui->analysisDock->setWidget(chart1);
        ui->cameraviewDock->setWidget(m_fftChart);
        ui->resultsLabel->setText(m_fftChart->resultsMessage());
    }
}
