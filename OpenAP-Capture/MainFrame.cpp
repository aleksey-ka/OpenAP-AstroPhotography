// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "MainFrame.h"
#include "ui_MainFrame.h"

#include <QShortcut>
#include <QtConcurrent/QtConcurrent>
#include <QPainter>
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

#include "Renderer.h"

#include "Image.Qt.h"

#include <chrono>
#include <limits>

MainFrame::MainFrame( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::MainFrame ),
    tools( ui )
{
    // Multithreading noticibly improves throughput especially when writing to compressed image formats
    int numberOfCores = QThread::idealThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount( std::min( numberOfCores, 4 ) );

    setWindowIcon( QIcon( ":MainFrame.ico" ) );

    ui->setupUi( this ) ;

    ui->closeButton->hide();
    ui->imageSeriesView->hide();

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this, SLOT( on_toggleFullScreenButton_clicked() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ), this, SLOT( on_closeButton_clicked() ) );

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Up ), this, SLOT( on_guideUp() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Down ), this, SLOT( on_guideDown() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Left ), this, SLOT( on_guideLeft() ) );
    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Right ), this, SLOT( on_guideRight() ) );

    // TODO: In Qt 5.15 lambdas can be used in QShortcut constructor
    connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ), this ), &QShortcut::activated, [=]() { tools.Toggle<Tools::TargetCircle>(); } );

    connect( &imageReadyWatcher, &QFutureWatcher<std::shared_ptr<CRawU16Image>>::finished, this, &MainFrame::imageReady );
    connect( &imageSavedWatcher, &QFutureWatcher<QString>::finished, this, &MainFrame::imageSaved );

    connect( &exposureTimer, &QTimer::timeout, [=]() { if( exposureRemainingTime > 0 ) exposureRemainingTime--; showCaptureStatus(); } );

    int count = Hardware::Camera::GetCount();
    for( int i = 0; i < count; i++ ) {
        camerasInfo.emplace_back( Hardware::Camera::GetInfo( i ) );
    }

    // TODO: Fixing a bug with text color on Raspberry Pi (old Qt?). It shows always gray
    // To fix it needs changing the combo to editable and the edit inside the combo to read-only
    ui->cameraSelectionCombo->lineEdit()->setReadOnly( true );

    {
        QSignalBlocker lock( ui->cameraSelectionCombo );
        int selectedIndex = 0;
        static auto selectedCamera = settings.value( "SelectedCamera" ).toString();
        for( size_t i = 0; i < camerasInfo.size(); i++ ) {
            auto name = camerasInfo[i]->Name;
            ui->cameraSelectionCombo->addItem( name, QVariant( i ) );
            if( name == selectedCamera ) {
                selectedIndex = i;
            }
        }
        if( selectedCamera.startsWith( ":" ) ) {
            auto path = selectedCamera.mid( 1 );
            ui->cameraSelectionCombo->addItem( QFileInfo( path ).fileName(), QVariant( path ) );
            selectedIndex = ui->cameraSelectionCombo->count() - 1;
        }
        ui->cameraSelectionCombo->setCurrentIndex( selectedIndex );
        ui->cameraSelectionCombo->addItem( "Open Folder...", QVariant( "" ) );
    }
    connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_O ), this ), &QShortcut::activated, [=]() { on_cameraSelectionCombo_currentIndexChanged( INT_MAX ); } );

    ui->objectNameEdit->setText( settings.value( "Name" ).toString() );
    ui->formatComboBox->setCurrentText( settings.value( "FileFormat", ".pixels" ).toString() );

    auto defaultPath = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation ) + QDir::separator() +
            QApplication::applicationName() + QDir::separator() + "{TIME}{NAME}{FILTER}";
    saveToPath = settings.value( "SaveTo", defaultPath ).toString();
    ui->saveToEdit->setText( saveToPath );
    ui->saveToEdit->home( false );

    setExposureInSpinBox( settings.value( "Exposure", 100000 ).toInt() );
    ui->gainSpinBox->setValue( settings.value( "Gain", 0 ).toInt() );
    ui->offsetSpinBox->setValue( settings.value( "Offset", 64 ).toInt() );
    ui->useCameraWhiteBalanceCheckBox->setChecked( settings.value( "UseCameraWhiteBalance", false ).toBool() );

    focuser = Hardware::Focuser::Open();
    if( focuser != 0 ) {
       // TODO: In Qt 5.15 lambdas can be used in QShortcut constructor
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Up ), this ), &QShortcut::activated, [=]() { focuser->Forward(); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Down ), this ), &QShortcut::activated, [=]() { focuser->Backward(); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Right ), this ), &QShortcut::activated, [=]() { focuser->StepUp(); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Left ), this ), &QShortcut::activated, [=]() { focuser->StepDown(); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_M ), this ), &QShortcut::activated, [=]() { focuser->MarkZero(); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_0 ), this ), &QShortcut::activated, [=]() { focuser->GoToPos( 0 ); } );
       connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_G ), this ),
            &QShortcut::activated, [=]() {
                bool ok = false;
                int pos = QInputDialog::getInt( this, "Focuser", "MoveTo:", 0, INT_MIN, INT_MAX, 1, &ok );
                if( ok ) {
                    focuser->GoToPos( pos );
                }
            }
       );
   }
   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_F ), this ), &QShortcut::activated, [=]() {
       if( zoomView->isHidden() ) {
           // If zoom is not showing, then we want to show it with a new instance of focusing helper
           tools.New<Tools::FocusingHelper>();
       } else {
           // Otherwise toggling focusing helper on and off
           tools.Toggle<Tools::FocusingHelper>();
       }
       showZoom();
   } );

   filterWheel = Hardware::FilterWheel::Open();
   if( filterWheel ) {
       // TODO: Fixing a bug with text color on Raspberry Pi (old Qt?). It shows always gray
       // To fix it needs changing the combo to editable and the edit inside the combo to read-only
       ui->filterWheelComboBox->lineEdit()->setReadOnly( true );
       ui->filterComboBox->lineEdit()->setReadOnly( true );

       auto filterWheelDefs = QDir( "", "*.FilterWheel" ).entryList( QDir::Files );
       if( filterWheelDefs.size() > 0 ) {
           QSignalBlocker lock( ui->filterWheelComboBox );
           int index = 0;
            for( int i = 0; i < filterWheelDefs.size(); i++ ) {
                auto name = QFileInfo( filterWheelDefs[i] ).baseName();
                ui->filterWheelComboBox->addItem( name );
                if( name == settings.value( "FilterWheel" ) ) {
                    index = i;
                }
            }
            ui->filterWheelComboBox->setCurrentIndex( index );
            on_filterWheelComboBox_currentIndexChanged( index );
       } else {
            ui->filterWheelComboBox->addItem( "Filter Wheel" );
       }

       connect( ui->filterComboBox, QOverload<int>::of( &QComboBox::currentIndexChanged ),
            [this]( int index ) { filterWheel->SetPosition( index ); } );
   }

   ui->imageView->setCursor( QCursor( QPixmap( ":Res.CrossHair.png" ), 23, 23 ) );
   connect( ui->imageView, SIGNAL( imagePressed( int x, int y, Qt::MouseButton, Qt::KeyboardModifiers ) ),
        SLOT( on_imageView_imagePressed( int x, int y, Qt::MouseButton, Qt::KeyboardModifiers ) ), Qt::UniqueConnection );

   ui->imageSeriesView->setFocusPolicy( Qt::ClickFocus );

   zoomView = new ImageView( ui->imageView );
   zoomView->setStyleSheet( "border-bottom:none;border-right:none;" );
   zoomView->hide();
   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_Z ), this ), &QShortcut::activated, [=]() {
       if( ++zoom > 3 ) {
            zoom = 3;
       } else {
            showZoom();
       }
   } );
   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Z ), this ), &QShortcut::activated, [=]() {
       if( --zoom <= 0 ) {
           zoom = 0;
           zoomView->hide();
       } else {
           showZoom();
       }
   } );
   connect( ui->zoomOffRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) { zoom = 0; zoomView->hide(); } } );
   connect( ui->zoomHalfRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) showZoom(); } );
   connect( ui->zoom1xRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) showZoom(); } );
   connect( ui->zoom2xRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) showZoom(); } );
   connect( ui->zoom4xRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) showZoom(); } );
   connect( ui->zoomCfaRadioButton, &QRadioButton::toggled, [=](bool checked ) { if( checked ) showZoom(); } );

   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ), this ), &QShortcut::activated, [=]() {
       if( camera == 0 ) {
            on_cameraOpenCloseButton_clicked();
       }
       on_captureButton_clicked();
   } );
   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_R ), this ), &QShortcut::activated, [=]() {
       if( camera == 0 ) {
            on_cameraOpenCloseButton_clicked();
       }
       ui->continuousCaptureCheckBox->setChecked( true );
       on_captureButton_clicked();
   } );

   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_D ), this ), &QShortcut::activated, [=]() { debugMode = !debugMode; } );
   connect( new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ), this ), &QShortcut::activated, [=]() {
       auto focusingHelperTool = tools.TryGet<Tools::FocusingHelper>();
       if( focusingHelperTool ) {
           focusingHelperTool->getFocusingHelper()->toggleGlobalPolarAllign();
       }
   } );

   updateUI();
}

