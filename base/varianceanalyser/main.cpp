#include <qapplication.h>
#include "FormSubvisionImpl.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    FormSubvisionImpl w(&a);
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
