// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef RENDERER_H
#define RENDERER_H

#include <QImage>
#include <QPixmap>

class Renderer {
public:
    Renderer( const ushort* raw, int width, int height );

    QPixmap RenderHalfResWithHistogram();
    QPixmap RenderHistogram();

    QPixmap RenderRectHalfRes( int cx, int cy, int W, int H );

private:
    const ushort* raw;
    int width;
    int height;

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
};

#endif // RENDERER_H
