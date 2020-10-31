#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QMainWindow>

#include <QFutureWatcher>
#include <QTimer>

#include "camera.h"

namespace Ui {
    class MainFrame;
}

class MainFrame : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainFrame( QWidget *parent = nullptr );
    ~MainFrame();

private slots:
    void on_closeButton_clicked();

    void on_toggleFullScreenButton_clicked();

    void on_exposureSlider_valueChanged( int value );

    void on_gainSlider_valueChanged( int value );

    void on_captureButton_clicked();

private:
    Ui::MainFrame *ui;

    std::vector<std::shared_ptr<ASI_CAMERA_INFO>> camerasInfo;
    std::shared_ptr<ASICamera> camera;

    QFutureWatcher<std::shared_ptr<const ASICamera::Image>> imageReadyWatcher;
    int exposureRemainingTime;
    QTimer exposureTimer;
    void imageReady();

    ulong render( const ushort* raw, int width, int height );
    void renderHistogram( const uint* r, const uint* g, const uint* b, int size );
};

#endif // MAINFRAME_H
