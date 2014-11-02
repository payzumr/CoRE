#include "arguments.h"

Arguments::Arguments(QObject *parent) :
    QObject(parent)
{
}

Arguments::Arguments(Arguments *arguments) :
    QObject()
{
    this->interfaceName = arguments->interfaceName;
    this->ulIntervalMs = arguments->ulIntervalMs;
}
