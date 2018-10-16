#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QPixmap>
#include <DepthSense.hxx>

class CameraThread : public QThread
{
    Q_OBJECT
public:
    explicit CameraThread(QObject *parent=nullptr);

    void set(DepthSense::Context&c);
signals:

public slots:
    void run() override;
private:
    DepthSense::Context    *m_context;
};

#endif // CAMERATHREAD_H
