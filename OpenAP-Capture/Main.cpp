// Copyright (C) 2020 Aleksey Kalyuzhny. Released under the terms of the
// GNU General Public License version 3. See <http://www.gnu.org/licenses/>

#include <QApplication>
#include <QScreen>
#include <QTest>

#include "MainFrame.h"

bool run( int argc, char* argv[] );

int main( int argc, char *argv[] )
{
    for( int i = 0; i < argc; i++ ) {
        if( strcmp( argv[i], "-xml" ) == 0 ) {
            return run( argc, argv );
        }
    }

    QApplication a( argc, argv );

    QCoreApplication::setOrganizationName( "aleksey-ka" );
    QCoreApplication::setApplicationName( "OpenAP-Capture" );

    MainFrame w;

    // Center main frame on the screen
    QRect screenRect = a.screens().first()->geometry();
    int x = ( screenRect.width() - w.width() ) / 2;
    int y = ( screenRect.height() - w.height() ) / 2;
    w.move( x, y );

    w.show();

    return a.exec();
}
