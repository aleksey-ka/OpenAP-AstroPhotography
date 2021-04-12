// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QMainWindow>

#include <QFutureWatcher>
#include <QTimer>
#include <QSettings>

#include "ImageView.h"

#include "Hardware.Camera.h"
#include "Hardware.Focuser.h"
#include "Hardware.FilterWheel.h"

#include "MainFrame.Tools.h"

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

    void on_cameraSelectionCombo_currentIndexChanged( int );

    void on_cameraOpenCloseButton_clicked();

    void on_exposureSpinBox_valueChanged( int value );

    void on_exposureSlider_valueChanged( int value );

    void on_gainSpinBox_valueChanged( int value );

    void on_gainSlider_valueChanged( int value );

    void on_captureButton_clicked();

    void on_guideUp();

    void on_guideDown();

    void on_guideLeft();

    void on_guideRight();

    void on_coolerCheckBox_stateChanged( int );

    void on_temperatureSpinBox_valueChanged( int );

    void on_filterWheelComboBox_currentIndexChanged( int index );

    void on_imageView_imagePressed( int x, int y, Qt::MouseButton, Qt::KeyboardModifiers );

protected:
    void resizeEvent( QResizeEvent* ) override;

private:
    Ui::MainFrame *ui;
    ImageView* zoomView;
    void updateUI();

    void showZoom( bool update = true );

    QSettings settings;

    std::vector<std::shared_ptr<Hardware::CAMERA_INFO>> camerasInfo;
    std::shared_ptr<Hardware::Camera> camera;

    std::shared_ptr<Hardware::CAMERA_INFO> openCamera( int index );
    void closeCamera();

    // Capture
    QFutureWatcher<std::shared_ptr<const CRawU16Image>> imageReadyWatcher;
    QFutureWatcher<QString> imageSavedWatcher;
    std::shared_ptr<const CRawU16Image> currentImage;
    int zoom = 0;
    QPoint zoomCenter;
    int exposureRemainingTime;
    uint64_t seriesId = 0;
    QAtomicInt capturedFrames = 0;
    ulong startTimestamp = 0;
    ulong previousTimestamp = 0;
    QTimer exposureTimer;
    void startCapture();
    void showCaptureStatus();
    void imageReady();
    QString saveToPath;
    void imageSaved();

    // Manual guider controls
    int guiding = -1;
    void guide( Hardware::GUIDE_DIRECTION );
    void guideStop();

    // Focuser
    std::shared_ptr<Hardware::Focuser> focuser;
     // Filter wheel
    std::shared_ptr<Hardware::FilterWheel> filterWheel;

    Tools tools;

    // Rendering
    ulong render( const ushort* raw, int width, int height, int bitDepth );

    // Exposure controls and scaling
    void setExposureInSpinBox( int exposure );
    static int sliderPosToExposure( int pos );
    static int exposureToSliderPos( int exposure );
    static int exposureSuffixToScale( const QString& suffix );
    static int exposureToScaleAndSuffix( int exposure, QString& suffix );
    static QString exposureToString( int exposure );
    static int exposureFromValueAndSuffix( int value, const QString& suffix );

    // Temperature control
    void setCooler( bool isOn, int targetTemperature );

    bool debugMode = false;
    void doIt();
};

#endif // MAINFRAME_H
