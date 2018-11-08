/* Implementation of FrameReader */

#include <qapplication.h>
#include "FrameReader.h"
#include "FormSubvisionImpl.h"

void FrameReader::run(void) {
    int frameid;
    while ((frameid = display->readFrame()) != -1) {
        //display->sync.wakeOne();
        display->app->lock();
        display->slotRedraw();
        display->app->unlock();
//        emit redraw();
        display->clearData();
    }
}
