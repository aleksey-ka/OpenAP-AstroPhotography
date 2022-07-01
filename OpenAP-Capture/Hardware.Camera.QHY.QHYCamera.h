// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef HARDWARE_CAMERA_QHY_QHYCAMERA_H
#define HARDWARE_CAMERA_QHY_QHYCAMERA_H

#include "Hardware.Camera.h"

#include <qhyccd.h>

#include <atomic>
#include <vector>

class QHYCamera : public Hardware::Camera {
public:
    // Get attached cameras count
    static int GetCount();
    // Get attached camera info by index
    static std::shared_ptr<Hardware::CAMERA_INFO> GetInfo( int index );

    // Open and initialize the camera
    static std::shared_ptr<QHYCamera> Open( const Hardware::CAMERA_INFO& );
    // Close the camera
    void Close() override;

    // Get camera info
    std::shared_ptr<Hardware::CAMERA_INFO> GetInfo() const override;

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
    Hardware::IMAGE_TYPE GetFormat() const override { /*lazyROIFormat(); return convert( imgType );*/ return Hardware::IT_NONE; }
    int GetWidth() const override { lazyROIFormat(); return width; }
    int GetHeight() const override { lazyROIFormat(); return height; }
    int GetBinning() const override { lazyROIFormat(); return bin; }
    // Image format (all in one)
    void GetROIFormat( int& width, int& height, int& bin, Hardware::IMAGE_TYPE& imgType ) const override;
    void SetROIFormat( int width, int height, int bin, Hardware::IMAGE_TYPE imgType ) override;

    // Do single exposure
    std::shared_ptr<const CRawU16Image> DoExposure() const override;

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
    void GuideOn( Hardware::ST4_GUIDE_DIRECTION ) const override;
    void GuideOff( Hardware::ST4_GUIDE_DIRECTION ) const override;

    virtual void SetImageInfoTemplate( const ImageInfo& imageInfo ) override { imageInfoTemplate = imageInfo; };

    void PrintDebugInfo() override;

    QHYCamera( const char* id );
    ~QHYCamera();

private:
    qhyccd_handle* handle = 0;
    mutable std::shared_ptr<Hardware::CAMERA_INFO> cameraInfo;

    struct QHY_CONTROL_CAPS
    {
        CONTROL_ID ControlType;
        double MinValue;
        double MaxValue;
        double Step;

        QHY_CONTROL_CAPS( double minv, double maxv, double step ) : MinValue( minv ), MaxValue( maxv ), Step( step ) {}
    };

    mutable std::vector<QHY_CONTROL_CAPS> controlCaps;

    mutable long exposure = -1;
    //mutable ASI_BOOL isAutoExposure = ASI_FALSE;
    mutable long gain = -1;
    //mutable ASI_BOOL isAutoGain = ASI_FALSE;
    mutable long offset = -1;
    //mutable ASI_BOOL isAutoOffset = ASI_FALSE;
    mutable long WB_R = -1;
    //mutable ASI_BOOL isAutoWB_R = ASI_FALSE;
    mutable long WB_B = -1;
    //mutable ASI_BOOL isAutoWB_B = ASI_FALSE;
    mutable long coolerOn = -1;
    mutable long targetTemperature = -1;

    mutable int width = 0;
    mutable int height = 0;
    mutable int bin = 0;
    //mutable ASI_IMG_TYPE imgType = ASI_IMG_END;

    mutable std::atomic<bool> isClosing{ false };
    mutable std::atomic<bool> isExposure{ false };

    ImageInfo imageInfoTemplate;

    void lazyROIFormat() const;
    void lazyControlCaps() const;
    bool hasControlCaps( CONTROL_ID ) const;
    void getControlCaps( CONTROL_ID, long& min, long& max, long& defaultVal ) const;
    void getControlValue( CONTROL_ID, long& value, bool& isAuto ) const;
    void setControlValue( CONTROL_ID, long value, bool isAuto );
    static void checkResult( uint32_t );

    static std::shared_ptr<Hardware::CAMERA_INFO> createCameraInfo( const char* id );
    /*static Hardware::IMAGE_TYPE convert( ASI_IMG_TYPE );
    static ASI_IMG_TYPE convert( Hardware::IMAGE_TYPE );
    static Hardware::BAYER_PATTERN convert( ASI_BAYER_PATTERN );
    static Hardware::ST4_GUIDE_DIRECTION convert( ASI_GUIDE_DIRECTION );
    static ASI_GUIDE_DIRECTION convert( Hardware::ST4_GUIDE_DIRECTION );*/
};

#endif // HARDWARE_CAMERA_QHY_QHYCAMERA_H

