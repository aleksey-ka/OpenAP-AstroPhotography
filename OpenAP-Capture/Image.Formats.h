// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGE_FORMATS_H
#define IMAGE_FORMATS_H

#include "Image.RawU16Image.h"

class Png16BitGrayscale : public ImageFileFormat {
public:
    virtual std::shared_ptr<const CRawU16Image> Load( const char* filePath, const ImageInfo& ) const override;
    virtual void Save( const char* filePath, const CRawU16Image* ) const override;
};

#endif // IMAGE_FORMATS_H
