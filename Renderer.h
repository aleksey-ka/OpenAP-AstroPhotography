// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef RENDERER_H
#define RENDERER_H

#include <QImage>
#include <QPixmap>

enum TRenderingMethod {
    RM_HalfResolution,
    RM_FullResolution
};

class Renderer {
public:
    Renderer( const ushort* raw, int width, int height );

    QPixmap Render( TRenderingMethod );
    QPixmap RenderHistogram();

    QPixmap RenderRectHalfRes( int cx, int cy, int W, int H );
    QPixmap RenderGrayScale();
private:
    const ushort* raw;
    int width;
    int height;

    // Histogram data
    std::vector<uint> histR;
    std::vector<uint> histG;
    std::vector<uint> histB;

    // Fast statistics (calculated for each pixel on each frame)
    ushort maxValue = 0;
    uint maxCount = 0;
    ushort minValue = USHRT_MAX;
    uint minCount = 0;
    inline ushort addToStatistics( ushort value )
    {
        if( value >= maxValue ) {
            if( value == maxValue ) {
                maxCount++;
            } else {
                maxValue = value;
                maxCount = 1;
            }
        } else if( value <= minValue ) {
            if( value == minValue ) {
                minCount++;
            } else {
                minValue = value;
                minCount = 1;
            }
        }
        return value;
    }

    void renderHalfResolutionWithHistogram( uchar* rgb, int byteWidth );
    void renderHighQualityLinearWithHistogram( uchar* rgb, int byteWidth );
};

#endif // RENDERER_H