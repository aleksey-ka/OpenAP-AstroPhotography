// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef MOCKCAMERA_H
#define MOCKCAMERA_H

#include "camera.h"

#include <atomic>

class MockCamera : public Camera {
public:
    // Get mock cameras count
    static int GetCount();
    // Get mock camera info by index
    static std::shared_ptr<ASI_CAMERA_INFO> GetInfo( int index );

    // Open and initialize the camera by id (see camera info)
    static std::shared_ptr<MockCamera> Open( int id );
    virtual void Close();

    // Get camera info
    virtual std::shared_ptr<ASI_CAMERA_INFO> GetInfo() const;

    // Exposure in microsectods
    virtual long GetExposure( bool& isAuto ) const { isAuto = false; return info.Exposure; }
    virtual void SetExposure( long value, bool isAuto = false ) { info.Exposure = value; }
    virtual void GetExposureCaps( long& min, long& max, long& defaultVal ) const {}

    // Gain
    virtual long GetGain( bool& isAuto ) const { isAuto = false; return 0; }
    virtual void SetGain( long value, bool isAuto = false ) {}
    virtual void GetGainCaps( long& min, long& max, long& defaultVal ) const {}

    // Offset
    virtual long GetOffset() const { return 0; }
    virtual void SetOffset( long value ) {}
    virtual void GetOffsetCaps( long& min, long& max, long& defaultVal ) const {}

    // White balance
    virtual long GetWhiteBalanceR() const { return 0; }
    virtual void SetWhiteBalanceR( long value, bool isAuto = false ) {}
    virtual void GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const {}
    virtual long GetWhiteBalanceB() const { return 0; }
    virtual void SetWhiteBalanceB( long value, bool isAuto = false ) {}
    virtual void GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const {}

    // Image format
    virtual ASI_IMG_TYPE GetFormat() const { return ASI_IMG_RAW16; }
    virtual int GetWidth() const { info.Width; }
    virtual int GetHeight() const { info.Height; }
    virtual int GetBinning() const { return 1; }
    // Image format (all in one)
    virtual void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const;
    virtual void SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType ) {}

    // Do single exposure
    virtual std::shared_ptr<const Raw16Image> DoExposure() const;

    // Number of dropped frames
    virtual int GetDroppedFrames() const { return 0; }
    // Temperature
    virtual double GetCurrentTemperature() const { return 0; }
    // Cooler control
    virtual bool HasCooler() const { return false; }
    virtual bool IsCoolerOn() const { return false; }
    virtual void SetCoolerOn( bool ) {}
    virtual double GetTargetTemperature() const { return 0; }
    virtual void SetTargetTemperature( double ) {}

    // Guiding
    virtual void GuideOn( ASI_GUIDE_DIRECTION ) const {}
    virtual void GuideOff( ASI_GUIDE_DIRECTION ) const {}

    virtual void PrintDebugInfo() {}

    MockCamera( int id );

private:
    int index;
    ImageInfo info;
    mutable std::atomic<bool> isClosing{ false };
    mutable std::atomic<bool> isExposure{ false };
};

#endif // MOCKCAMERA_H
