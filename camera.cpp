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
    isAuto = isAutoExposure = ASI_TRUE;
    return exposure;
}

void ASICamera::SetExposure( long value, bool isAuto )
{
    ASI_BOOL isAutoExposure = isAuto ? ASI_TRUE : ASI_FALSE;
    checkResult( ASISetControlValue( id, ASI_EXPOSURE, value, isAutoExposure ) );
}

long ASICamera::GetGain( bool& isAuto ) const
{
    long exposure = -1;
    ASI_BOOL isAutoExposure = ASI_FALSE;
    checkResult( ASIGetControlValue( id, ASI_GAIN, &exposure, &isAutoExposure ) );
    isAuto = isAutoExposure = ASI_TRUE;
    return exposure;
}

void ASICamera::SetGain( long value, bool isAuto )
{
    ASI_BOOL isAutoExposure = isAuto ? ASI_TRUE : ASI_FALSE;
    checkResult( ASISetControlValue( id, ASI_GAIN, value, isAutoExposure ) );
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
    imgType == ASI_IMG_END;
}

const ushort* ASICamera::DoExposure( int width, int height ) const
{
    lazyROIFormat();

    if( buf.size() < width * height ) {
        buf.resize( width * height );
    }

    checkResult( ASIStartExposure( id, ASI_FALSE ) );
    bool capture = true;
    do {
        ASI_EXPOSURE_STATUS status;
        checkResult( ASIGetExpStatus( id, &status ) );
        switch( status ) {
            case ASI_EXP_SUCCESS: qDebug() << "OK"; capture = false; break;
            case ASI_EXP_FAILED: qDebug() << "Failed"; capture = false; break;
        }
    } while( capture );
    checkResult( ASIGetDataAfterExp( id, (unsigned char*)buf.data(), buf.size() * sizeof( ushort ) ) );

    return buf.data();
}

int ASICamera::GetDroppedFrames() const
{
    int dropped = 0;
    checkResult( ASIGetDroppedFrames( id, &dropped ) );
    return dropped;
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
            case ASI_SUCCESS:
            case ASI_ERROR_END:
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
