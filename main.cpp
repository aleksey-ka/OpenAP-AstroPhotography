#include <QApplication>
#include <QDesktopWidget>

#include "mainframe.h"

int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    MainFrame w;

    // Center main frame on the screen
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int x = ( screenGeometry.width() - w.width() ) / 2;
    int y = ( screenGeometry.height() - w.height() ) / 2;
    w.move( x, y );

    w.show();

    return a.exec();
}
