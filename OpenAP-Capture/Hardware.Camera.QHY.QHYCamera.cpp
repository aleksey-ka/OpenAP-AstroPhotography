// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "Hardware.Camera.QHY.QHYCamera.h"

#include <QDebug>
#include <QThread>

namespace {
    class QHYInit {
    public:
        QHYInit() { InitQHYCCDResource(); };
        ~QHYInit() { ReleaseQHYCCDResource(); }
    };
}

int QHYCamera::GetCount()
{
    static QHYInit init;
    return ScanQHYCCD();
}

std::shared_ptr<Hardware::CAMERA_INFO> QHYCamera::createCameraInfo( const char* id )
{
    auto result = std::make_shared<Hardware::CAMERA_INFO>();
    strncpy( result->GUID, id, sizeof( result->GUID ) );
    checkResult( GetQHYCCDModel( result->GUID, result->Name ) );
    result->IsColorCamera = true;
    /*result->BayerPattern = convert( cameraInfo.BayerPattern );
    result->BitDepth = cameraInfo.BitDepth;
    result->PixelSize = cameraInfo.PixelSize;
    result->ElectronsPerADU = cameraInfo.ElecPerADU;*/

    return result;
}

std::shared_ptr<Hardware::CAMERA_INFO> QHYCamera::GetInfo( int index )
{
    char id[64];
    checkResult( GetQHYCCDId( index, id ) );
    return createCameraInfo( id );
}

std::shared_ptr<QHYCamera> QHYCamera::Open( const Hardware::CAMERA_INFO& cameraInfo )
{
    return std::make_shared<QHYCamera>( cameraInfo.GUID );
}

void QHYCamera::Close()
{
    if( handle != 0 ) {
        isClosing = true;
        while( isExposure );
        checkResult( CloseQHYCCD( handle ) );
        handle = 0;
    }
}

std::shared_ptr<Hardware::CAMERA_INFO> QHYCamera::GetInfo() const
{
    return cameraInfo;
}

QHYCamera::QHYCamera( const char* id )
{
    handle = OpenQHYCCD( (char*)id );
    cameraInfo = createCameraInfo( id );

    uint32_t readModes = -1;
    checkResult( GetQHYCCDNumberOfReadModes( handle, &readModes ) );
    checkResult( SetQHYCCDReadMode( handle, 0 ) );
    qDebug() << "Read modes" << readModes;
    checkResult( InitQHYCCD( handle ) );

    /*assert( id != -1 );

    checkResult( ASIOpenCamera( id ) );
    checkResult( ASIInitCamera( id ) );
    checkResult( ASIDisableDarkSubtract( id ) );
    checkResult( ASISetControlValue( id, ASI_BANDWIDTHOVERLOAD, 95, ASI_FALSE ) );*/

    /*int numOfControls = 0;
    checkResult( ASIGetNumOfControls( id, &numOfControls ) );
    for( int i = 0; i < numOfControls; i++ ) {
        ASI_CONTROL_CAPS controlCaps;
        checkResult( ASIGetControlCaps( id, i, &controlCaps) );
        qDebug() << controlCaps.Name;
    }*/
}

QHYCamera::~QHYCamera()
{
    try {
        QHYCamera::Close();
    } catch( std::exception& e ) {
       qDebug() << "Error: " << e.what();
    }
}

long QHYCamera::GetExposure( bool& isAuto ) const
{
    getControlValue( CONTROL_EXPOSURE, exposure, isAuto );
    return exposure;
}

void QHYCamera::SetExposure( long value, bool isAuto )
{
    setControlValue( CONTROL_EXPOSURE, value, isAuto );
    exposure = -1; //cameraInfo = 0;
}

void QHYCamera::GetExposureCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( CONTROL_EXPOSURE, min, max, defaultVal );
}

long QHYCamera::GetGain( bool& isAuto ) const
{
    getControlValue( CONTROL_GAIN, gain, isAuto );
    return gain;
}

void QHYCamera::SetGain( long value, bool isAuto )
{
    setControlValue( CONTROL_GAIN, value, isAuto );
    gain = -1; //cameraInfo = 0;
}

void QHYCamera::GetGainCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( CONTROL_GAIN, min, max, defaultVal );
}

long QHYCamera::GetOffset() const
{
    bool isAuto;
    getControlValue( CONTROL_OFFSET, offset, isAuto );
    return offset;
}

void QHYCamera::SetOffset( long value )
{
    setControlValue( CONTROL_OFFSET, value, false );
    offset = -1; //cameraInfo = 0;
}

