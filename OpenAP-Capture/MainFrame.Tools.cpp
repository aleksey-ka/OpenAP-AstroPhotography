// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include "MainFrame.Tools.h"
#include "ui_MainFrame.h"

#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Tools::TargetCircle::Draw( QPainter& painter, QPixmap& pixmap ) const
{
    QPen pen( QColor::fromRgb( 0xFF, 0, 0 ) );
    pen.setWidth( 1 );
    painter.setPen( pen );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.drawEllipse( pixmap.width() / 2 - 30, pixmap.height() / 2 - 30, 60, 60 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Tools::FocusingHelper::Draw( QPainter& painter, QPixmap& ) const
{
    QPen pen( QColor::fromRgb( 0xFF, 0, 0 ) );
    pen.setWidth( 1 );
    painter.setPen( pen );
    painter.setRenderHint( QPainter::Antialiasing );

    double x = focusingHelper->PX;
    double y = focusingHelper->PY;
    if( not ui->showFullResolution->isChecked() ) {
        x /= 2;
        y /= 2;
    }
    painter.drawEllipse( round( x ) - 3, round( y ) - 3, 6, 6 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Tools::OnNewImage( const CRawU16Image* image )
{
    for( auto tool : tools ) {
        tool->OnNewImage( image );
    }
}

void Tools::Draw( QPixmap& pixmap )
{
    if( tools.size() > 0 ) {
        QPainter painter( &pixmap );
        for( auto tool : tools ) {
            tool->Draw( painter, pixmap );
        }
    }
}

int Tools::find( const std::type_info& what )
{
    for( size_t i = 0; i < tools.size(); i++) {
        auto& tool = *tools[i];
        if( typeid( tool ) == what ) {
            return i;
        }
    }
    return -1;
}

bool Tools::doDelete(  const std::type_info& what )
{
    int pos = find( what );
    if( pos != - 1 ) {
        tools.erase( tools.begin() + pos );
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
