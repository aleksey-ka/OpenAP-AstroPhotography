#include "camera.h"

#include <QDebug>

int ASICamera::GetCount()
{
    return ASIGetNumOfConnectedCameras();
}

std::shared_ptr<ASI_CAMERA_INFO> ASICamera::GetInfo( int index )
{
    auto cameraInfo = std::make_shared<ASI_CAMERA_INFO>();
    checkResult( ASIGetCameraProperty( cameraInfo.get(), index ) );
    return cameraInfo;
}

std::shared_ptr<ASICamera> ASICamera::Open( int id )
{
    return std::make_shared<ASICamera>( id );
}

void ASICamera::Close()
{
    if( id != -1 ) {
        checkResult( ASICloseCamera( id ) );
        id = -1;
    }
}


ASICamera::ASICamera( int _id ) : id( _id )
{
    assert( id != -1 );

    checkResult( ASIOpenCamera( id ) );
    checkResult( ASIInitCamera( id ) );
    checkResult( ASIDisableDarkSubtract( id ) );
}

ASICamera::~ASICamera()
{
    try {
        Close();
    } catch( std::exception& e ) {
       qDebug() << "Error: " << e.what();
    }
}

long ASICamera::GetExposure( bool& isAuto ) const
{
    long exposure = -1;
    ASI_BOOL isAutoExposure = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_EXPOSURE, &exposure, &isAutoExposure ) );
    isAuto = isAutoExposure == ASI_TRUE;
    return exposure;
}

void ASICamera::SetExposure( long value, bool isAuto )
{
    checkResult( ASISetControlValue( id, ASI_EXPOSURE, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
}

void ASICamera::GetExposureCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( ASI_EXPOSURE, min, max, defaultVal );
}

long ASICamera::GetGain( bool& isAuto ) const
{
    long exposure = -1;
    ASI_BOOL isAutoExposure = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_GAIN, &exposure, &isAutoExposure ) );
    isAuto = isAutoExposure == ASI_TRUE;
    return exposure;
}

void ASICamera::SetGain( long value, bool isAuto )
{
    checkResult( ASISetControlValue( id, ASI_GAIN, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
}

void ASICamera::GetGainCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( ASI_GAIN, min, max, defaultVal );
}

long ASICamera::GetWhiteBalanceR() const
{
    long wb = -1;
    ASI_BOOL isAuto = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_WB_R, &wb, &isAuto ) );
    return wb;
}

void ASICamera::SetWhiteBalanceR( long value, bool isAuto )
{
    checkResult( ASISetControlValue( id, ASI_WB_R, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
}

void ASICamera::GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( ASI_WB_R, min, max, defaultVal );
}

long ASICamera::GetWhiteBalanceB() const
{
    long wb = -1;
    ASI_BOOL isAuto = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_WB_B, &wb, &isAuto ) );
    return wb;
}

void ASICamera::SetWhiteBalanceB( long value, bool isAuto )
{
    checkResult( ASISetControlValue( id, ASI_WB_B, value, isAuto ? ASI_TRUE : ASI_FALSE ) );
}

void ASICamera::GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( ASI_WB_B, min, max, defaultVal );
}

long ASICamera::GetOffset() const
{
    long wb = -1;
    ASI_BOOL isAuto = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_OFFSET, &wb, &isAuto ) );
    return wb;
}

void ASICamera::SetOffset( long value )
{
    checkResult( ASISetControlValue( id, ASI_OFFSET, value, ASI_FALSE ) );
}

void ASICamera::GetOffsetCaps( long& min, long& max, long& defaultVal ) const
{
    getControlCaps( ASI_OFFSET, min, max, defaultVal );
}

void ASICamera::GetROIFormat( int& _width, int& _height, int& _bin, ASI_IMG_TYPE& _imgType ) const
{
    lazyROIFormat();

    _width = width;
    _height = height;
    _bin = bin;
    _imgType = imgType;
}

void ASICamera::lazyROIFormat() const
{
    if( imgType == ASI_IMG_END ) {
        checkResult( ASIGetROIFormat( id, &width, &height, &bin, &imgType ) );
    }
}

void ASICamera::SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType )
{
    checkResult( ASISetROIFormat( id, width, height, bin, imgType ) );
    this->imgType = ASI_IMG_END;
}

class ImageImpl : public ASICamera::Image {
public:
    ImageImpl( int width, int height ) : buf( width * height)
    {
        RawPixels = buf.data();
    }

    unsigned char* Buffer() { return reinterpret_cast<unsigned char*>( buf.data() ); }
    int Size() { return buf.size() * sizeof( ushort ); }

private:
    mutable std::vector<unsigned short> buf;
};

