#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QMainWindow>

#include <QFutureWatcher>
#include <QTimer>
#include <QSettings>

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

    void on_exposureSpinBox_valueChanged( int value );

    void on_exposureSlider_valueChanged( int value );

    void on_gainSpinBox_valueChanged( int value );

    void on_gainSlider_valueChanged( int value );

    void on_captureButton_clicked();

    void on_guideUp();
    void on_guideDown();
    void on_guideLeft();
    void on_guideRight();

private:
    Ui::MainFrame *ui;
    QSettings settings;

    std::vector<std::shared_ptr<ASI_CAMERA_INFO>> camerasInfo;
    std::shared_ptr<ASICamera> camera;

    // Capture
    QFutureWatcher<std::shared_ptr<const Raw16Image>> imageReadyWatcher;
    QFutureWatcher<QString> imageSavedWatcher;
    int exposureRemainingTime;
    int capturedFrames = 0;
    QTimer exposureTimer;
    void startCapture();
    void showCaptureStatus();
    void imageReady();
    QString saveToPath;
    void imageSaved();

    // Manual guider controls
    int guiding = -1;
    void guide( ASI_GUIDE_DIRECTION );
    void guideStop();

    // Rendering
    ulong render( const ushort* raw, int width, int height );
    void renderHistogram( const uint* r, const uint* g, const uint* b, int size );
    bool drawTargetingCircle = false;

    // Exposure controls and scaling
    void setExposureInSpinBox( int exposure );
    static int sliderPosToExposure( int pos );
    static int exposureToSliderPos( int exposure );
    static int exposureSuffixToScale( const QString& suffix );
    static int exposureToScaleAndSuffix( int exposure, QString& suffix );
    static int exposureFromValueAndSuffix( int value, const QString& suffix );


};

#endif // MAINFRAME_H
