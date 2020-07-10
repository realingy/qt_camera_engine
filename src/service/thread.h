#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>

template<class T>
class Thread : public QThread
{
public:
    typedef void (T::*CallBack)();
    Thread(T* object, CallBack method):_pObject(object), _method(method),_stopped(true),_running(false)
    {
    }

    virtual ~Thread()
    {
        stop();
        wait();
    }

    void start()
    {
         QMutexLocker locker(&_mutex);
         if(!_running)
         {
             try
             {
                 QThread::start();
                 _stopped = false;
                 _running = true;
             }
             catch(...)
             {
                 _running = false;
                 throw;
             }
         }
    }

    void stop()
    {
        _stopped = true;
        _running = false;
        QMutexLocker locker(&_mutex);
    }

    void wait(long milliseconds)
    {
        if(_running)
        {
            QMutexLocker locker(&_mutex);
        }
    }

    void wait()
    {
        QMutexLocker locker(&_mutex);
    }

    bool isStopped() const
    {
        return _stopped;
    }

    bool isRunning() const
    {
        return _running;
    }

    void msleep(unsigned long microseconds)
    {
        QThread::msleep(microseconds);
    }

    void sleep(unsigned long sec)
    {
        QThread::sleep(sec);
    }

protected:
    void run()
    {
        QMutexLocker locker(&_mutex);
        try
        {
            (_pObject->*_method)();
        }

        catch(...)
        {
            _running = false;
            throw;
        }
        _running = false;
    }

private:
    T* _pObject;
    CallBack _method;
    volatile bool _stopped;
    volatile bool _running;
    QMutex _mutex;
};


#endif // THREAD_H
