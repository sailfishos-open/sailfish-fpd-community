#ifndef FPDBIOMETRYD_H
#define FPDBIOMETRYD_H

#include <QObject>
#include <QMetaEnum>
#include <QTimer>
#include <QDBusMessage>

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

    enum Error {
        FPERROR_UNKNOWN,
        FPERROR_NONE,
        FPERROR_HW_UNAVAILABLE,
        FPERROR_UNABLE_TO_PROCESS,
        FPERROR_TIMEOUT,
        FPERROR_NO_SPACE,
        FPERROR_ABORTED,
        FPERROR_UNABLE_TO_REMOVE,
        FPERROR_UNSPECIFIED
    };
    Q_ENUM(Error)

    enum Reply
    {
        /** Operation successfully started */
        FPREPLY_STARTED                  = 0,

        /** Unspecified (low level) failure */
        FPREPLY_FAILED                   = 1,

        /** Abort() while already idle */
        FPREPLY_ALREADY_IDLE             = 2,

        /** Abort/Enroll/Identify() while busy */
        FPREPLY_ALREADY_BUSY             = 3,

        /** Not allowed  */
        FPREPLY_DENIED                   = 4,

        /** Enroll() key that already exists */
        FPREPLY_KEY_ALREADY_EXISTS       = 5,

        /** Remove() key that does not exist */
        FPREPLY_KEY_DOES_NOT_EXIST       = 6,

        /** Identify() without having any keys */
        FPREPLY_NO_KEYS_AVAILABLE        = 7,

        /** Null or otherwise illegal key name */
        FPREPLY_KEY_IS_INVALID           = 8,
    };
    Q_ENUM(Reply)

    FPDCommunity();

    /* ========================================================================= *
     * FINGERPRINT_DAEMON_DBUS_SERVICE (API Version 1)
     * ========================================================================= */
    Q_INVOKABLE int Enroll(const QString &finger, const QDBusMessage &message);
    Q_INVOKABLE int Identify(const QDBusMessage &message);
    Q_INVOKABLE QString GetState();
    Q_INVOKABLE QStringList GetAll(); //Returns list of templates in store
    Q_INVOKABLE int Abort(const QDBusMessage &message);
    Q_INVOKABLE int Verify(const QDBusMessage &message);
    Q_INVOKABLE int Remove(const QString &finger, const QDBusMessage &message);

    Q_SIGNAL void Added(const QString &finger);
    Q_SIGNAL void Removed(const QString &finger);
    Q_SIGNAL void Identified(const QString &finger);
    Q_SIGNAL void Aborted();
    Q_SIGNAL void Failed();
    Q_SIGNAL void Verified();

    Q_SIGNAL void StateChanged(const QString &state);
    Q_SIGNAL void EnrollProgressChanged(int progress);
    Q_SIGNAL void AcquisitionInfo(const QString &info);
    Q_SIGNAL void ErrorInfo(const QString &error);
    Q_SIGNAL void ListChanged();

    /* ========================================================================= */

    /* Community DBUS API additions */
    Q_INVOKABLE void Clear(const QDBusMessage &message);

private slots:
    void slot_enrollProgress(float pc);
    void slot_succeeded(uint32_t finger);
    void slot_failed(const QString &message);
    void slot_acquired(int info); //Convert the android Acquired State to SFOS state
    void slot_removed(uint32_t finger);
    void slot_authenticated(uint32_t finger);
    void slot_cancelIdentify();
    void slot_enumerated();

private:
    void abort();
    void enumerate();
    void setUser(uint32_t uid);

private:
    AndroidFP m_androidFP;
    QTimer m_cancelTimer;

    bool m_dbusRegistered = false;
    QString m_dbusCaller;

    QString m_fingerDatabasePath;
    QMap<uint32_t, QString> m_fingerMap;

    State m_state = FPSTATE_IDLE;
    AcquiredState m_acquired = FPACQUIRED_UNSPECIFIED;
    QString m_addingFinger;

    void setState(State newState);
    void registerDBus();
    void saveFingers();
    void loadFingers();
};

#endif // FPDBIOMETRYD_H
