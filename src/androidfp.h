#ifndef ANDROIDFP_H
#define ANDROIDFP_H

#include <QObject>
#include <QList>
#include <QString>

#include "biometry.h"

class AndroidFP : public QObject
{
    Q_OBJECT
public:
    explicit AndroidFP(QObject *parent = nullptr);
    void setGroup(uint32_t gid, const QString &storePath);
    void enroll(uid_t user_id);
    void remove(uid_t finger);
    void cancel();
    void authenticate();
    void enumerate();
    void clear();
    QList<uint32_t> fingerprints() const;

    static QString getDefaultGroupPath(uint32_t uid);

signals:
    void failed(const QString& message);
    void succeeded(uint32_t fingerId); //After enrollment
    void removed(uint32_t finger); //0 for clear
    void authenticated(uint32_t fingerId);
    void enrollProgress(float progress); //Progress is 0..1
    void acquired(int info);
    void enumerated(); // fingerprint list ready

private:
    void enumerateCallback(uint32_t finger, uint32_t remaining);
    void enrollCallback(uint32_t finger, uint32_t remaining);
    void removeCallback(uint32_t finger, uint32_t remaining);
    void acquiredCallback(UHardwareBiometryFingerprintAcquiredInfo info);

    static void enrollresult_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *);
    static void acquired_cb(uint64_t, UHardwareBiometryFingerprintAcquiredInfo, int32_t, void *);
    static void authenticated_cb(uint64_t, uint32_t, uint32_t, void *);
    static void removed_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *);
    static void enumerate_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context);
    static void error_cb(uint64_t, UHardwareBiometryFingerprintError error, int32_t vendorCode, void *context);

private:
    UHardwareBiometry m_biometry = nullptr;
    UHardwareBiometryParams fp_params;

    float m_enrollRemaining = 0.0;
    uint32_t m_removingFinger = 0;
    QList<uint32_t> m_fingers;
};

#endif // ANDROIDFP_H
