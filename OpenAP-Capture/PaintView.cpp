// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "PaintView.h"

#include <QStyle>
#include <QPainter>

PaintView::PaintView( QWidget* parent ) : QWidget( parent )
{
    //setBackgroundRole( QPalette::Base );
    //setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
}

void PaintView::paintEvent( QPaintEvent* event )
{
    QWidget::paintEvent( event );

    if( onPaint != nullptr ) {
        QPainter painter( this );
        onPaint( this, painter );
    }
}

void PaintView::mousePressEvent( QMouseEvent* event )
{
    if( onMousePressed != nullptr ) {
        onMousePressed( this, event );
    }
}

void PaintView::keyPressEvent( QKeyEvent* event )
{
    if( onKeyPressed != nullptr ) {
        onKeyPressed( this, event );
    }
}

void PaintView::wheelEvent( QWheelEvent* event )
{
    if( onWheel != nullptr ) {
        onWheel( this, event );
    }
}
