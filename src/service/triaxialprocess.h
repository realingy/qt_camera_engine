#ifndef TRIAXIALPROCESS_H
#define TRIAXIALPROCESS_H

#include <QObject>
#include "thread.h"

enum EnumDirection {
    DIR_X = 0,
    DIR_Y,
    DIR_Z,
};

class TriaxialProcess : public QObject
{
    Q_OBJECT
public:
    explicit TriaxialProcess(QObject *parent = 0);

    void eventProc();

signals:
    void sigTriaxial(EnumDirection dir, qlonglong value);

public slots:

private:
    Thread<TriaxialProcess> m_thread;

    qlonglong m_value;

    EnumDirection m_direction;

    int m_fd;

};

#endif // TRIAXIALPROCESS_H