MainFrame::~MainFrame()
{
    if( camera != 0 ) {
        closeCamera();
    }
    if( focuser != 0 ) {
        focuser->Close();
    }
    if( filterWheel != 0 ) {
        filterWheel->Close();
    }
    delete zoomView;
    delete ui;
}

void MainFrame::updateUI()
{
    ui->histogramView->setVisible( camera != 0 );
    ui->cameraControlsFrame->setVisible( camera != 0 );
    ui->captureFrame->setVisible( camera != 0 );
    ui->temperatureFrame->setVisible( camera != 0 && camera->HasCooler() );
    ui->cameraOpenCloseButton->setText( camera != 0 ? "X" : ">" );
    ui->useCameraWhiteBalanceCheckBox->setVisible( camera != 0 && camera->GetInfo()->IsColorCamera );
    ui->filterWheelFrame->setVisible( filterWheel != 0 );
}

void MainFrame::resizeEvent( QResizeEvent* event )
{
    QMainWindow::resizeEvent( event );
    if( zoomView->isVisible() ) {
        showZoom( false );
    }
}

static QPixmap focusingHelperPixmap( TRenderingMethod rendering, const CRawU16Image* image, int x0, int y0, int w, int h )
{
    if( rendering == RM_HalfResolution ) {
        int cx = x0 + w / 2;
        int cy = y0 + h / 2;
        x0 = cx - w;
        y0 = cy - h;
        x0 += x0 % 2;
        y0 += y0 % 2;
        w *= 2;
        h *= 2;
        return Qt::CreatePixmap( CRawU16( image ).StretchHalfRes( x0, y0, w, h ) );
    } else {
        return Qt::CreatePixmap( CRawU16( image ).Stretch( x0, y0, w, h ) );
    }
}

static void drawTimeSeries( QPainter& painter, QColor color, int shift, double scale, int offset, const std::vector<double>& data )
{
    QPen penB( color );
    penB.setWidthF( 0.5 );
    painter.setPen( penB );
    for( size_t i = 1; i < data.size(); i++ ) {
        int x1 = 5 * ( i - 1 + shift );
        int y1 = round( 300 + scale * data[i - 1] + offset);
        int x2 = 5 * ( i + shift );
        int y2 = round( 300 + scale * data[i] + offset );
        painter.drawLine( x1, y1, x2, y2 );
    }
}

