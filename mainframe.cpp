// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "mainframe.h"
#include "ui_mainframe.h"
#include <QShortcut>
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QDebug>

#include <chrono>

MainFrame::MainFrame( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainFrame )
{
    setWindowIcon( QIcon( ":mainframe.ico" ) );

    ui->setupUi( this ) ;

    ui->closeButton->hide();

    ui->imageView->setBackgroundRole( QPalette::Base );
    ui->imageView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->imageView->setScaledContents( false );
    ui->imageView->setAlignment( Qt::AlignCenter );

    ui->histogramView->setBackgroundRole( QPalette::Base );
    ui->histogramView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->histogramView->setScaledContents( false );
    ui->histogramView->setAlignment( Qt::AlignCenter );

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this, SLOT( on_toggleFullScreenButton_clicked() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ), this, SLOT( on_closeButton_clicked() ) );

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Up ), this, SLOT( on_guideUp() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Down ), this, SLOT( on_guideDown() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Left ), this, SLOT( on_guideLeft() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Right ), this, SLOT( on_guideRight() ) );

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ), this, [=]() { drawTargetingCircle = !drawTargetingCircle; } );

    connect( &imageReadyWatcher, &QFutureWatcher<std::shared_ptr<Raw16Image>>::finished, this, &MainFrame::imageReady );
    connect( &imageSavedWatcher, &QFutureWatcher<QString>::finished, this, &MainFrame::imageSaved );

    connect( &exposureTimer, &QTimer::timeout, [=]() { exposureRemainingTime--; showCaptureStatus(); } );

    int count = ASICamera::GetCount();
    for( int i = 0; i < count; i++ ) {
        camerasInfo.emplace_back( ASICamera::GetInfo( i ) );
    }

    for( int i = 0; i < count; i++ ) {
        ui->cameraSelectionCombo->addItem( camerasInfo[i]->Name, QVariant( i ) );
    }

    saveToPath = settings.value( "SaveTo" ).toString();
    ui->saveToEdit->setText( saveToPath );
    ui->saveToEdit->home( false );

    setExposureInSpinBox( settings.value( "Exposure", 100000 ).toInt() );
    ui->gainSpinBox->setValue( settings.value( "Gain", 0 ).toInt() );
    ui->offsetSpinBox->setValue( settings.value( "Offset", 64 ).toInt() );
    ui->useCameraWhiteBalanceCheckBox->setChecked( settings.value( "UseCameraWhiteBalance", false ).toBool() );
}

MainFrame::~MainFrame()
{
    guideStop();
    delete ui;
}

void MainFrame::on_closeButton_clicked()
{
    close();
}

void MainFrame::on_toggleFullScreenButton_clicked()
{
    bool isFullScreen = windowState().testFlag( Qt::WindowFullScreen );
    if( isFullScreen ) {
        ui->closeButton->hide();
        showNormal();  
    } else {
        showFullScreen();
        ui->closeButton->show();
    }
}

void MainFrame::on_exposureSpinBox_valueChanged( int value )
{
    int exposure = exposureFromValueAndSuffix( value, ui->exposureSpinBox->suffix() );
    ui->exposureSlider->setValue( exposureToSliderPos( exposure ) );
}

void MainFrame::on_exposureSlider_valueChanged( int pos )
{
    int exposure = sliderPosToExposure( pos );
    setExposureInSpinBox( exposure );
}

void MainFrame::on_gainSpinBox_valueChanged( int value )
{
    ui->gainSlider->setValue( value );
}

void MainFrame::on_gainSlider_valueChanged( int value )
{
    ui->gainSpinBox->setValue( value );
}

void MainFrame::on_coolerCheckBox_stateChanged( int state )
{
    setCooler( state == Qt::Checked, ui->temperatureSpinBox->value() );
}

void MainFrame::on_temperatureSpinBox_valueChanged( int targetTemperature )
{
    setCooler( ui->coolerCheckBox->isChecked(), targetTemperature );
}

void MainFrame::on_guideUp()
{
    guide( ASI_GUIDE_NORTH );
}

void MainFrame::on_guideDown()
{
    guide( ASI_GUIDE_SOUTH );
}

void MainFrame::on_guideLeft()
{
    guide( ASI_GUIDE_EAST );
}

void MainFrame::on_guideRight()
{
    guide( ASI_GUIDE_WEST );
}

void MainFrame::guide( ASI_GUIDE_DIRECTION direction )
{
    if( guiding != -1 ) {
        camera->GuideOff( (ASI_GUIDE_DIRECTION)guiding );
    }
    if( guiding != direction ) {
        camera->GuideOn( direction );
        guiding = direction;
    } else {
        guiding = -1;
    }
}

