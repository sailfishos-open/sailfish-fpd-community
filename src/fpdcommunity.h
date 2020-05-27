#ifndef FPDBIOMETRYD_H
#define FPDBIOMETRYD_H

#include <QObject>
#include "androidfp.h"

#define SERVICE_NAME "org.sailfishos.fingerprint1"

class FPDCommunity : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    FPDCommunity();
    Q_INVOKABLE void Enroll(const QString &finger);
    Q_INVOKABLE void Identify();
    Q_INVOKABLE void Clear();

signals:
    void StateChanged(const QString &state);
    void EnrollProgressChanged(int progress);
    void AcquisitionInfo(const QString &info);
    void Added(const QString &finger);
    void Identified(const QString &finger);

private slots:
    void slot_enrollProgres(float pc);
    void slot_succeeded(int finger);

private:
    AndroidFP m_androidFP;
    bool m_dbusRegistered = false;

    void registerDBus();
};

#endif // FPDBIOMETRYD_H