void MainFrame::showZoom( bool update )
{  
    if( ui->zoomOffRadioButton->isChecked() ) {
        QSignalBlocker lock( ui->zoom1xRadioButton );
        ui->zoom1xRadioButton->setChecked( true );
    }
    TRenderingMethod rendering = RM_FullResolution;
    int scale = -1;
    if( ui->zoomHalfRadioButton->isChecked() ) {
        scale = 1;
        rendering = RM_HalfResolution;
    } else if( ui->zoom1xRadioButton->isChecked() ) {
        scale = 1;
    } else if( ui->zoom2xRadioButton->isChecked() ) {
        scale = 2;
    } else if( ui->zoom4xRadioButton->isChecked() ) {
        scale = 4;
    } else if( ui->zoomCfaRadioButton->isChecked() ) {
        scale = 4;
        rendering = RM_CFA;
    }
    if( zoom == 0 ) {
        zoom = 1;
    }
    if( update ) {
        int size = 320;
        int imageSize = ( size * zoom ) / scale + 1;
        zoomView->resize( imageSize * scale + 1, imageSize * scale + 1 );
        if( currentImage != 0 ) {
            QPixmap pixmap;
            QPoint c = zoomCenter.isNull() ? QPoint( currentImage->Width() / 2, currentImage->Height() / 2 ) : zoomCenter;
            auto focusingHelperTool = tools.TryGet<Tools::FocusingHelper>();
            if( focusingHelperTool ) {
                auto focusingHelper = focusingHelperTool->getFocusingHelper();
                // Lock on the star (center on local maximum) and measure its params
                int focuserPos = focuser != 0 ? focuser->GetPos() : INT_MIN;
                focusingHelper->AddFrame( currentImage.get(), imageSize, focuserPos );
                c.setX( focusingHelper->cx );
                c.setY( focusingHelper->cy );
                zoomCenter = c;

                if( ui->countourCheckBox->isChecked() ) {
                    // This mode is useful in many ways. Particularly it might be used over VNC to speed up drawing
                    pixmap = Qt::CreatePixmap( focusingHelper->Mask );
                } else if( ui->liveStackCheckBox->isChecked() ) {
                    focusingHelper->SetStackSize( ui->stackSizeSpinBox->value() );
                    pixmap = Qt::CreatePixmap( focusingHelper->GetStackedImage( ui->stretchCheckBox->isChecked(), ui->factorSpinBox->value() ) );
                } else {
                    pixmap = focusingHelperPixmap( rendering, currentImage.get(), c.x() - imageSize / 2, c.y() - imageSize / 2, imageSize, imageSize );
                }

                QPainter painter( &pixmap );
                QPen pen( QColor::fromRgb( 0xFF, 0, 0 ) );
                pen.setWidthF( 0.5 );
                painter.setPen( pen );
                painter.setRenderHint( QPainter::Antialiasing );
                painter.drawEllipse( pixmap.width() / 2 - focusingHelper->R, pixmap.height() / 2 - focusingHelper->R, 2 * focusingHelper->R, 2 * focusingHelper->R );
                painter.drawEllipse( pixmap.width() / 2 - focusingHelper->R_out, pixmap.height() / 2 - focusingHelper->R_out, 2 * focusingHelper->R_out, 2 * focusingHelper->R_out );

                QFont font( "Consolas" );
                font.setPointSizeF( 9 );
                painter.setFont( font );
                int pos = 0;
                if( focuser != 0 && focuserPos != INT_MIN ) {
                    painter.drawText( 5, pos += 15, QString( "FOCUSER %1 (%2)" ).arg( QString::number( focuserPos ), QString::number( focuser->StepsPerMove() ) ) );
                }
                painter.drawText( 5, pos += 15, QString( "HFD %1" ).arg( QString::number( focusingHelper->HFD, 'f', 2 ) ) );
                if( focusingHelper->extra.size() > 0 ) {
                    for( auto helper : focusingHelper->extra ) {
                        painter.drawText( 5, pos += 15, QString( "HFD %1" ).arg( QString::number( helper->HFD, 'f', 2 ) ) );
                    }
                    painter.drawText( 5, pos += 15, QString( "dCX %1 (%2)" ).arg( QString::number( focusingHelper->dCX, 'f', 2 ), QString::number( focusingHelper->sigmadCX, 'f', 2 ) ) );
                    painter.drawText( 5, pos += 15, QString( "dCY %1 (%2)" ).arg( QString::number( focusingHelper->dCY, 'f', 2 ), QString::number( focusingHelper->sigmadCY, 'f', 2 ) ) );
                    painter.drawText( 5, pos += 15, QString( "L %1 (%2)" ).arg( QString::number( focusingHelper->L, 'f', 2 ), QString::number( focusingHelper->sigmaL, 'f', 2 ) ) );
                } else {
                    painter.drawText( 5, pos += 15, QString( "dCX %1" ).arg( QString::number( focusingHelper->dCX, 'f', 2 ) ) );
                    painter.drawText( 5, pos += 15, QString( "dCY %1" ).arg( QString::number( focusingHelper->dCY, 'f', 2 ) ) );
                }

                if( debugMode ) {
                    // Drawing series of metrics tp see correlations. For now only in debug mode
                    const auto& seriesMax = focusingHelper->currentSeries->Max;
                    int scale = seriesMax.empty() ? 1 : seriesMax[0] / 150;
                    drawTimeSeries( painter, QColor::fromRgb( 0, 0xFF, 0 ), 0, -1.0 / scale, 150, seriesMax );
                    drawTimeSeries( painter, QColor::fromRgb( 0x70, 0x70, 0xFF ), 0, 20, 0, focusingHelper->currentSeries->FWHM );
                    drawTimeSeries( painter, QColor::fromRgb( 0xFF, 0, 0 ), 0, 30, 0, focusingHelper->currentSeries->HFD );

                    for( auto helper : focusingHelper->extra ) {
                        int shift = focusingHelper->currentSeries->Max.size() - helper->currentSeries->Max.size();
                        const auto& seriesMax = helper->currentSeries->Max;
                        int scale = seriesMax.empty() ? 1 : seriesMax[0] / 150;
                        drawTimeSeries( painter, QColor::fromRgb( 0xFF, 0xFF, 0 ), shift, -1.0 / scale, 150, seriesMax );
                        drawTimeSeries( painter, QColor::fromRgb( 0xFF, 0xFF, 0 ), shift, 20, 0, helper->currentSeries->FWHM );
                        drawTimeSeries( painter, QColor::fromRgb( 0xFF, 0xFF, 0 ), shift, 30, 0, helper->currentSeries->HFD );
                    }
                }

                if( focuser != 0 && focuserPos > INT_MIN && focusingHelper->focuserPositions.size() > 0 ) {
                    pen.setWidthF( 2.0 );
                    painter.setPen( pen );
                    for( size_t i = 0; i < focusingHelper->focuserPositions.size(); i++ ) {
                        auto stat = focusingHelper->getFocuserStatsByIndex( i );
                        if( stat.Pos > INT_MIN ) {
                            int x = ( 8 * ( stat.Pos - focuserPos ) ) / focuser->StepsPerMove() + imageSize / 2;
                            int y = (int)round( -15 * stat.HFD ) + imageSize;
                            painter.drawLine( x - 3, y, x + 3, y );
                        }
                    }
                }

            } else {
                // Not in focusing mode
                if( ui->stretchCheckBox->isChecked() ) {
                    pixmap = focusingHelperPixmap( rendering, currentImage.get(), c.x() - imageSize / 2, c.y() - imageSize / 2, imageSize, imageSize );
                } else {
                    Renderer renderer( currentImage->RawPixels(), currentImage->Width(), currentImage->Height(), currentImage->BitDepth() );
                    pixmap = renderer.Render( rendering, c.x() - imageSize / 2, c.y() - imageSize / 2, imageSize, imageSize );
                }
            }
            if( scale > 1 ) {
                pixmap = pixmap.scaled( imageSize * scale, imageSize * scale, Qt::IgnoreAspectRatio );
            }
            zoomView->setPixmap( pixmap );
        }
    }

    zoomRecalcLayout();
    zoomView->show();
}

