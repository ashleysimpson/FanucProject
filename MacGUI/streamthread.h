#ifndef STREAMTHREAD_H
#define STREAMTHREAD_H
#include <QtCore>

class StreamThread : public QThread
{
public:
    StreamThread();
    void run();
};

#endif // STREAMTHREAD_H
