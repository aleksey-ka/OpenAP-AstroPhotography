// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <Image.Image.h>

#include <memory>
#include <vector>

struct ImageInfo {
    int Width;
    int Height;
    int Offset;
    int Gain;
    int Exposure;
    int BitDepth;
    int64_t Timestamp;
    int64_t SeriesId;
    double Temperature;
    std::string Camera;
    std::string CFA;
    std::string Channel;
    std::string FilterDescription;
};

class ImageFileFormat;

class CRawU16Image : public CPixelBuffer<unsigned short> {
public:
    CRawU16Image( const ImageInfo& _imageInfo ) :
        imageInfo( _imageInfo ),
        CPixelBuffer( _imageInfo.Width, _imageInfo.Height )
    {
    }

    const unsigned short* RawPixels() const { return Pixels(); };
    unsigned short* RawPixels() { return Pixels(); };

    int BitDepth() const { return imageInfo.BitDepth; }

    const unsigned char* Buffer() const { return reinterpret_cast<const unsigned char*>( buffer.data() ); }
    unsigned char* Buffer() { return reinterpret_cast<unsigned char*>( buffer.data() ); }
    int BufferSize() const { return buffer.size() * sizeof( unsigned short ); }

    static std::shared_ptr<const CRawU16Image> LoadFromFile( const char* filePath ) { return LoadFromFileRW( filePath ); }
    static std::shared_ptr<CRawU16Image> LoadFromFileRW( const char* filePath );
    void SaveToFile( const char* filePath, const ImageFileFormat* = 0 ) const;

    const ImageInfo& Info() const { return imageInfo; }

private:
    ImageInfo imageInfo;
};

class ImageFileFormat {
public:
    virtual std::shared_ptr<CRawU16Image> Load( const char* filePath, const ImageInfo& ) const = 0;
    virtual void Save( const char* filePath, const CRawU16Image* ) const = 0;

    virtual ~ImageFileFormat() {}
};
