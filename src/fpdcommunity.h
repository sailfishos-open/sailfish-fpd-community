#ifndef FPDBIOMETRYD_H
#define FPDBIOMETRYD_H

#include <QObject>
#include <QMetaEnum>
#include "androidfp.h"

#define SERVICE_NAME "org.sailfishos.fingerprint1"

template<typename QEnum>
QString QtEnumToString (const QEnum value)
{
  return QMetaEnum::fromType<QEnum>().valueToKey(value);
}

class FPDCommunity : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    enum State {
        FPSTATE_UNKNOWN,
        FPSTATE_UNSET,
        FPSTATE_IDLE,
        FPSTATE_ENROLLING,
        FPSTATE_IDENTIFYING,
        FPSTATE_ENUMERATING,
        FPSTATE_REMOVING,
        FPSTATE_VERIFYING,
        FPSTATE_ABORTING,
        FPSTATE_TERMINATING
    };
    Q_ENUM(State)

    enum AcquiredState {
        FPACQUIRED_UNKNOWN,
        FPACQUIRED_GOOD,
        FPACQUIRED_PARTIAL,
        FPACQUIRED_INSUFFICIENT,
        FPACQUIRED_IMAGER_DIRTY,
        FPACQUIRED_TOO_SLOW,
        FPACQUIRED_TOO_FAST,
        FPACQUIRED_UNSET,
        FPACQUIRED_UNSPECIFIED,
        FPACQUIRED_UNRECOGNIZED,
    };
    Q_ENUM(AcquiredState)

    FPDCommunity();
    Q_INVOKABLE void Enroll(const QString &finger);
    Q_INVOKABLE void Identify();
    Q_INVOKABLE void Clear();
    Q_INVOKABLE QString GetState();
    Q_INVOKABLE QStringList GetAll(); //Returns list of templates in store
    Q_INVOKABLE void Abort();

signals:
    void StateChanged(const QString &state);
    void EnrollProgressChanged(int progress);
    void AcquisitionInfo(const QString &info);
    void Added(const QString &finger);
    void Identified(const QString &finger);

private slots:
    void slot_enrollProgress(float pc);
    void slot_succeeded(int finger);
    void slot_failed(const QString &message);
    void slot_acquired(int info); //Convert the android Acquired State to SFOS state
    void slot_removed(int finger);
    void slot_authenticated(int finger);
    void slot_cancelIdentify();

private:
    AndroidFP m_androidFP;
    bool m_dbusRegistered = false;
    State m_state = FPSTATE_IDLE;
    AcquiredState m_acquired = FPACQUIRED_UNSPECIFIED;

    void setState(State newState);
    void registerDBus();
};

#endif // FPDBIOMETRYD_H