void QHYCamera::GetOffsetCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( CONTROL_OFFSET, min, max, defaultVal );
}

long QHYCamera::GetWhiteBalanceR() const
{
    //checkResult( ASIGetControlValue( id, ASI_WB_R, &WB_R, &isAutoWB_R ) );
    return WB_R;
}

void QHYCamera::SetWhiteBalanceR( long value, bool isAuto )
{
    //checkResult( ASISetControlValue( id, ASI_WB_R, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
    //WB_R = -1; cameraInfo = 0;
}

void QHYCamera::GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const
{
    //getControlCaps( ASI_WB_R, min, max, defaultVal );
}

long QHYCamera::GetWhiteBalanceB() const
{
    //checkResult( ASIGetControlValue( id, ASI_WB_B, &WB_B, &isAutoWB_B ) );
    return WB_B;
}

void QHYCamera::SetWhiteBalanceB( long value, bool isAuto )
{
    //checkResult( ASISetControlValue( id, ASI_WB_B, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
    //WB_B = -1; cameraInfo = 0;
}

void QHYCamera::GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const
{
    //getControlCaps( ASI_WB_B, min, max, defaultVal );
}

void QHYCamera::GetROIFormat( int& _width, int& _height, int& _bin, Hardware::IMAGE_TYPE& _imgType ) const
{
    /*lazyROIFormat();

    _width = width;
    _height = height;
    _bin = bin;
    _imgType = convert( imgType );*/
}

void QHYCamera::lazyROIFormat() const
{
    /*if( imgType == ASI_IMG_END ) {
        checkResult( ASIGetROIFormat( id, &width, &height, &bin, &imgType ) );
    }*/
}

void QHYCamera::SetROIFormat( int width, int height, int bin, Hardware::IMAGE_TYPE imgType )
{
    //checkResult( ASISetROIFormat( id, width, height, bin, convert( imgType ) ) );
    //this->imgType = ASI_IMG_END; cameraInfo = 0;
}

std::shared_ptr<const CRawU16Image> QHYCamera::DoExposure() const
{
    lazyROIFormat();

    ImageInfo imageInfo( imageInfoTemplate );

    /*auto cameraInfo = GetInfo();
    imageInfo.Camera = cameraInfo->Name;
    if( cameraInfo->IsColorCamera ) {
        assert( cameraInfo->BayerPattern == Hardware::BP_BAYER_RG );
        imageInfo.CFA = "RGGB";
    }*/

    imageInfo.Width = width;
    imageInfo.Height = height;

    // With binning (ie summing up the pixel values) the bit depth effectively increases
    imageInfo.BitDepth = 12;//cameraInfo->BitDepth;
    switch( bin ) {
        case 1: break;
        case 2: imageInfo.BitDepth += 2; break;
        case 4: imageInfo.BitDepth += 4; break;
    }
    imageInfo.BitDepth = std::min( 16, imageInfo.BitDepth );

    bool isAuto;
    imageInfo.Exposure = exposure != -1 ? exposure : GetExposure( isAuto );
    imageInfo.Gain = gain != -1 ? gain : GetGain( isAuto );
    imageInfo.Offset = offset != -1 ? offset : GetOffset();
    imageInfo.Temperature = GetCurrentTemperature();

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t( now );
    imageInfo.Timestamp = timestamp;

    //uint32_t w = 1920, h = 1080;
    uint32_t w = 3864, h = 2180;
    imageInfo.Width = w;
    imageInfo.Height = h;
    checkResult( SetQHYCCDResolution( handle, 0, 0, w, h ) );

    checkResult( ExpQHYCCDSingleFrame( handle ) );
    auto memLength = GetQHYCCDMemLength( handle );

    auto result = std::make_shared<CRawU16Image>( imageInfo );
    //assert( result->BufferSize() >= memLength );

    uint8_t* buf = new uint8_t[memLength];

    uint32_t bpp = 16, ch = 0;
    checkResult( GetQHYCCDSingleFrame( handle, &w, &h, &bpp, &ch, buf ) );
    memcpy( result->Buffer(), buf, result->BufferSize() );

    auto pixels = result->RawPixels();
    size_t count = result->Count();
    unsigned short* src = (unsigned short*)buf;
    for( size_t i = 0; i < count; i++ ) pixels[i] = ( src[i] >> 4 );
    delete[] buf;

    return result;

    /*isExposure = true;
    if( isClosing ) {
        isExposure = false;
        return 0;
    }

    checkResult( ASIStartExposure( id, ASI_FALSE ) );

    auto result = std::make_shared<CRawU16Image>( imageInfo );

    ASI_EXPOSURE_STATUS status;
    do {
        // This delay will significantly reduce the load on CPU, and has no effect
        // on FPS in this mode, as the status is still updated 1000 times/s
        QThread::usleep( 1000 );
        checkResult( ASIGetExpStatus( id, &status ) );
        switch( status ) {
            case ASI_EXP_WORKING:
                if( isClosing ) {
                    checkResult( ASIStopExposure( id ) );
                }
                continue;
            case ASI_EXP_SUCCESS:
                if( isClosing ) {
                    isExposure = false;
                    return 0;
                } else {
                    checkResult( ASIGetDataAfterExp( id, result->Buffer(), result->BufferSize() ) );
                    auto pixels = result->RawPixels();
                    size_t count = result->Count();
                    switch( imageInfo.BitDepth ) {
                        case 16: break;
                        case 14: for( size_t i = 0; i < count; i++ ) pixels[i] >>= 2; break;
                        case 12: for( size_t i = 0; i < count; i++ ) pixels[i] >>= 4; break;
                        default:
                            assert( false );
                    }

                    // Mini histogram to evaluate the effective bit depth of the result
                    /*const size_t lengthOfH = 16;
                    uint64_t h[lengthOfH];
                    for( size_t i = 0; i < lengthOfH; i++ ) {
                        h[i] = 0;
                    }
                    for( size_t i = 0; i < count; i++ ) {
                        auto val = pixels[i];
                        h[ val % lengthOfH]++;
                    }
                    for( size_t i = 0; i < lengthOfH; i++ ) {
                        qDebug() << h[i];
                    }*/

                    /*isExposure = false;
                    return result;
                }
            case ASI_EXP_FAILED:
                isExposure = false;
                return 0;
            default:
                assert( false );
        }
    } while( true );*/

    return 0;
}

