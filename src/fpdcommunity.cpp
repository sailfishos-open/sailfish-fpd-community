#include "fpdcommunity.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QCoreApplication>

FPDCommunity::FPDCommunity()
{
    qDebug() << Q_FUNC_INFO;
    connect(&m_androidFP, &AndroidFP::enrollProgress, this, &FPDCommunity::slot_enrollProgres);
    connect(&m_androidFP, &AndroidFP::succeeded, this, &FPDCommunity::slot_succeeded);

    registerDBus();
}

void FPDCommunity::registerDBus()
{
    qDebug() << Q_FUNC_INFO;
    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::systemBus();
        qDebug() << "Registering service on dbus" << SERVICE_NAME;
        if (!connection.registerService(SERVICE_NAME))
        {
            qDebug() << "Didnt register service";
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject("/", this, QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties))
        {
            qDebug() << "Didnt register object"
                        "";
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        qDebug() << "Sucessfully registered to dbus systemBus";
    }
}

void FPDCommunity::Enroll(const QString &finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    m_androidFP.enroll(100000); //nemo userID
}

void FPDCommunity::Identify()
{
    qDebug() << Q_FUNC_INFO;
    m_androidFP.authenticate();
}

void FPDCommunity::Clear()
{
    qDebug() << Q_FUNC_INFO;
    m_androidFP.clear();
}

void FPDCommunity::slot_enrollProgres(float pc)
{
    qDebug() << Q_FUNC_INFO << pc;
    emit EnrollProgressChanged(pc * 100);
}

void FPDCommunity::slot_succeeded(int finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    emit Added(QString::number(finger));
}

