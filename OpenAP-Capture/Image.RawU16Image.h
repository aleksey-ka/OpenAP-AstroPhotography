// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_RAWU16IMAGE_H
#define IMAGE_RAWU16IMAGE_H

#include <memory>
#include <vector>

struct ImageInfo {
    int Width;
    int Height;
    int Offset;
    int Gain;
    int Exposure;
    int64_t Timestamp;
    int64_t SeriesId;
    double Temperature;
    std::string Camera;
    std::string CFA;
    std::string Channel;
    std::string FilterDescription;
};

class ImageFileFormat;

class CRawU16Image {
public:
    CRawU16Image( const ImageInfo& _imageInfo ) :
        imageInfo( _imageInfo ),
        buf( _imageInfo.Width * _imageInfo.Height )
    {
    }

    const unsigned short* RawPixels() const { return buf.data(); };
    unsigned short* RawPixels() { return buf.data(); };
    int Count() const { return buf.size(); }

    int Width() const { return imageInfo.Width; }
    int Height() const { return imageInfo.Height; }

    const unsigned char* Buffer() const { return reinterpret_cast<const unsigned char*>( buf.data() ); }
    unsigned char* Buffer() { return reinterpret_cast<unsigned char*>( buf.data() ); }
    int BufferSize() const { return buf.size() * sizeof( unsigned short ); }

    static std::shared_ptr<const CRawU16Image> LoadFromFile( const char* filePath );
    void SaveToFile( const char* filePath, const ImageFileFormat* = 0 ) const;

    const ImageInfo& Info() const { return imageInfo; }

private:
    ImageInfo imageInfo;
    std::vector<unsigned short> buf;
};

class ImageFileFormat {
public:
    virtual std::shared_ptr<const CRawU16Image> Load( const char* filePath, const ImageInfo& ) const = 0;
    virtual void Save( const char* filePath, const CRawU16Image* ) const = 0;
};

#endif // IMAGE_RAWU16IMAGE_H
