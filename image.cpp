#include "image.h"

std::shared_ptr<const Raw16Image> Raw16Image::LoadFromFile( const char* filePath )
{
    int width = 1936;
    int height = 1096;

    auto result = std::make_shared<Raw16Image>( width, height );

    FILE* file = fopen( filePath, "rb" );
    fread( result->Buffer(), 1, result->BufferSize(), file );
    fclose( file );

    return result;
}

void Raw16Image::SaveToFile( const char* filePath )
{
    FILE* out = fopen( filePath, "wb" );
    fwrite( Buffer(), 1, BufferSize(), out );
    fclose( out );
}
