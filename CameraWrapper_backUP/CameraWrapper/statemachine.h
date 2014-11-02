#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "arguments.h"

#include <QThread>

/*class Statemachine : public QObject
{
    Q_OBJECT

public:
    explicit Statemachine(QObject *parent = 0);

signals:

public slots:

};*/

class Statemachine : public QThread
{
    Q_OBJECT

public:
    Statemachine(Arguments *arguments);
    ~Statemachine();
    void run();

signals:
    //void resultReady(const QString &s);

private:
    Arguments *arguments;
    int sd;
    quint8 ucInterfaceIndex;

    void delay();
};


#endif // STATEMACHINE_H
