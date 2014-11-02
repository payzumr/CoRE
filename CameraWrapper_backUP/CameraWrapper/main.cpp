#include "statemachine.h"
#include "arguments.h"

#include <QCoreApplication>
#include <QString>
#include "version.h"


#define ARGS_POS_INTERFACE           1
#define ARGS_POS_INTERVAL            2
//#define ARGS_POS_CT_IDENTIFIER       2
//#define ARGS_POS_CT_MASK             3
#define ARGS_STATIC_COUNT            3

#define INTERVAL_DEFAULT        1000
#define INTERVAL_MIN              10


//quint32 ulCtIdentifier;

int main(int argc, char *argv[])
{
    int sReturnVal;

    QCoreApplication a(argc, argv);
    if (argc < ARGS_STATIC_COUNT)
    {
        qCritical("Usage: ./CameraWrapper interface interval(ms)");
        a.exit();
        return(EXIT_FAILURE);
    }
    Arguments *arguments = new Arguments();
    // Get interface name
    arguments->interfaceName = argv[ARGS_POS_INTERFACE];
    arguments->ulIntervalMs = QString(argv[ARGS_POS_INTERVAL]).toInt();
    if (arguments->ulIntervalMs < INTERVAL_MIN)
    {
        qWarning("Warning: Interval out of bounds...resetting to 1000ms");
        arguments->ulIntervalMs = INTERVAL_DEFAULT;
    }

    // init muEye
    // ----------
    // check libueye_api Version if working with demo
    /*
     *DWORD ver = is_GetDLLVersion();
    bool bDllVersion = false;
    int nDllMajor = ver >> 24;
    int nDllMinor = ver >> 16 & 0xff;
    if (nDllMajor > DLL_VER_MAJOR)
        bDllVersion = true;
    else if (nDllMajor == DLL_VER_MAJOR)
    {
        if (nDllMinor >= DLL_VER_MINOR)
            bDllVersion = true;
    }

    // if DLL version is to old, show a user message and exit the program
    if (!bDllVersion)
    {
        qCritical(QString("Version of library found is not compatible! libueye_api < %1.%2".arg(DLL_VER_MAJOR).arg(DLL_VER_MINOR)));
        return 0;
    }*/

    Statemachine *threadStatemachine = new Statemachine(arguments);
    //connect(threadStatemachine, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));
    //connect(threadStatemachine, SIGNAL(finished()), threadStatemachine, SLOT(deleteLater()));
    threadStatemachine->start();


    sReturnVal = a.exec();
    // cleanup
    delete arguments;
    // close program
    return sReturnVal;
}
