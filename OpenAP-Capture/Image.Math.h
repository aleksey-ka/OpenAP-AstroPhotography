// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_MATH_H
#define IMAGE_MATH_H

#include <QImage>
#include <QPixmap>
#include <Image.h>

struct CChannelStat {
    int Median;
    int Sigma;
};

class CPixelStatistics {
public:
    CPixelStatistics( int numberOfChannels, int bitsPerChannel );

    std::vector<uint>& operator [] ( size_t index ) { return channels[index]; }
    const std::vector<uint>& operator [] ( size_t index ) const { return channels[index]; }

    CChannelStat stat( int channel, int count ) const;

    size_t p( int channel, int target ) const;
    size_t maxP( int channel, int start, int end ) const;

private:
    const size_t channelSize;
    std::vector<std::vector<uint>> channels;
};

QPixmap FocusingHelper( const Raw16Image* image, int x0, int y0, int W, int H );

#endif // IMAGE_MATH_H
