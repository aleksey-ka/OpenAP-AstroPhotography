#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <vector>

#include <ASICamera2.h>

class ASICamera {
public:
    static int GetCount();
    static std::shared_ptr<ASI_CAMERA_INFO> GetInfo( int index );

    static std::shared_ptr<ASICamera> Open( int id );
    void Close();

    // Exposure in microsectods
    long GetExposure( bool& isAuto ) const;
    void SetExposure( long value, bool isAuto = false );

    long GetGain( bool& isAuto ) const;
    void SetGain( long value, bool isAuto = false );

    void GetROIFormat( int& width, int& height, int& bin, ASI_IMG_TYPE& imgType ) const;
    void SetROIFormat( int width, int height, int bin, ASI_IMG_TYPE imgType );

    ASI_IMG_TYPE GetFormat() const { lazyROIFormat(); return imgType; }
    int GetWidth() const { lazyROIFormat(); return width; }
    int GetHeight() const { lazyROIFormat(); return height; }
    int GetBinning() const { lazyROIFormat(); return bin; }

    const unsigned short* DoExposure( int width, int height ) const;

    int GetDroppedFrames() const;

    ASICamera( int id );
    ~ASICamera();

private:

    int id;

    mutable int width = 0;
    mutable int height = 0;
    mutable int bin = 0;
    mutable ASI_IMG_TYPE imgType = ASI_IMG_END;

    mutable std::vector<unsigned short> buf;

    void lazyROIFormat() const;
    static void checkResult( ASI_ERROR_CODE );
};

#endif // CAMERA_H
