// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include "Image.RawImage.h"

class Png16BitGrayscale : public ImageFileFormat {
public:
    virtual std::shared_ptr<CRawU16Image> Load( const char* filePath, const ImageInfo& ) const override;
    virtual void Save( const char* filePath, const CRawU16Image* ) const override;
};

class FitsU16 : public ImageFileFormat {
public:
    virtual std::shared_ptr<CRawU16Image> Load( const char* filePath, const ImageInfo& ) const override;
    virtual void Save( const char* filePath, const CRawU16Image* ) const override;
};