void MainFrame::guideStop()
{
    if( guiding != -1 ) {
        camera->GuideOff( (ASI_GUIDE_DIRECTION)guiding );
        guiding = -1;
    }
}

void MainFrame::on_captureButton_clicked()
{
    if( ui->saveToCheckBox->isChecked() ) {
        auto path = ui->saveToEdit->text();
        settings.setValue( "SaveTo", path );

        static const QString TIMESTAMP( "{TIMESTAMP}" );
        auto timestamp = QDateTime::currentDateTime().toString("yyyy-MM-ddThh-mm-ss");
        if( path.contains( TIMESTAMP ) ) {
            path.replace( TIMESTAMP, timestamp );
        } else {
            path = path + QDir::separator() + timestamp;
        }
        saveToPath = path;
    } else {
        saveToPath.clear();
    }

    settings.setValue( "Exposure", ui->exposureSpinBox->value() * exposureSuffixToScale( ui->exposureSpinBox->suffix() ) );
    settings.setValue( "Gain", ui->gainSpinBox->value() );
    settings.setValue( "Offset", ui->offsetSpinBox->value() );
    settings.setValue( "UseCameraWhiteBalance", ui->useCameraWhiteBalanceCheckBox->isChecked() );

    startCapture();
}

void MainFrame::startCapture()
{
    int selectedIndex = ui->cameraSelectionCombo->currentData().toInt();

    auto exposure = exposureFromValueAndSuffix( ui->exposureSpinBox->value(), ui->exposureSpinBox->suffix() );
    auto gain = ui->gainSpinBox->value();
    auto offset = ui->offsetSpinBox->value();
    auto useCameraWhiteBalance = ui->useCameraWhiteBalanceCheckBox->isChecked();
    auto coolerOn = ui->coolerCheckBox->isChecked();
    auto targetTemperature = ui->temperatureSpinBox->value();

    if( camerasInfo.size() > 0 ) {
        auto start = std::chrono::steady_clock::now();

        if( camera == nullptr ) {
            std::shared_ptr<ASI_CAMERA_INFO> cameraInfo = camerasInfo[selectedIndex];
            qDebug() << cameraInfo->Name;

            camera = ASICamera::Open( cameraInfo->CameraID );

            setWindowTitle( cameraInfo->Name );

            int width = 0;
            int height = 0;
            int bin = 0;
            ASI_IMG_TYPE imgType = ASI_IMG_END;
            camera->GetROIFormat( width, height, bin, imgType );
            imgType = ASI_IMG_RAW16;
            camera->SetROIFormat( width, height, bin, imgType );
            camera->GetROIFormat( width, height, bin, imgType );
            qDebug() << width << "x" << height << " bin" << bin;
            switch( imgType ) {
                case ASI_IMG_RAW8: qDebug() << "RAW8"; break;
                case ASI_IMG_RGB24: qDebug() << "RGB24"; break;
                case ASI_IMG_RAW16: qDebug() << "RAW16"; break;
                case ASI_IMG_Y8: qDebug() << "Y8"; break;
                default:
                    assert( false );
            }

            setCooler( coolerOn, targetTemperature );

            //camera->PrintDebugInfo();

            auto end = std::chrono::steady_clock::now();
            auto msec = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
            qDebug() << "Camera initialized in" << msec << "msec";
        }

        bool isAuto = false;
        long min, max, defaultVal;

        camera->SetExposure( exposure );
        long exposure = camera->GetExposure( isAuto );
        camera->GetExposureCaps( min, max, defaultVal );
        qDebug() << "Exposure "<< exposure << ( isAuto ? " (auto)" : "" ) <<
            "[" << min << max << defaultVal << "]";

        camera->SetGain( gain );
        long gain = camera->GetGain( isAuto );
        camera->GetGainCaps( min, max, defaultVal );
        qDebug() << "Gain " << gain << ( isAuto ? "(auto)" : "" ) <<
            "[" << min << max << defaultVal << "]";

        camera->SetWhiteBalanceR( useCameraWhiteBalance ? 52 : 50 );
        camera->GetWhiteBalanceRCaps( min, max, defaultVal );
        qDebug() << "WB_R" << camera->GetWhiteBalanceR() <<
            "[" << min << max << defaultVal << "]";

        camera->SetWhiteBalanceB( useCameraWhiteBalance ? 95 : 50 );
        camera->GetWhiteBalanceBCaps( min, max, defaultVal );
        qDebug() << "WB_B" << camera->GetWhiteBalanceB() <<
            "[" << min << max << defaultVal << "]";

        camera->SetOffset( offset );
        camera->GetOffsetCaps( min, max, defaultVal );
        qDebug() << "Offset" << camera->GetOffset() <<
            "[" << min << max << defaultVal << "]";

        imageReadyWatcher.setFuture( QtConcurrent::run( [=]() {
            return camera->DoExposure();
        } ) );

    } else {
        qDebug() << "No camera";

        imageReadyWatcher.setFuture( QtConcurrent::run( [=]() {
            QThread::usleep( exposure );
            return Raw16Image::LoadFromFile( "image.cfa" );
        } ) );
    }

    exposureRemainingTime = exposure / 1000000;
    exposureTimer.start( 1000 );
    showCaptureStatus();

    ui->captureButton->setEnabled( false );
}

