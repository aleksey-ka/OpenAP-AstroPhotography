// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <memory>

#include "Image.RawImage.h"

namespace Hardware {

enum BAYER_PATTERN {
    BP_BAYER_RG = 0,
    BP_BAYER_BG,
    BP_BAYER_GR,
    BP_BAYER_GB
};

struct CAMERA_INFO {
    int Id = -1;
    char Name[64];
    bool IsColorCamera;
    BAYER_PATTERN BayerPattern;
    double PixelSize;
    float ElectronsPerADU;
    int BitDepth;
};

enum IMAGE_TYPE {
    IT_RAW8 = 0,
    IT_RGB24,
    IT_RAW16,
    IT_Y8,
    IT_NONE
};

enum ST4_GUIDE_DIRECTION {
    GD_GUIDE_NORTH = 0,
    GD_GUIDE_SOUTH,
    GD_GUIDE_EAST,
    GD_GUIDE_WEST
};

class Camera {
public:
    // Number of available cameras
    static int GetCount();
    // Camera info for a camera by index
    static std::shared_ptr<CAMERA_INFO> GetInfo( int index );
    virtual std::shared_ptr<CAMERA_INFO> GetInfo() = 0;

    // Open camera
    static std::shared_ptr<Camera> Open( const CAMERA_INFO& );
    static std::shared_ptr<Camera> OpenFolder( const char* path );
    // Close camera
    virtual void Close() = 0;

    // Get camera info
    virtual std::shared_ptr<CAMERA_INFO> GetInfo() const = 0;

    // Exposure in microsectods
    virtual long GetExposure( bool& isAuto ) const = 0;
    virtual void SetExposure( long value, bool isAuto = false ) = 0;
    virtual void GetExposureCaps( long& min, long& max, long& defaultVal ) const = 0;

    // Gain
    virtual long GetGain( bool& isAuto ) const = 0;
    virtual void SetGain( long value, bool isAuto = false ) = 0;
    virtual void GetGainCaps( long& min, long& max, long& defaultVal ) const = 0;

    // Offset
    virtual long GetOffset() const = 0;
    virtual void SetOffset( long value ) = 0;
    virtual void GetOffsetCaps( long& min, long& max, long& defaultVal ) const = 0;

    // White balance
    virtual long GetWhiteBalanceR() const = 0;
    virtual void SetWhiteBalanceR( long value, bool isAuto = false ) = 0;
    virtual void GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const = 0;
    virtual long GetWhiteBalanceB() const = 0;
    virtual void SetWhiteBalanceB( long value, bool isAuto = false ) = 0;
    virtual void GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const = 0;

    // Image format
    virtual IMAGE_TYPE GetFormat() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetBinning() const = 0;
    // Image format (all in one)
    virtual void GetROIFormat( int& width, int& height, int& bin, IMAGE_TYPE& imgType ) const = 0;
    virtual void SetROIFormat( int width, int height, int bin, IMAGE_TYPE imgType ) = 0;

    // Do single exposure
    virtual std::shared_ptr<const CRawU16Image> DoExposure() const = 0;

    // Number of dropped frames
    virtual int GetDroppedFrames() const = 0;
    // Temperature
    virtual double GetCurrentTemperature() const = 0;
    // Cooler control
    virtual bool HasCooler() const = 0;
    virtual bool IsCoolerOn() const = 0;
    virtual void SetCoolerOn( bool ) = 0;
    virtual double GetTargetTemperature() const = 0;
    virtual void SetTargetTemperature( double ) = 0;

    // Guiding through ST4 port
    virtual void GuideOn( ST4_GUIDE_DIRECTION ) const = 0;
    virtual void GuideOff( ST4_GUIDE_DIRECTION ) const = 0;

    // Set image info to be used as a template for capture images
    virtual void SetImageInfoTemplate( const ImageInfo& ) = 0;

    virtual ~Camera() {}
};

} // namespace Hardware
