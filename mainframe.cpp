#include "mainframe.h"
#include "ui_mainframe.h"
#include <QShortcut>
#include <QDebug>

#include <ASICamera2.h>

#include <chrono>

MainFrame::MainFrame(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainFrame)
{
    ui->setupUi( this ) ;

    ui->imageView->setBackgroundRole( QPalette::Base );
    ui->imageView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->imageView->setScaledContents( false );
    ui->imageView->setAlignment( Qt::AlignCenter );

    ui->histogramView->setBackgroundRole( QPalette::Base );
    ui->histogramView->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
    ui->histogramView->setScaledContents( false );
    ui->histogramView->setAlignment( Qt::AlignCenter );

    new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_F ), this, SLOT( on_toggleFullScreenButton_clicked() ) );
}

MainFrame::~MainFrame()
{
    delete ui;
}

void MainFrame::on_toggleFullScreenButton_clicked()
{
    bool isFullScreen = windowState().testFlag( Qt::WindowFullScreen );
    if( isFullScreen ) {
        showNormal();
    } else {
        showFullScreen();
    }
}

void MainFrame::on_captureFrameButton_clicked()
{
    int count = ASIGetNumOfConnectedCameras();
    if( count > 0 ) {
        ASI_CAMERA_INFO cameraInfo;
        ASIGetCameraProperty( &cameraInfo, 0 );
        printf( "%s\n", cameraInfo.Name );

        int cameraID = cameraInfo.CameraID;
        ASIOpenCamera( cameraID );
        ASIInitCamera( cameraID );

        long exposure = 100000;
        ASI_BOOL isAutoExposure = ASI_FALSE;
        ASISetControlValue( cameraID, ASI_EXPOSURE, exposure, isAutoExposure );
        exposure = 0;
        ASIGetControlValue( cameraID, ASI_EXPOSURE, &exposure, &isAutoExposure );
        printf( "Exposure %ld %s\n", exposure, isAutoExposure == ASI_TRUE ? "(auto)" : "" );

        long gain = 0;
        ASI_BOOL isAutoGain = ASI_FALSE;
        ASISetControlValue( cameraID, ASI_GAIN, gain, isAutoGain );
        ASIGetControlValue( cameraID, ASI_GAIN, &gain, &isAutoGain );
        printf( "Gain %ld %s\n", gain, isAutoGain == ASI_TRUE ? "(auto)" : "" );

        int width = 0;
        int height = 0;
        int bin = 0;
        ASI_IMG_TYPE imgType = ASI_IMG_END;
        ASIGetROIFormat( cameraID, &width, &height, &bin, &imgType );
        imgType = ASI_IMG_RAW16;
        ASISetROIFormat( cameraID, width, height, bin, imgType );
        printf( "%dx%d bin%d ", width, height, bin );
        switch( imgType ) {
            case ASI_IMG_RAW8: printf( "RAW8\n" ); break;
            case ASI_IMG_RGB24: printf( "RGB24\n" ); break;
            case ASI_IMG_RAW16: printf( "RAW16\n" ); break;
            case ASI_IMG_Y8: printf( "Y8\n" ); break;
        }

        ASIStartExposure( cameraID, ASI_FALSE );
        bool capture = true;
        do {
            ASI_EXPOSURE_STATUS status;
            ASIGetExpStatus( cameraID, &status );
            switch( status ) {
                case ASI_EXP_SUCCESS: printf( "OK\n" ); capture = false; break;
                case ASI_EXP_FAILED: printf( "Failed\n" ); capture = false; break;
            }
        } while( capture );
        std::vector<ushort> buf( width * height );
        size_t size = buf.size() * sizeof( ushort );
        ASIGetDataAfterExp( cameraID, (unsigned char*)buf.data(), size );
        ASICloseCamera( cameraID );

        FILE* out = fopen( "image.cfa", "wb" );
        fwrite( buf.data(), 1, size, out );
        fclose( out );

        render( buf.data(), width, height );

        printf( "DONE\n" );
    } else {
        qDebug() << "No camera";

        FILE* file = fopen( "image.cfa", "rb" );
        int width = 1936;
        int height = 1096;
        std::vector<ushort> raw( width * height );
        fread( raw.data(), sizeof( ushort ), width * height, file );
        fclose( file );

        auto msec = render( raw.data(), width, height );

        qDebug() << msec << "msec";
    }
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
    ui->imageView->setPixmap( QPixmap::fromImage( image ) );

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
