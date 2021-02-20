// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_DEBAYER_HQLINEAR_H
#define IMAGE_DEBAYER_HQLINEAR_H

#include "Image.Debayer.h"

class CDebayer_RawU16_HQLiner : public CDebayer_RawU16 {
public:
    using CDebayer_RawU16::CDebayer_RawU16;

    void ToRgbU16( unsigned short* rgb, int stride, int x0, int y0, int w, int h );
    void ToRgbU8( unsigned char* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb );
};

#endif // IMAGE_DEBAYER_HQLINEAR_H
