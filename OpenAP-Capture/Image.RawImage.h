// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <Image.Image.h>

#include <memory>
#include <vector>

enum IMAGE_FLAGS {
    IF_SERIES_START = 0x1,
    IF_SERIES_END = 0x2
};

struct ImageInfo {
    int Width = 0;
    int Height = 0;
    int Offset = 0;
    int Gain = 0;
    int Exposure = 0;
    int BitDepth = 0;
    int64_t Timestamp = 0;
    int64_t SeriesId = 0;
    double Temperature = 0.0;
    uint32_t Flags = 0;
    std::string Camera;
    std::string CFA;
    std::string Channel;
    std::string FilterDescription;
    std::string FilePath;
};

class ImageFileFormat;

class CRawU16Image : public CPixelBuffer<unsigned short> {
public:
    CRawU16Image( const ImageInfo& _imageInfo ) :
        CPixelBuffer( _imageInfo.Width, _imageInfo.Height ),
        imageInfo( _imageInfo )
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
