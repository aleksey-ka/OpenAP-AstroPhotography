// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QLabel>
#include <QMouseEvent>
#include <QStyle>

#include "Image.h"

class ImageView : public QLabel
{
    Q_OBJECT
public:
    explicit ImageView( QWidget* parent = nullptr );

protected:
    void mousePressEvent( QMouseEvent* ) override;

signals:
    void imagePressed( int x, int y, Qt::MouseButton, Qt::KeyboardModifiers );
};

#endif // IMAGEVIEW_H
