// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef HARDWARE_CAMERA_MOCKCAMERA_H
#define HARDWARE_CAMERA_MOCKCAMERA_H

#include "Hardware.Camera.h"

#include <QStringList>

#include <atomic>

class MockCamera : public Camera {
public:
    // Get mock cameras count
    static int GetCount();
    // Get mock camera info by index
    static std::shared_ptr<ASI_CAMERA_INFO> GetInfo( int index );

    // Open and initialize the camera by id (see camera info)
    static std::shared_ptr<MockCamera> Open( int id );
    virtual void Close() override;

    // Get camera info
    virtual std::shared_ptr<ASI_CAMERA_INFO> GetInfo() const override;

    // Exposure in microsectods
    virtual long GetExposure( bool& isAuto ) const override { isAuto = false; return currentSettings.Exposure; }
    virtual void SetExposure( long value, bool /*isAuto = false*/ ) override { currentSettings.Exposure = value; }
    virtual void GetExposureCaps( long& /*min*/, long& /*max*/, long& /*defaultVal*/ ) const override {}

    // Gain
    virtual long GetGain( bool& isAuto ) const override { isAuto = false; return 0; }
    virtual void SetGain( long /*value*/, bool /*isAuto = false*/ ) override {}
    virtual void GetGainCaps( long& /*min*/, long& /*max*/, long& /*defaultVal*/ ) const override {}

    // Offset
    virtual long GetOffset() const override { return 0; }
    virtual void SetOffset( long /*value*/ ) override {}
    virtual void GetOffsetCaps( long& /*min*/, long& /*max*/, long& /*defaultVal*/ ) const override {}

    // White balance
    virtual long GetWhiteBalanceR() const override { return 0; }
    virtual void SetWhiteBalanceR( long /*value*/, bool /*isAuto = false*/ ) override {}
    virtual void GetWhiteBalanceRCaps( long& /*min*/, long& /*max*/, long& /*defaultVal*/ ) const override {}
    virtual long GetWhiteBalanceB() const override { return 0; }
    virtual void SetWhiteBalanceB( long /*value*/, bool /*isAuto = false*/ ) override {}
    virtual void GetWhiteBalanceBCaps( long& /*min*/, long& /*max*/, long& /*defaultVal*/ ) const override {}

    // Image format
    virtual ASI_IMG_TYPE GetFormat() const override { return ASI_IMG_RAW16; }
    virtual int GetWidth() const override { return currentSettings.Width; }
    virtual int GetHeight() const override { return currentSettings.Height; }
    virtual int GetBinning() const override { return 1; }
    // Image format (all in one)
    virtual void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const override;
    virtual void SetROIFormat( int /*width*/, int /*height*/, int /*bin*/, ASI_IMG_TYPE /*imgType*/ ) override {}

    // Do single exposure
    virtual std::shared_ptr<const CRawU16Image> DoExposure() const override;

    // Number of dropped frames
    virtual int GetDroppedFrames() const override { return 0; }
    // Temperature
    virtual double GetCurrentTemperature() const override { return 0; }
    // Cooler control
    virtual bool HasCooler() const override { return false; }
    virtual bool IsCoolerOn() const override { return false; }
    virtual void SetCoolerOn( bool ) override {}
    virtual double GetTargetTemperature() const override { return 0; }
    virtual void SetTargetTemperature( double ) override {}

    // Guiding
    virtual void GuideOn( ASI_GUIDE_DIRECTION ) const override {}
    virtual void GuideOff( ASI_GUIDE_DIRECTION ) const override {}

    virtual void SetImageInfoTemplate( const ImageInfo& imageInfo ) override { templateImageInfo = imageInfo; };

    virtual void PrintDebugInfo() override {}

    MockCamera( int id );

private:
    int index;
    QStringList frames;
    mutable int nextFrame = 0;
    mutable bool forwardPass = true;
    ImageInfo templateImageInfo;
    ImageInfo currentSettings;
    mutable std::atomic<bool> isClosing{ false };
    mutable std::atomic<bool> isExposure{ false };
};

#endif // HARDWARE_CAMERA_MOCKCAMERA_H
