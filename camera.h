#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <vector>

#include <ASICamera2.h>

#include "image.h"

class ASICamera {
public:
    // Get attached cameras count
    static int GetCount();
    // Get attachecd camera info
    static std::shared_ptr<ASI_CAMERA_INFO> GetInfo( int index );

    // Open and initialize the camera by id (see camera info)
    static std::shared_ptr<ASICamera> Open( int id );
    // Close the camera
    void Close();

    // Exposure in microsectods
    long GetExposure( bool& isAuto ) const;
    void SetExposure( long value, bool isAuto = false );
    void GetExposureCaps( long& min, long& max, long& defaultVal ) const;

    // Gain
    long GetGain( bool& isAuto ) const;
    void SetGain( long value, bool isAuto = false );
    void GetGainCaps( long& min, long& max, long& defaultVal ) const;

    // White balance
    long GetWhiteBalanceR() const;
    void SetWhiteBalanceR( long value, bool isAuto = false );
    void GetWhiteBalanceRCaps( long& min, long& max, long& defaultVal ) const;
    long GetWhiteBalanceB() const;
    void SetWhiteBalanceB( long value, bool isAuto = false );
    void GetWhiteBalanceBCaps( long& min, long& max, long& defaultVal ) const;

    // Offset
    long GetOffset() const;
    void SetOffset( long value );
    void GetOffsetCaps( long& min, long& max, long& defaultVal ) const;

    // Image format
    ASI_IMG_TYPE GetFormat() const { lazyROIFormat(); return imgType; }
    int GetWidth() const { lazyROIFormat(); return width; }
    int GetHeight() const { lazyROIFormat(); return height; }
    int GetBinning() const { lazyROIFormat(); return bin; }
    // Image format (all in one)
    void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const;
    void SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType );

    // Do single exposure
    std::shared_ptr<const Raw16Image> DoExposure() const;

    // Number of dropped frames
    int GetDroppedFrames() const;
    // Temperature
    double GetTemperature() const;

    // Guiding
    void GuideOn( ASI_GUIDE_DIRECTION ) const;
    void GuideOff( ASI_GUIDE_DIRECTION ) const;

    void PrintDebugInfo();

    ASICamera( int id );
    ~ASICamera();

private:
    int id;
    mutable std::vector<ASI_CONTROL_CAPS> controlCaps;

    mutable int width = 0;
    mutable int height = 0;
    mutable int bin = 0;
    mutable ASI_IMG_TYPE imgType = ASI_IMG_END;

    void lazyROIFormat() const;
    void lazyControlCaps() const;
    void getControlCaps( ASI_CONTROL_TYPE, long& min, long& max, long& defaultVal ) const;
    static void checkResult( ASI_ERROR_CODE );
};

#endif // CAMERA_H
