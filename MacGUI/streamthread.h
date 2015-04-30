#ifndef STREAMTHREAD_H
#define STREAMTHREAD_H

#include <QtCore>
#include <QFileDialog>

class StreamThread : public QThread
{
public:
    StreamThread();
    void run();
};

#endif // STREAMTHREAD_H