std::shared_ptr<const ASICamera::Image> ASICamera::DoExposure() const
{
    checkResult( ASIStartExposure( id, ASI_FALSE ) );

    lazyROIFormat();

    auto result = std::make_shared<ImageImpl>( width, height );

    bool capture = true;
    do {
        ASI_EXPOSURE_STATUS status;
        checkResult( ASIGetExpStatus( id, &status ) );
        switch( status ) {
            case ASI_EXP_SUCCESS: qDebug() << "OK"; capture = false; break;
            case ASI_EXP_FAILED: qDebug() << "Failed"; capture = false; break;
            case ASI_EXP_WORKING: continue;
            default:
                assert( false );
        }
    } while( capture );
    checkResult( ASIGetDataAfterExp( id, result->Buffer(), result->Size() ) );

    return result;
}

int ASICamera::GetDroppedFrames() const
{
    int dropped = 0;
    checkResult( ASIGetDroppedFrames( id, &dropped ) );
    return dropped;
}

double ASICamera::GetTemperature() const
{
    long temperature = 0;
    ASI_BOOL isAutoTemperature = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_TEMPERATURE, &temperature, &isAutoTemperature ) );
    return static_cast<double>( temperature ) / 10.0;
}

void ASICamera::PrintDebugInfo()
{
    qDebug() << "ASI SDK" << ASIGetSDKVersion();

    lazyControlCaps();

    for( auto cap : controlCaps ) {
        qDebug() << cap.Name << cap.Description;
        qDebug() << "[" << cap.MinValue << cap.MaxValue << cap.DefaultValue <<
            ( cap.IsAutoSupported ? "auto" : "" ) << ( cap.IsWritable ? "rw" : "r" ) << "]";
    }
}

void ASICamera::lazyControlCaps() const
{
    if( controlCaps.size() == 0 ) {
        int count = 0;
        checkResult( ASIGetNumOfControls( id, &count ) );
        controlCaps.resize( count );
        for( int i = 0; i < count; i++ ) {
            checkResult( ASIGetControlCaps( id, i, &controlCaps[i] ) );
        }
    }
}

void ASICamera::getControlCaps( ASI_CONTROL_TYPE controlType, long& min, long& max, long& defaultVal ) const
{
    lazyControlCaps();

    for( auto cap : controlCaps ) {
        if( cap.ControlType == controlType ) {
            min = cap.MinValue;
            max = cap.MaxValue;
            defaultVal = cap.DefaultValue;
            return;
        }
    }
    assert( false );
}

class ASIException : public std::exception {
public:
    ASIException( ASI_ERROR_CODE _errorCode ) :
        errorCode( _errorCode )
    {
    }

    const char* what() const noexcept override
    {
        switch( errorCode ) {
            case ASI_ERROR_INVALID_INDEX: return "No camera connected or index value out of boundary";
            case ASI_ERROR_INVALID_ID: return "Invalid ID";
            case ASI_ERROR_INVALID_CONTROL_TYPE: return "Invalid control type";
            case ASI_ERROR_CAMERA_CLOSED: return "Camera is not open";
            case ASI_ERROR_CAMERA_REMOVED: return "Failed to find the camera, maybe the camera has been removed";
            case ASI_ERROR_INVALID_PATH: return "Cannot find the path of the file";
            case ASI_ERROR_INVALID_FILEFORMAT: return "Invalid file format";
            case ASI_ERROR_INVALID_SIZE: return "Invalid video format size";
            case ASI_ERROR_INVALID_IMGTYPE: return "Unsupported image formate";
            case ASI_ERROR_OUTOF_BOUNDARY: return "The startpos is out of boundary";
            case ASI_ERROR_TIMEOUT: return "Timeout";
            case ASI_ERROR_INVALID_SEQUENCE: return "Stop capture first";
            case ASI_ERROR_BUFFER_TOO_SMALL: return "Buffer size is not big enough";
            case ASI_ERROR_VIDEO_MODE_ACTIVE: return "Video mode is active";
            case ASI_ERROR_EXPOSURE_IN_PROGRESS: return "Exposure in progress";
            case ASI_ERROR_GENERAL_ERROR: return "General error, eg: value is out of valid range";
            case ASI_ERROR_INVALID_MODE: return "The current mode is wrong";
            default:
                return 0;
        }
    }

private:
    ASI_ERROR_CODE errorCode;
};

void ASICamera::checkResult( ASI_ERROR_CODE errorCode )
{
    if( errorCode != ASI_SUCCESS ) {
        throw ASIException( errorCode );
    }
}
