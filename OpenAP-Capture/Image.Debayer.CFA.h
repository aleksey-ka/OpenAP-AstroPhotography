// Copyright (C) 2021 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include "Image.Debayer.h"

class CDebayer_RawU16_CFA : public CDebayer_RawU16 {
public:
    using CDebayer_RawU16::CDebayer_RawU16;

    void ToRgbU8( std::uint8_t* rgb, int stride, int x0, int y0, int w, int h, unsigned int* hr, unsigned int* hg, unsigned int* hb );
};
