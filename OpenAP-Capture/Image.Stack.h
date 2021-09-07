// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_SEQUENCE_H
#define IMAGE_SEQUENCE_H

#include <Image.Math.h>
#include <Image.RawImage.h>

#include <memory>

class ImageSequence {
public:
    virtual ~ImageSequence() {}
    virtual size_t Count() const = 0;
    virtual std::shared_ptr<const CRawU16Image> LoadRawU16( size_t index) const = 0;
};

class CStacker {
public:
    class Callback {
    public:
        virtual void OnShowImage( std::shared_ptr<const CRgbImage> ) = 0;
    };

    int GetBitDepth() const { return bitDepth; }

protected:
    virtual ~CStacker() {}

    std::shared_ptr<CPixelBuffer<double>> stack1;
    int count1;
    std::shared_ptr<CPixelBuffer<double>> stack2;
    int count2;

    size_t count = 0;
    int bitDepth = 0;

    virtual std::shared_ptr<const CPixelBuffer<double>> calibrateImage( std::shared_ptr<const CRawU16Image> rawImage );
    void prepareTwoStacks( const ImageSequence&, CStacker::Callback* );

    void analyzePixels( const CPixelBuffer<double>& );
};

class CDarksStacker : public CStacker {
public:
    std::shared_ptr<CPixelBuffer<double>> Process( const ImageSequence&, Callback* callback = 0 );
};

class CFlatsStacker : public CStacker {
public:
    std::shared_ptr<CPixelBuffer<double>> Process( const ImageSequence&, Callback* callback = 0 );

    void SetDarkFrame( std::shared_ptr<CPixelBuffer<double>> value ) { darkFrame = value; }

private:
    std::shared_ptr<CPixelBuffer<double>> darkFrame;
    virtual std::shared_ptr<const CPixelBuffer<double>> calibrateImage( std::shared_ptr<const CRawU16Image> rawImage );
};

class CLightsStacker : public CStacker {
public:
    std::shared_ptr<CPixelBuffer<double>> Process( const ImageSequence&, Callback* callback = 0 );

    void SetDarkFrame( std::shared_ptr<CPixelBuffer<double>> value ) { darkFrame = value; }
    void SetFlatFrame( std::shared_ptr<CPixelBuffer<double>> value ) { flatFrame = value; }

    void SetOffset( double value ) { offset = value; }


private:
    std::shared_ptr<CPixelBuffer<double>> darkFrame;
    std::shared_ptr<CPixelBuffer<double>> flatFrame;
    double offset = 0.0;
    virtual std::shared_ptr<const CPixelBuffer<double>> calibrateImage( std::shared_ptr<const CRawU16Image> rawImage );
};

#endif // IMAGE_SEQUENCE_H
