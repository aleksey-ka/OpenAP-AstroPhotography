// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef ASICAMERA_H
#define ASICAMERA_H

#include "camera.h"

#include <atomic>
#include <vector>

class ASICamera : public Camera {
public:
    // Get attached cameras count
    static int GetCount();
    // Get attached camera info by index
    static std::shared_ptr<ASI_CAMERA_INFO> GetInfo( int index );

    // Open and initialize the camera by id (see camera info)
    static std::shared_ptr<ASICamera> Open( int id );
    // Close the camera
    void Close() override;

    // Get camera info
    std::shared_ptr<ASI_CAMERA_INFO> GetInfo() const override;

    // Exposure in microsectods
    long GetExposure( bool& isAuto ) const override;
    void SetExposure( long value, bool isAuto = false ) override;
    void GetExposureCaps( long& min, long& max, long& defaultVal ) const override;

    // Gain
    long GetGain( bool& isAuto ) const override;
    void SetGain( long value, bool isAuto = false ) override;
    void GetGainCaps( long& min, long& max, long& defaultVal ) const override;

    // Offset
    long GetOffset() const override;
    void SetOffset( long value ) override;
    void GetOffsetCaps( long& min, long& max, long& defaultVal ) const override;

    // White balance
    long GetWhiteBalanceR() const override;
    void SetWhiteBalanceR( long value, bool isAuto = false ) override;
    void GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const override;
    long GetWhiteBalanceB() const override;
    void SetWhiteBalanceB( long value, bool isAuto = false ) override;
    void GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const override;

    // Image format
    ASI_IMG_TYPE GetFormat() const override { lazyROIFormat(); return imgType; }
    int GetWidth() const override { lazyROIFormat(); return width; }
    int GetHeight() const override { lazyROIFormat(); return height; }
    int GetBinning() const override { lazyROIFormat(); return bin; }
    // Image format (all in one)
    void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const override;
    void SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType ) override;

    // Do single exposure
    std::shared_ptr<const Raw16Image> DoExposure() const override;

    // Number of dropped frames
    int GetDroppedFrames() const override;
    // Temperature
    double GetCurrentTemperature() const override;
    // Cooler control
    bool HasCooler() const override;
    bool IsCoolerOn() const override;
    void SetCoolerOn( bool ) override;
    double GetTargetTemperature() const override;
    void SetTargetTemperature( double ) override;

    // Guiding
    void GuideOn( ASI_GUIDE_DIRECTION ) const override;
    void GuideOff( ASI_GUIDE_DIRECTION ) const override;

    void SetFilterDescription( const char* txt ) { filterDescription = txt; }

    void PrintDebugInfo() override;

    ASICamera( int id );
    ~ASICamera();

private:
    int id;
    mutable std::shared_ptr<ASI_CAMERA_INFO> cameraInfo;

    mutable std::vector<ASI_CONTROL_CAPS> controlCaps;

    mutable long exposure = -1;
    mutable ASI_BOOL isAutoExposure = ASI_FALSE;
    mutable long gain = -1;
    mutable ASI_BOOL isAutoGain = ASI_FALSE;
    mutable long offset = -1;
    mutable ASI_BOOL isAutoOffset = ASI_FALSE;
    mutable long WB_R = -1;
    mutable ASI_BOOL isAutoWB_R = ASI_FALSE;
    mutable long WB_B = -1;
    mutable ASI_BOOL isAutoWB_B = ASI_FALSE;
    mutable long coolerOn = -1;
    mutable long targetTemperature = -1;

    mutable int width = 0;
    mutable int height = 0;
    mutable int bin = 0;
    mutable ASI_IMG_TYPE imgType = ASI_IMG_END;

    mutable std::atomic<bool> isClosing{ false };
    mutable std::atomic<bool> isExposure{ false };

    std::string filterDescription;

    void lazyROIFormat() const;
    void lazyControlCaps() const;
    bool hasControlCaps( ASI_CONTROL_TYPE ) const;
    void getControlCaps( ASI_CONTROL_TYPE, long& min, long& max, long& defaultVal ) const;
    static void checkResult( ASI_ERROR_CODE );
};

#endif // ASICAMERA_H

