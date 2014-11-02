#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QObject>

class Arguments : public QObject
{
    Q_OBJECT

public:
    QString interfaceName;
    quint32 ulIntervalMs;

    explicit Arguments(QObject *parent = 0);
    Arguments(Arguments *arguments);

signals:

public slots:

};

#endif // ARGUMENTS_H