void MainFrame::zoomRecalcLayout()
{
    auto parentRect = ui->imageView->frameGeometry();
    auto geometry = zoomView->geometry();
    zoomView->move( parentRect.right() - geometry.width(), parentRect.bottom() - geometry.height() );
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

void MainFrame::on_cameraSelectionCombo_currentIndexChanged( int index )
{
    if( camera != 0 ) {
        closeCamera();
        updateUI();
    }
    if( index >= (int) camerasInfo.size() ) {
        QString path = index != INT_MAX ? ui->cameraSelectionCombo->currentData().toString() : "";
        if( path.isEmpty() ) {
            auto savedPath = settings.value( "SelectedCamera" ).toString();
            if( savedPath.startsWith( ":" ) ) {
                savedPath = savedPath.mid( 1 );
            } else {
                savedPath = settings.value( "OpenFolderFrom" ).toString();
            }
            path = QFileDialog::getExistingDirectory( this, QString(), savedPath,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

            settings.setValue( "OpenFolderFrom", path );

            QSignalBlocker lock( ui->cameraSelectionCombo );
            if( ui->cameraSelectionCombo->count() == (int)camerasInfo.size() + 1 ) {
                ui->cameraSelectionCombo->insertItem( camerasInfo.size(), QFileInfo( path ).fileName(), QVariant( path ) );
            } else {
                ui->cameraSelectionCombo->setItemText( camerasInfo.size(), QFileInfo( path ).fileName() );
                ui->cameraSelectionCombo->setItemData( camerasInfo.size(), QVariant( path ) );
            }
            ui->cameraSelectionCombo->setCurrentIndex( camerasInfo.size() );
        }
        const static QString namedValue( "%1: <span style='color:#008800;'>%2</span><br>");

        QString txt;
        txt.append( namedValue.arg( "Path", path ) );

        ui->infoLabel->setText( txt );

        settings.setValue( "SelectedCamera", ":" + path );
    } else {
        auto info = camerasInfo[index];

        const static QString namedValue( "%1: <span style='color:#008800;'>%2</span><br>");

        QString txt;
        txt.append( namedValue.arg( "Bit depth", QString::number( info->BitDepth ) ) );
        txt.append( namedValue.arg( "Pixel size", QString::number( info->PixelSize ) ) );

        ui->infoLabel->setText( txt );

        settings.setValue( "SelectedCamera", info->Name );
    }

    currentImage.reset();
    ui->imageView->clear();
    ui->imageView->setText( "No image" );

    resetGraph();
}

void MainFrame::on_cameraOpenCloseButton_clicked()
{
    if( camera == 0 ) {
        auto data = ui->cameraSelectionCombo->currentData();
        if( data.type() == QVariant::String ) {
            setWindowTitle( openCamera( camerasInfo.size() )->Name );
        } else {
            setWindowTitle( openCamera( data.toInt() )->Name );
        }
    } else {
        closeCamera();
        setWindowTitle( "No camera" );
    }
    updateUI();
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
    guide( Hardware::GD_GUIDE_NORTH );
}

void MainFrame::on_guideDown()
{
    guide( Hardware::GD_GUIDE_SOUTH );
}

void MainFrame::on_guideLeft()
{
    guide( Hardware::GD_GUIDE_EAST );
}

void MainFrame::on_guideRight()
{
    guide( Hardware::GD_GUIDE_WEST );
}

void MainFrame::guide( Hardware::ST4_GUIDE_DIRECTION direction )
{
    if( guiding != -1 ) {
        camera->GuideOff( (Hardware::ST4_GUIDE_DIRECTION)guiding );
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
        camera->GuideOff( (Hardware::ST4_GUIDE_DIRECTION)guiding );
        guiding = -1;
    }
}

void MainFrame::on_captureButton_clicked()
{
    ui->captureButton->setEnabled( false );
    ui->saveToCheckBox->setEnabled( false );

    if( ui->saveToCheckBox->isChecked() ) {
        auto name = ui->objectNameEdit->text();
        settings.setValue( "Name", name );
        auto path = ui->saveToEdit->text();
        settings.setValue( "SaveTo", path );

        static const QString TIMESTAMP( "{TIME}" );
        auto timestamp = QDateTime::currentDateTime().toString("yyyy-MM-ddThh-mm-ss");
        if( path.contains( TIMESTAMP ) ) {
            path.replace( TIMESTAMP, "-" + timestamp );
        }
        static const QString FILTER( "{FILTER}" );
        if( path.contains( FILTER ) ) {
            auto filter = ui->filterComboBox->currentText();
            if( not filter.isEmpty() ) {
                path.replace( FILTER, "-" + filter );
            } else {
                path.replace( FILTER, "" );
            }
        }
        static const QString NAME( "{NAME}" );
        if( path.contains( NAME ) ) {
            if( not name.isEmpty() ) {
                path.replace( NAME, "-" + name );
            } else {
                path.replace( NAME, "" );
            }
        }
        path.replace( "\\-", "\\" );
        path.replace( "/-", "/" );
        saveToPath = path;
    } else {
        saveToPath.clear();
    }
    seriesId = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

    settings.setValue( "Exposure", ui->exposureSpinBox->value() * exposureSuffixToScale( ui->exposureSpinBox->suffix() ) );
    settings.setValue( "Gain", ui->gainSpinBox->value() );
    settings.setValue( "Offset", ui->offsetSpinBox->value() );
    settings.setValue( "UseCameraWhiteBalance", ui->useCameraWhiteBalanceCheckBox->isChecked() );

    startCapture();
}

std::shared_ptr<Hardware::CAMERA_INFO> MainFrame::openCamera( int index )
{
    auto start = std::chrono::steady_clock::now();

    std::shared_ptr<Hardware::CAMERA_INFO> cameraInfo;
    if( index < (int) camerasInfo.size() ) {
        cameraInfo = camerasInfo[index];
        camera = Hardware::Camera::Open( *cameraInfo );
    } else {
        auto str = ui->cameraSelectionCombo->currentData().toString();
        camera = Hardware::Camera::OpenFolder( str.toUtf8().constData() );
        cameraInfo = camera->GetInfo();
    }

    int width = 0;
    int height = 0;
    int bin = 0;
    Hardware::IMAGE_TYPE imgType = Hardware::IT_NONE;
    camera->GetROIFormat( width, height, bin, imgType );
    imgType = Hardware::IT_RAW16;
    // TO_DO: Quick fix for ZWO ASI294MM Pro in bin1
    if( width * height > 4048 * 4048 ) {
        bin = 2;
        width /= 2;
        height /= 2;
    }
    camera->SetROIFormat( width, height, bin, imgType );
    camera->GetROIFormat( width, height, bin, imgType );
    qDebug() << width << "x" << height << " bin" << bin;
    switch( imgType ) {
        case Hardware::IT_RAW8: qDebug() << "RAW8"; break;
        case Hardware::IT_RGB24: qDebug() << "RGB24"; break;
        case Hardware::IT_RAW16: qDebug() << "RAW16"; break;
        case Hardware::IT_Y8: qDebug() << "Y8"; break;
        default:
            assert( false );
    }

    //camera->PrintDebugInfo();

    auto end = std::chrono::steady_clock::now();
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
    qDebug() << "Camera initialized in" << msec << "msec";

    return cameraInfo;
}

void MainFrame::closeCamera()
{
    guideStop();
    camera->Close();
    camera = 0;
}

void MainFrame::startCapture()
{
    auto exposure = exposureFromValueAndSuffix( ui->exposureSpinBox->value(), ui->exposureSpinBox->suffix() );
    auto gain = ui->gainSpinBox->value();
    auto offset = ui->offsetSpinBox->value();
    auto useCameraWhiteBalance = ui->useCameraWhiteBalanceCheckBox->isChecked();

    camera->SetExposure( exposure );
    camera->SetGain( gain );
    camera->SetWhiteBalanceR( useCameraWhiteBalance ? 52 : 50 );
    camera->SetWhiteBalanceB( useCameraWhiteBalance ? 95 : 50 );
    camera->SetOffset( offset );

    ImageInfo imageInfo;
    if( filterWheel != 0 ) {
        auto channel = ui->filterComboBox->currentText();
        imageInfo.Channel = channel.toLocal8Bit().constData();
        auto fullFilterDescription = ui->filterComboBox->currentData( Qt::ToolTipRole ).toString();
        imageInfo.FilterDescription = fullFilterDescription.toLocal8Bit().constData();
    }
    imageInfo.SeriesId = seriesId;
    camera->SetImageInfoTemplate( imageInfo );

    auto info = camera->GetInfo();
    ui->ePerADULabel->setText( QString( "e<sup>-</sup>/ADU: <span style='color:#008800;'>%1</span>" ).arg( QString::number( info->ElectronsPerADU, 'f', 3 ) ) );

    imageReadyWatcher.setFuture( QtConcurrent::run( [=]() {
        auto start = std::chrono::steady_clock::now();

        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>( start.time_since_epoch() ).count();
        if( capturedFrames > 0 ) {
            qDebug() << "Since previous exposure " << timestamp - previousTimestamp << "msec";
            qDebug() << "FPS " << ( 1000.0 * capturedFrames ) / ( timestamp - startTimestamp );
        } else {
            startTimestamp = timestamp;
        }
        previousTimestamp = timestamp;

        auto result = camera->DoExposure();

        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
        qDebug() << "Captured in " << msec << "msec";

        return result;
    } ) );

    exposureRemainingTime = exposure / 1000000;
    exposureTimer.start( 1000 );
    showCaptureStatus();
}

void MainFrame::showCaptureStatus()
{
    ui->captureButton->setText( QString::number( exposureRemainingTime ) +
        ( capturedFrames > 0 ? " (" + QString::number( capturedFrames ) + ")" : "" ) );
}

QString MainFrame::formatImageInfo( const ImageInfo& info )
{
    const static QString namedValue( "%1: <span style='color:#008800;'>%2</span>%3<br>");

    QString txt;
    txt.append( namedValue.arg( "Size", QString::number( info.Width ) + "x" + QString::number( info.Height ), "" ) );
    txt.append( namedValue.arg( "BitDepth", QString::number( info.BitDepth ), "" ) );
    txt.append( namedValue.arg( "Exposure", exposureToString( info.Exposure ), "" ) );
    txt.append( namedValue.arg( "Gain", QString::number( info.Gain ), "" ) );
    txt.append( namedValue.arg( "Offset", QString::number( info.Offset ), "" ) );
    txt.append( namedValue.arg( "Temperature", QString::number( info.Temperature, 'f', 1 ), " <sup>0</sup>C" ) );
    if( not info.FilterDescription.empty() ) {
        txt.append( namedValue.arg( "Filter", QString( info.FilterDescription.c_str() ), "" ) );
    }
    txt.append( namedValue.arg( "Timestamp",
        QDateTime::fromSecsSinceEpoch( info.Timestamp ).toString("yyyy-MM-ddThh-mm-ss"), "" ) );
    txt.append( "<br>" );
    txt.append( namedValue.arg( "Saved As", QFileInfo( QString::fromStdString( info.FilePath ) ).fileName(), "" ) );
    return txt;
}

void MainFrame::imageReady()
{
    exposureTimer.stop();

    auto result = imageReadyWatcher.result();

    int currentIndex = capturedFrames.fetchAndAddOrdered( 1 );

    if( result != 0 ) {
        imageSavedWatcher.setFuture( QtConcurrent::run( [=]() {
            const auto& info = result->Info();
            if( saveToPath.length() > 0 ) {
                auto ext = ui->formatComboBox->currentText();
                settings.setValue( "FileFormat", ext );
                const ImageFileFormat* format = 0;
                if( ext == ".fits") {
                    const static FitsU16 fitsU16;
                    format = &fitsU16;
                } else if( ext == ".png") {
                    const static Png16BitGrayscale png16BitGrayscale;
                    format = &png16BitGrayscale;
                } else {
                    assert( ext == ".pixels" );
                }

                QDir().mkpath( saveToPath );
                const static QString nameTemplate( "%1.%2%3%4.u16%5" );
                auto name = nameTemplate.arg( QString::number( info.SeriesId, 16 ), QString::number( currentIndex ).rightJustified( 5, '0' ),
                    ( info.CFA.empty() ? "" : ".cfa" ), ( info.Channel.empty() ? "" : "." + info.Channel ).c_str(), ext );

                result->SaveToFile( ( saveToPath + QDir::separator() + name ).toLocal8Bit().constData(), format );
            }

            return formatImageInfo( info );

        } ) );

        currentImage = result;

        auto msec = render( result->RawPixels(), result->Width(), result->Height(), result->BitDepth() );
        qDebug() << "Rendered in " << msec << "msec";

        selectionStart = selectionEnd = -1;

        if( result->Info().Flags & IF_SERIES_START ) {
            resetGraph();
        }

        if( ui->graphCheckBox->isChecked() ) {

            if( graphs.isEmpty() ) {
                graphs.insert( "1:MEAN", GraphData( "mean", QColor::fromRgb( 0, 0xA0, 0 ), 2 ) );
                graphs.insert( "2:SIGMA", GraphData( u8"σ", QColor::fromRgb( 0xA0, 0, 0 ), 2 ) );
                graphs.insert( "3:T", GraphData( "T", QColor::fromRgb( 0xA0, 0xA0, 0xA0 ), 2 ) );
                if( ui->graphsTypeComboBox->currentText() == "DARKS" ) {
                    graphs.insert( "calibrated_mean", GraphData( "CMEAN", QColor::fromRgb( 0, 0, 0xA0 ), 2 ) );
                    graphs.insert( "calibrated_sigma", GraphData( "CSIGMA", QColor::fromRgb( 0, 0xA0, 0xA0 ), 2 ) );
                    graphs.insert( "calibrated_delta", GraphData( "CDELTA", QColor::fromRgb( 0xA0, 0xA0, 0 ), 2 ) );
                }
            }
            graphImageInfo.append( currentImage->Info() );

            //auto h = pixels_histogram( result->RawPixels(), result->Count(), result->BitDepth() );
            //auto value = pixels_histogram_median( h, 0 );

            /*CRawU16 raw16( result->RawPixels(), result->Width(), result->Height(), result->BitDepth() );
            auto stars = raw16.DetectStars( 0, 0, result->Width(), result->Height() );

            double flux = 1.0;
            for( auto detection : stars.DetectionRegions ) {
                flux += detection->FluxAtHalfDetectionThreshold;
            }*/

            auto [mean, sigma, min, max] = simple_pixel_statistics( result->RawPixels(), result->Count() );
            graphs.find( "1:MEAN" )->Values.emplace_back( mean );
            graphs.find( "2:SIGMA" )->Values.emplace_back( sigma );
            graphs.find( "3:T" )->Values.emplace_back( result->Info().Temperature );

            if( ui->graphsTypeComboBox->currentText() == "DARKS" ) {
                std::shared_ptr<void> data = graphs.find( "calibrated_mean" )->Data;
                std::shared_ptr<CPixelBuffer<uint16_t>> ref;
                if( !data ) {
                    ref = std::make_shared<CPixelBuffer<uint16_t>>( currentImage->Width(), currentImage->Height() );
                    pixels_set( ref->Pixels(), currentImage->Pixels(), ref->Count() );
                    graphs.find( "calibrated_mean" )->Data = ref;
                } else {
                    ref = std::static_pointer_cast<CPixelBuffer<uint16_t>>( data );
                }
                CPixelBuffer<double> diff( currentImage->Width(), currentImage->Height() );
                pixels_set( diff.Pixels(), currentImage->Pixels(), diff.Count() );
                pixels_add_value( diff.Pixels(), mean, diff.Count() );
                pixels_subtract( diff.Pixels(), ref->Pixels(), diff.Count() );

                auto [mean, sigma, min, max] = simple_pixel_statistics( diff.Pixels(),  diff.Count() );
                graphs.find( "calibrated_mean" )->Values.emplace_back( mean );
                graphs.find( "calibrated_sigma" )->Values.emplace_back( sigma );
                graphs.find( "calibrated_delta" )->Values.emplace_back( fabs( max - min ) );

                CPixelBuffer<uint16_t> result( currentImage->Width(), currentImage->Height() );
                pixels_set_round_limit( result.Pixels(), diff.Pixels(), diff.Count(), currentImage->BitDepth() );

                render( result.Pixels(), result.Width(), result.Height(), currentImage->BitDepth() );
            }

            ui->imageSeriesView->setVisible( true );
            ui->imageSeriesView->setOnPaint( [this]( PaintView* view, QPainter& painter ) {

                QPen pen1( QColor::fromRgb( 0x30, 0x30, 0x30 ) );
                pen1.setWidth( 1 );

                painter.setPen( pen1 );
                painter.drawLine( 0, 0, view->width(), 0 );
                painter.drawLine( 0, 100 - graphScaleY, view->width(), 100 - graphScaleY );
                painter.drawLine( 0, 100, view->width(), 100  );
                painter.drawLine( 0, 100 + graphScaleY, view->width(), 100 + graphScaleY  );

                if( graphs.size() == 0 ) {
                    return;
                }

                int length = graphs.first().Values.size();

                int widthX = view->width() / graphScaleX;
                if( selectionStart == -1 ) {
                    if( length > widthX ) {
                        scrollX = length - widthX - 1;
                    }
                } else {
                    if( selectionStart < scrollX ) {
                        scrollX = std::max( 0, selectionStart - 10 );
                    } else if( selectionStart > scrollX + widthX ) {
                        scrollX = std::max( 0, selectionStart - widthX + 10 );
                    }
                }

                QPen pen3( view->hasFocus() ?
                QColor::fromRgb( 0x00, 0x33, 0x66 ) : QColor::fromRgb( 0x33, 0x33, 0x33 ) );
                pen3.setWidth( 3 );

                for( int i = 0; i < length; i++ ) {
                    if( i == selectionStart ) {
                        painter.setPen( pen3 );
                    }
                    if( i >= scrollX ) {
                        int x = ( i - scrollX ) * graphScaleX;
                        if( i == selectionStart ) {
                            //painter.drawText( x + 5, 195, QFileInfo( QString::fromStdString( graphImageInfo[i].FilePath ) ).fileName() );
                        }
                        painter.drawLine( x, 0, x, 200 );
                    }
                    if( i == selectionEnd ) {
                        painter.setPen( pen1 );
                    }
                }

                if( length > 0 ) {

                    QFont font( "Consolas" );
                    font.setPointSizeF( 6.5 );
                    font.setBold( true );
                    painter.setFont( font );

                    QPen pen4( QColor::fromRgb( 0xA0, 0xA0, 0xA) );
                    painter.setPen( pen4 );

                    painter.drawText( 5, 10, QString::number( selectionStart >= 0 ? selectionStart + 1 : length ) );

                    painter.setRenderHint( QPainter::Antialiasing );

                    int n = 0;
                    for( const auto& graph : graphs ) {
                        const auto& data = graph.Values;

                        std::vector<double> values( length );
                        for( int i = 0; i < length; i++ ) {
                            values[i] = data[i];
                        }
                        std::sort( values.begin(), values.end() );
                        double median = values[length / 2];
                        for( int i = 0; i < length; i++ ) {
                            values[i] = fabs( median - data[i] );
                        }
                        std::sort( values.begin(), values.end() );
                        double MAE = values[length / 2];
                        if( MAE == 0 ) {
                            MAE = 1;
                        }
                        double min = std::numeric_limits<double>().max();
                        double max = std::numeric_limits<double>().min();
                        for( auto val: graph.Values ) {
                            if( val < min ) {
                                min = val;
                            }
                            if( val > max ) {
                                max = val;
                            }
                        }

                        for( int i = scrollX; i < length; i++ ) {
                            if( i > scrollX ) {
                                painter.setPen( graph.Pen );
                                double prev = graphScaleY * ( data[i - 1] - median ) / MAE;
                                double cur = graphScaleY * ( data[i] - median ) / MAE;
                                painter.drawLine(
                                    ( i - 1 - scrollX) * graphScaleX, 100 - round( prev ),
                                    ( i - scrollX ) * graphScaleX, 100 - round( cur ) );
                            }
                        }

                        double val = selectionStart != -1 ? graph.Values[selectionStart] : graph.Values.back();
                        painter.drawText( 5, 20 + 10 * n, QString( "%1 %2 (%3 %4)" )
                            .arg( graph.Name ).arg( val, 0, 'f', 1 ).arg( min, 0, 'f', 1 ).arg( max, 0, 'f', 1 ) );
                        n++;
                    }
                }
            } );

            ui->imageSeriesView->setOnMousePressed( [this]( PaintView* view, QMouseEvent* event ) {
                if( event->button() == Qt::LeftButton ) {
                    int length = graphs.first().Values.size();
                    int pos = std::min( (int)round( event->x() / (double)graphScaleX ) + scrollX, length - 1 );
                    if( event->modifiers() == Qt::ShiftModifier ) {
                        if( selectionStart == -1 ) {
                            selectionStart = selectionEnd = 0;
                        }
                        if( pos > selectionEnd ) {
                            selectionEnd = pos;
                        } else {
                            selectionStart = pos;
                        }
                    } else {
                        selectionStart = selectionEnd = pos;;
                    }
                    view->update();

                    currentImage = CRawU16Image::LoadFromFile( graphImageInfo[selectionStart].FilePath.c_str() );
                    render( currentImage->RawPixels(), currentImage->Width(), currentImage->Height(), currentImage->BitDepth() );
                    ui->infoLabel->setText( formatImageInfo( currentImage->Info() ) );
                }
            } );

            ui->imageSeriesView->setOnKeyPressed( [this]( PaintView* view, QKeyEvent* event) {

                if( graphs.isEmpty() ) {
                    return;
                }

                bool shouldUpdate = false;
                bool moveCaretKey = false;
                int pos = -1;

                if( event->key() == Qt::Key_Delete ) {
                    auto result = QMessageBox::question( this, "Delete frames", "Move selected frames to [TRASH] subfolder?",
                                                         QMessageBox::Yes|QMessageBox::No );
                    if( result == QMessageBox::Yes ) {
                        auto rootDir =  QFileInfo( QString::fromLocal8Bit( graphImageInfo[selectionStart].FilePath.c_str() ) ).absoluteDir();
                        QDir trashDir = QDir( rootDir.absolutePath() + QDir::separator() + "[TRASH]" );
                        if( !trashDir.exists() ) {
                            if( !trashDir.mkdir( "." ) ) {
                                QMessageBox::critical( this, "Could not create [TRASH] folder", trashDir.absolutePath() );
                                return;
                            }
                        }
                        for( int i = selectionEnd; i >= selectionStart; i-- ) {
                            auto path = QString::fromLocal8Bit( graphImageInfo[i].FilePath.c_str() );
                            auto file = QFile( path );
                            auto newPath = trashDir.absolutePath() + QDir::separator() + QFileInfo( file.fileName() ).fileName();
                            if( !file.rename( newPath ) ) {
                                QMessageBox::critical( this, "Error", file.errorString() );
                                selectionStart = i + 1;
                                break;
                            }
                            int dotPos = path.lastIndexOf( '.' );
                            auto infoFile = QFile( path.mid( 0, dotPos + 1 ) + "info" );
                            newPath = trashDir.absolutePath() + QDir::separator() + QFileInfo( infoFile.fileName() ).fileName();
                            if( !infoFile.rename( newPath ) ) {
                                // It is less critical if the .info file is not moved
                                QMessageBox::critical( this, "Error", file.errorString() );
                                break;
                            }
                        }
                        if( selectionEnd >= selectionStart ) {
                            graphImageInfo.erase( graphImageInfo.begin() + selectionStart, graphImageInfo.begin() + selectionEnd + 1 );
                            for( auto& graph : graphs ) {
                                auto& values = graph.Values;
                                values.erase( values.begin() + selectionStart, values.begin() + selectionEnd + 1 );
                            }
                            pos = selectionStart - 1;
                            shouldUpdate = true;
                        } else {
                            selectionStart = selectionEnd;
                        }
                    }
                }
                // Length after modifications
                int length = graphs.first().Values.size();
                if( event->key() == Qt::Key_Left ) {
                    pos = selectionStart - 1;
                    moveCaretKey = true;
                    shouldUpdate = true;
                } else if( event->key() == Qt::Key_Right ) {
                    pos = selectionEnd + 1;
                    moveCaretKey = true;
                    shouldUpdate = true;
                } else if( event->key() == Qt::Key_Home ) {
                    pos = 0;
                    moveCaretKey = true;
                    shouldUpdate = true;
                } else if( event->key() == Qt::Key_End ) {
                    pos = length - 1;
                    moveCaretKey = true;
                    shouldUpdate = true;
                }
                if( shouldUpdate ) {
                    if( pos != -1 ) {
                        pos = length > 0 ? std::max( 0, std::min( pos, length - 1 ) ) : -1;
                        if( moveCaretKey && event->modifiers() == Qt::ShiftModifier && pos >= 0 ) {
                            if( pos < selectionStart ) {
                                selectionStart = pos;
                            }
                            if( pos > selectionEnd ) {
                                selectionEnd = pos;
                            }
                        } else {
                            selectionStart = selectionEnd = pos;
                        }
                        if( selectionStart >= 0 ) {
                            currentImage = CRawU16Image::LoadFromFile( graphImageInfo[selectionStart].FilePath.c_str() );
                            render( currentImage->RawPixels(), currentImage->Width(), currentImage->Height(), currentImage->BitDepth() );
                            ui->infoLabel->setText( formatImageInfo( currentImage->Info() ) );
                        }
                    }
                    view->update();
                }

            } );

            ui->imageSeriesView->setOnWheel( [this]( PaintView* view, QWheelEvent* event ) {
                if( event->angleDelta().y() < 0 ) {
                    if( event->modifiers() == Qt::ShiftModifier ) {
                        if( graphScaleX > 1 ) {
                            graphScaleX--;
                        }
                    } else if( graphScaleY > 1 ) {
                        graphScaleY--;
                    }
                    view->update();
                } else if( event->angleDelta().y() > 0 ) {
                    if( event->modifiers() == Qt::ShiftModifier ) {
                        if( graphScaleX < 10 ) {
                            graphScaleX++;
                        }
                    } else if( graphScaleY < 10 ) {
                        graphScaleY++;
                    }
                    view->update();
                }
            } );
        }
    }

    if( result->Info().Flags & IF_SERIES_END ) {
        ui->continuousCaptureCheckBox->setChecked( false );
    }

    if( ui->continuousCaptureCheckBox->isChecked() ) {
        ui->continuousCaptureCheckBox->setText( "Continuous capture (uncheck to stop)" );
        startCapture();
    } else {
        capturedFrames = 0;
        ui->continuousCaptureCheckBox->setText( "Continuous capture" );
        ui->captureButton->setText( "Capture" );
        ui->captureButton->setEnabled( true );
        ui->saveToCheckBox->setEnabled( true );
    }
}

void MainFrame::resetGraph()
{
    graphs.clear();
    graphImageInfo.clear();
    selectionStart = selectionEnd = -1;
}

void MainFrame::imageSaved()
{
    ui->infoLabel->setText( imageSavedWatcher.result() );
}

ulong MainFrame::render( const ushort* raw, int width, int height, int bitDepth )
{
    auto start = std::chrono::steady_clock::now();

    if( zoom > 0 ) {
        showZoom();
    }
    if( !ui->renderOffCheckBox->isChecked() ) {
        QPixmap pixmap;
        if( ui->stretchCheckBox->isChecked() ) {
            CRawU16 rawU16( raw, width, height, bitDepth );
            if( ui->showQuarterResolution->isChecked() ) {
                pixmap = Qt::CreatePixmap( rawU16.StretchQuarterRes( 0, 0, width, height ) );
            } else if( ui->showFullResolution->isChecked() ) {
                pixmap = Qt::CreatePixmap( rawU16.Stretch( 0, 0, width, height ) );
            } else {
                pixmap = Qt::CreatePixmap( rawU16.StretchHalfRes( 0, 0, width, height ) );
            }
        } else {
            Renderer renderer( raw, width, height, bitDepth );
            if( ui->showQuarterResolution->isChecked() ) {
                pixmap = renderer.Render( RM_QuarterResolution );
            } else if( ui->showFullResolution->isChecked() ) {
                pixmap = renderer.Render(  RM_FullResolution );
            } else {
                pixmap = renderer.Render( RM_HalfResolution );
            }
            ui->histogramView->setPixmap( renderer.RenderHistogram() );
        }
        tools.Draw( pixmap );
        ui->imageView->setPixmap( pixmap );
    } else {
        ui->imageView->clear();
    }

    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>( end - start ).count();
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

QString MainFrame::exposureToString( int exposure )
{
    QString suffix;
    int scale = exposureToScaleAndSuffix( exposure, suffix );
    return QString::number( exposure / scale ) + suffix;
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

void MainFrame::on_filterWheelComboBox_currentIndexChanged( int index )
{
    auto name = ui->filterWheelComboBox->itemText( index );
    auto fileName = name + ".FilterWheel";

    QStringList slotNames;
    if( QFileInfo::exists( fileName ) ) {
        QFile inputFile( fileName );
        if( inputFile.open( QIODevice::ReadOnly ) ) {
           QTextStream in( &inputFile );
           while( !in.atEnd() ) {
              slotNames.append( in.readLine() );
           }
           inputFile.close();
        }
        settings.setValue( "FilterWheel", name );
    }

    QSignalBlocker lock( ui->filterComboBox );
    ui->filterComboBox->clear();
    for( int i = 0; i < filterWheel->GetSlotsCount(); i++ ) {
        QString name;
        QString description;
        if( i < slotNames.size() ) {
            QString slot = slotNames.at( i );
            name = slot.section( " ", 0, 0 );
            description = slot.section( " ", 1 );
        } else {
            name = QString::number( i + 1 );
        }
        ui->filterComboBox->addItem( name );
        ui->filterComboBox->setItemData( i, description, Qt::ToolTipRole );

    }
    ui->filterComboBox->setCurrentIndex( filterWheel->GetPosition() );
}

void MainFrame::on_imageView_imagePressed( int cx, int cy, Qt::MouseButton, Qt::KeyboardModifiers modifiers )
{
    int scale = 2;
    if( ui->showFullResolution->isChecked() ) {
        scale = 1;
    } else if( ui->showQuarterResolution->isChecked() ) {
        scale = 4;
    }
    zoomCenter.setX( cx * scale );
    zoomCenter.setY( cy * scale );

    if( modifiers.testFlag( Qt::ControlModifier ) ) {
        auto focusingHelperTool = tools.TryGet<Tools::FocusingHelper>();
        if( focusingHelperTool && modifiers.testFlag( Qt::AltModifier ) ) {
            // Add additional star to existing helper
            focusingHelperTool->getFocusingHelper()->addExtra( zoomCenter.x(), zoomCenter.y() );
        } else {
            // Switch the focusing helper on
            tools.New<Tools::FocusingHelper>( zoomCenter.x(), zoomCenter.y() );
        }
    } else if( zoomView->isHidden() ) {
        // Normally the zoom view must open in normal mode
        tools.Delete<Tools::FocusingHelper>();
    }

    showZoom();

    if( modifiers.testFlag( Qt::ShiftModifier ) ) {
        // Move mouse pointer to the center of zoom view
        auto center = zoomView->contentsRect().center();
        QCursor::setPos( zoomView->mapToGlobal( center ) );
    }
}

void MainFrame::on_graphCheckBox_toggled( bool checked )
{
    ui->imageSeriesView->setVisible( checked );
    if( zoomView->isVisible() ) {
        QCoreApplication::processEvents();
        zoomRecalcLayout();
    }
}
