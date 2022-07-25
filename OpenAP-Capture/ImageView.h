// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <QLabel>
#include <QMouseEvent>

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
