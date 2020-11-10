// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <vector>

struct ImageInfo {
    int Width;
    int Height;
    int Offset;
    int Gain;
    int Exposure;
    int64_t Timestamp;
    std::string Camera;
};

class Raw16Image {
public:
    Raw16Image( const ImageInfo& _imageInfo ) :
        imageInfo( _imageInfo ),
        buf( _imageInfo.Width * _imageInfo.Height )
    {
    }

    const unsigned short* RawPixels() const { return buf.data(); };

    int Width() const { return imageInfo.Width; }
    int Height() const { return imageInfo.Height; }

    const unsigned char* Buffer() const { return reinterpret_cast<unsigned char*>( buf.data() ); }
    unsigned char* Buffer() { return reinterpret_cast<unsigned char*>( buf.data() ); }
    int BufferSize() const { return buf.size() * sizeof( unsigned short ); }

    static std::shared_ptr<const Raw16Image> LoadFromFile( const char* filePath );
    void SaveToFile( const char* filePath ) const;

private:
    ImageInfo imageInfo;
    mutable std::vector<unsigned short> buf;
};

#endif // IMAGE_H