int QHYCamera::GetDroppedFrames() const
{
    int dropped = 0;
    //checkResult( ASIGetDroppedFrames( id, &dropped ) );
    return dropped;
}

double QHYCamera::GetCurrentTemperature() const
{
    long temperature = 0;
    //ASI_BOOL isAuto = ASI_FALSE;
    //checkResult( ASIGetControlValue( id, ASI_TEMPERATURE, &temperature, &isAuto ) );
    return static_cast<double>( temperature ) / 10.0;
}

bool QHYCamera::HasCooler() const
{
    //return hasControlCaps( ASI_COOLER_ON );
    return false;
}

bool QHYCamera::IsCoolerOn() const
{
    /*if( coolerOn == -1 ) {
        ASI_BOOL isAuto = ASI_FALSE;
        checkResult( ASIGetControlValue( id, ASI_COOLER_ON, &coolerOn, &isAuto ) );
    }
    return coolerOn == 1;*/
    return false;
}

void QHYCamera::SetCoolerOn( bool value )
{
    /*if( coolerOn != value ) {
        checkResult( ASISetControlValue( id, ASI_COOLER_ON, value ? 1 : 0, ASI_FALSE ) );
        coolerOn = value;
    }*/
}

double QHYCamera::GetTargetTemperature() const
{
    /*if( targetTemperature == -1 ) {
        ASI_BOOL isAuto = ASI_FALSE;
        checkResult( ASIGetControlValue( id, ASI_TARGET_TEMP, &targetTemperature, &isAuto ) );
    }*/
    return targetTemperature;
}

void QHYCamera::SetTargetTemperature( double temperature )
{
    /*if( targetTemperature != temperature ) {
        checkResult( ASISetControlValue( id, ASI_TARGET_TEMP, (long)temperature, ASI_FALSE ) );
        targetTemperature = temperature;
    }*/
}

void QHYCamera::GuideOn( Hardware::ST4_GUIDE_DIRECTION direction ) const
{
    //checkResult( ASIPulseGuideOn( id, convert( direction ) ) );
}

void QHYCamera::GuideOff( Hardware::ST4_GUIDE_DIRECTION direction ) const
{
    //checkResult( ASIPulseGuideOff( id, convert( direction ) ) );
}

void QHYCamera::PrintDebugInfo()
{
    /*qDebug() << "ASI SDK" << ASIGetSDKVersion();

    lazyControlCaps();

    for( auto cap : controlCaps ) {
        qDebug() << cap.Name << cap.Description;
        qDebug() << "[" << cap.MinValue << cap.MaxValue << cap.DefaultValue <<
            ( cap.IsAutoSupported ? "auto" : "" ) << ( cap.IsWritable ? "rw" : "r" ) << "]";
    }*/
}

