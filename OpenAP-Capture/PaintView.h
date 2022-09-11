// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#pragma once

#include <QWidget>
#include <QMouseEvent>

class PaintView : public QWidget
{
    Q_OBJECT
public:
    explicit PaintView( QWidget* parent = nullptr );

    void setOnPaint( std::function<void( PaintView*, QPainter& )> value ) { onPaint = value; update(); }
    void setOnMousePressed( std::function<void( PaintView*, QMouseEvent* )> value ) { onMousePressed = value; }
    void setOnKeyPressed( std::function<void( PaintView*, QKeyEvent* )> value ) { onKeyPressed = value; }
    void setOnWheel( std::function<void( PaintView*, QWheelEvent* )> value ) { onWheel = value; }

protected:
    void mousePressEvent( QMouseEvent* ) override;
    void paintEvent( QPaintEvent* ) override;
    void focusOutEvent( QFocusEvent* event ) override { QWidget::focusOutEvent( event ); update(); };
    void keyPressEvent( QKeyEvent* ) override;
    void wheelEvent( QWheelEvent* ) override;

private:
    std::function<void( PaintView*, QPainter& painter )> onPaint;
    std::function<void( PaintView*, QMouseEvent* )> onMousePressed;
    std::function<void( PaintView*, QKeyEvent* )> onKeyPressed;
    std::function<void( PaintView*, QWheelEvent* )> onWheel;
};