void MainFrame::showCaptureStatus()
{
    ui->captureButton->setText( QString::number( exposureRemainingTime ) +
        ( capturedFrames > 0 ? " (" + QString::number( capturedFrames ) + ")" : "" ) );
}

void MainFrame::imageReady()
{
    exposureTimer.stop();
    ui->captureButton->setText( "Capture" );

    auto result = imageReadyWatcher.result();

    if( camera ) {
        qDebug() << "Camera temperature is" << camera->GetCurrentTemperature();
    }

    if( saveToPath.length() > 0 ) {
        imageSavedWatcher.setFuture( QtConcurrent::run( [=]() {
            QDir().mkpath( saveToPath );
            auto name = QString::number( capturedFrames ).rightJustified( 5, '0' ) + ".cfa";
            result->SaveToFile( ( saveToPath + QDir::separator() + name ).toStdString().c_str() );
            return QString( name );
        } ) );
    }

    auto msec = render( result->RawPixels(), result->Width(), result->Height() );
    qDebug() << "Rendered in " << msec << "msec";

    if( ui->continuousCaptureCheckBox->isChecked() ) {
        capturedFrames++;
        ui->continuousCaptureCheckBox->setText( "Continuous capture (uncheck to stop)" );
        startCapture();
    } else {
        capturedFrames = 0;
        ui->continuousCaptureCheckBox->setText( "Continuous capture" );
        ui->captureButton->setEnabled( true );
    }
}

void MainFrame::imageSaved()
{
    // On image saved
}

ulong MainFrame::render( const ushort* raw, int width, int height )
{
    auto start = std::chrono::steady_clock::now();

    std::vector<uint> histR( 256 );
    std::fill( histR.begin(), histR.end(), 0 );
    std::vector<uint> histG( 256 );
    std::fill( histG.begin(), histG.end(), 0 );
    std::vector<uint> histB( 256 );
    std::fill( histG.begin(), histG.end(), 0 );

    size_t w = width / 2;
    size_t h = height / 2;
    size_t byteWidth = 3 * w;
    std::vector<uchar> pixels( byteWidth * h );
    uchar* rgb = pixels.data();
    for( size_t y = 0; y < h; y++ ) {
        const ushort* srcLine = raw + 2 * width * y;
        uchar* dstLine = rgb + byteWidth * y;
        for( size_t x = 0; x < w; x++ ) {
            const ushort* src = srcLine + 2 * x;
            uchar r = src[0] / 256;
            uchar g = src[1] / 256;
            uchar b = src[width + 1] / 256;

            uchar* dst = dstLine + 3 * x;
            dst[0] = r;
            dst[1] = g;
            dst[2] = b;

            if( r < histR.size() ) {
                histR[r]++;
            }
            if( g < histG.size() ) {
                histG[g]++;
            }
            if( b < histB.size() ) {
                histB[b]++;
            }
        }
    }

    QImage image( rgb, w, h, QImage::Format_RGB888 );
    QPixmap pixmap = QPixmap::fromImage( image );
    if( drawTargetingCircle ) {
        QPainter painter( &pixmap );
        QPen pen( QColor::fromRgb( 0xFF, 0, 0 ) );
        pen.setWidth( 3 );
        painter.setPen( pen );
        painter.drawEllipse( w / 2 - 30, h / 2 - 30, 60, 60 );
    }
    ui->imageView->setPixmap( pixmap );

    renderHistogram( histR.data(), histG.data(), histB.data(), histR.size() );

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
}

