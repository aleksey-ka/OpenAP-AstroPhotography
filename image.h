#ifndef IMAGE_H
#define IMAGE_H

#include <memory>
#include <vector>

class Raw16Image {
public:
    Raw16Image( int _width, int _height ) :
        width( _width ),
        height( _height ),
        buf( _width * _height )
    {
    }

    const unsigned short* RawPixels() const { return buf.data(); };

    int Width() const { return width; }
    int Height() const { return height; }

    unsigned char* Buffer() { return reinterpret_cast<unsigned char*>( buf.data() ); }
    int BufferSize() const { return buf.size() * sizeof( unsigned short ); }

    static std::shared_ptr<const Raw16Image> LoadFromFile( const char* filePath );
    void SaveToFile( const char* filePath );

private:
    int width;
    int height;
    mutable std::vector<unsigned short> buf;
};

#endif // IMAGE_H
