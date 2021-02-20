// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include <ASICamera2.h>

#include "Image.RawImage.h"

class Camera {
public:
    virtual void Close() = 0;

    // Get camera info
    virtual std::shared_ptr<ASI_CAMERA_INFO> GetInfo() const = 0;

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
    virtual ASI_IMG_TYPE GetFormat() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual int GetBinning() const = 0;
    // Image format (all in one)
    virtual void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const = 0;
    virtual void SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType ) = 0;

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
    virtual void GuideOn( ASI_GUIDE_DIRECTION ) const = 0;
    virtual void GuideOff( ASI_GUIDE_DIRECTION ) const = 0;

    // Set image info to be used as a template for capture images
    virtual void SetImageInfoTemplate( const ImageInfo& ) = 0;

    virtual void PrintDebugInfo() = 0;
};

#endif // CAMERA_H