void MainFrame::renderHistogram( const uint* r, const uint* g, const uint* b, int size )
{
    uint max = 0;
    for( int i = 0; i < size - 1; i++ ) {
        if( r[i] > max ) {
            max = r[i];
        }
        if( g[i] > max ) {
            max = g[i];
        }
        if( b[i] > max ) {
            max = b[i];
        }
    }

    int h = 128;
    int biteWidth = 3 * size;
    std::vector<uchar> hist( biteWidth * h );
    std::fill( hist.begin(), hist.end(), 0 );
    uchar* p = hist.data();
    for( int i = 0; i < size; i++ ) {
        int R = 127 - ( 127 * r[i] ) / max;
        int G = 127 - ( 127 * g[i] ) / max;
        int B = 127 - ( 127 * b[i] ) / max;
        for( int j = 0; j < h; j++ ) {
            uchar* p0 = p + j * biteWidth + 3 * i;
            if( R < 127 ) {
                if( j > R ) {
                    p0[0] = 0x50;
                } else if( j == R ){
                    p0[0] = 0xFF;
                }
            }
            if( G < 127 ) {
                if( j > G ) {
                    p0[1] = 0x50;
                } else if( j == G ){
                    p0[1] = 0xFF;
                }
            }
            if( B < 127 ) {
                if( j > B ) {
                    // Make blue on black brighter (otherwise it is too dim)
                    if( p0[0] == 0 && p0[1] == 0 ) {
                        p0[2] = 0x80;
                    } else {
                        p0[2] = 0x50;
                    }
                } else if( j == B ){
                    // Make blue outline briter
                    p0[0] = 0x80;
                    p0[1] = 0x80;
                    p0[2] = j < 127 ? 0xFF : 0;
                }
            }
        }
    }

    QImage image( p, size, h, QImage::Format_RGB888 );
    ui->histogramView->setPixmap( QPixmap::fromImage( image ) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exposure scaling

void MainFrame::setExposureInSpinBox( int exposure )
{
    // First set the slider to the appropriate range
    ui->exposureSlider->setValue( exposureToSliderPos( exposure ) );

    // Now set the exposure in the exposure spin box to the exact value
    // with convinient time units (s/ms/us)
    QString suffix;
    exposure /= exposureToScaleAndSuffix( exposure, suffix );
    ui->exposureSpinBox->setSuffix( suffix );
    ui->exposureSpinBox->setValue( exposure );
}

// Slider position to exposure translation table (quasi logorithmic)
static int exposures[] =
    { 1, 1, 1, 2, 2, 3, 4, 5, 6, 8,
      10, 12, 15, 20, 25, 30, 40, 50, 60, 80,
      100, 120, 150, 200, 250, 300, 400, 500, 600, 800,
      1000, 1200, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 8000,
      10000, 12000, 15000, 20000, 25000, 30000, 40000, 50000, 60000, 80000,
      100000, 120000, 150000, 200000, 250000, 300000, 400000, 500000, 600000, 800000,
      1000000, 1200000, 1500000, 2000000, 2500000, 3000000, 4000000, 5000000, 6000000, 8000000,
      10000000, 12000000, 15000000, 20000000, 25000000, 30000000, 40000000, 50000000, 60000000, 80000000,
      100000000 };

int MainFrame::sliderPosToExposure( int pos )
{
    return exposures[pos];
}

int MainFrame::exposureToSliderPos( int exposure )
{
    const int lastPos = sizeof( exposures ) / sizeof( int ) - 1;
    for( int i = 0; i <= lastPos; i++ ) {
        if( exposures[i] > exposure ) {
            return i - 1;
        }
    }
    return lastPos;
}

int MainFrame::exposureSuffixToScale( const QString& suffix )
{
    if( suffix == " s" ) {
        return 1000000;
    } else if( suffix == " ms" ) {
        return 1000;
    } else {
        assert( suffix == " us");
        return 1;
    }
}

int MainFrame::exposureToScaleAndSuffix( int exposure, QString& suffix )
{
    if( exposure >= 10000000 ) {
        // Seconds for exposures equal or above 10 s
        suffix = " s";
        return 1000000;
    } else if( exposure >= 10000 ) {
        // Milliseconds for exposures equal or above 10 ms
        suffix = " ms";
        return 1000;
    } else {
        // Microseconds for exposures below 10 ms
        suffix = " us";
        return 1;
    }
}

int MainFrame::exposureFromValueAndSuffix( int value, const QString& suffix )
{
    return value *  exposureSuffixToScale( suffix );
}

 // Temperature control
void MainFrame::setCooler( bool coolerOn, int targetTemperature )
{
    if( coolerOn ) {
        camera->SetTargetTemperature( targetTemperature );
    }
    camera->SetCoolerOn( coolerOn );
}