void QHYCamera::lazyControlCaps() const
{
    if( controlCaps.size() == 0 ) {
        for( int i = 0; i < CONTROL_MAX_ID; i++ ) {
            CONTROL_ID id = (CONTROL_ID)i;
            if( IsQHYCCDControlAvailable( handle, id ) ) {
                double minv, maxv, step;
                checkResult( GetQHYCCDParamMinMaxStep( handle, id, &minv, &maxv, &step ) );
                controlCaps.emplace_back( minv, maxv, step );
            }
        }
    }
}

bool QHYCamera::hasControlCaps( CONTROL_ID controlType ) const
{
    lazyControlCaps();

    for( auto cap : controlCaps ) {
        if( cap.ControlType == controlType ) {
            return true;
        }
    }
    return false;
}

void QHYCamera::getControlCaps( CONTROL_ID controlType, long& min, long& max, long& defaultVal ) const
{
    lazyControlCaps();

    for( auto cap : controlCaps ) {
        if( cap.ControlType == controlType ) {
            min = cap.MinValue;
            max = cap.MaxValue;
            defaultVal = cap.MinValue; // TO_DO
            return;
        }
    }
    assert( false );
}

void QHYCamera::getControlValue( CONTROL_ID control, long& value, bool& isAuto ) const
{
    double v = GetQHYCCDParam( handle, control );
    value = v;
    isAuto = false;
}

void QHYCamera::setControlValue( CONTROL_ID control, long value, bool )
{
    checkResult( SetQHYCCDParam( handle, control, value ) );
}

/*Hardware::IMAGE_TYPE QHYCamera::convert( ASI_IMG_TYPE type )
{
    switch( type ) {
        case ASI_IMG_RAW16: return Hardware::IT_RAW16;
        case ASI_IMG_RAW8: return Hardware::IT_RAW8;
        case ASI_IMG_RGB24: return Hardware::IT_RGB24;
        case ASI_IMG_Y8: return Hardware::IT_Y8;
        default:
            assert( false );
    }
    return Hardware::IT_NONE;
}

ASI_IMG_TYPE QHYCamera::convert( Hardware::IMAGE_TYPE type )
{
    switch( type ) {
        case Hardware::IT_RAW8: return ASI_IMG_RAW8;
        case Hardware::IT_RGB24: return ASI_IMG_RGB24;
        case Hardware::IT_RAW16: return ASI_IMG_RAW16;
        case Hardware::IT_Y8: return ASI_IMG_Y8;
        default:
            assert( false );
    }
    return ASI_IMG_END;
}

Hardware::BAYER_PATTERN QHYCamera::convert( ASI_BAYER_PATTERN pattern )
{
    switch( pattern ) {
        case ASI_BAYER_RG: return Hardware::BP_BAYER_RG;
        case ASI_BAYER_BG: return Hardware::BP_BAYER_BG;
        case ASI_BAYER_GR: return Hardware::BP_BAYER_GR;
        case ASI_BAYER_GB: return Hardware::BP_BAYER_GB;
        default:
            assert( false );
    }
    return Hardware::BP_BAYER_RG;
}

Hardware::ST4_GUIDE_DIRECTION QHYCamera::convert( ASI_GUIDE_DIRECTION direction )
{
    switch( direction ) {
        case ASI_GUIDE_NORTH: return Hardware::GD_GUIDE_NORTH;
        case ASI_GUIDE_SOUTH: return Hardware::GD_GUIDE_SOUTH;
        case ASI_GUIDE_EAST: return Hardware::GD_GUIDE_EAST;
        case ASI_GUIDE_WEST: return Hardware::GD_GUIDE_WEST;
        default:
            assert( false );
    }
    return Hardware::GD_GUIDE_NORTH;
}

ASI_GUIDE_DIRECTION QHYCamera::convert( Hardware::ST4_GUIDE_DIRECTION direction )
{
    switch( direction ) {
        case Hardware::GD_GUIDE_NORTH: return ASI_GUIDE_NORTH;
        case Hardware::GD_GUIDE_SOUTH: return ASI_GUIDE_SOUTH;
        case Hardware::GD_GUIDE_EAST: return ASI_GUIDE_EAST;
        case Hardware::GD_GUIDE_WEST: return ASI_GUIDE_WEST;
        default:
            assert( false );
    }
    return ASI_GUIDE_NORTH;
}*/

class QHYException : public std::exception {
public:
    const char* what() const noexcept override
    {
        return "QHYCCD_ERROR";
    }
};

void QHYCamera::checkResult( uint32_t errorCode )
{
    if( errorCode == QHYCCD_ERROR ) {
        throw QHYException();
    }
}
