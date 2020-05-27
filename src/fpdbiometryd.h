#ifndef FPDBIOMETRYD_H
#define FPDBIOMETRYD_H

#include <QObject>
#include "androidfp.h"

#define SERVICE_NAME "org.sailfishos.fingerprint1"

class FPDBiometryd : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    FPDBiometryd();
    Q_INVOKABLE void Enroll(const QString &finger);
    Q_INVOKABLE void Identify();

signals:
    void StateChanged(const QString &state);
    void EnrollProgressChanged(int32_t progress);
    void AcquisitionInfo(const QString &info);
    void Added(const QString &finger);
    void Identified(const QString &finger);


private:
    AndroidFP m_androidFP;
};

#endif // FPDBIOMETRYD_H
