#ifndef ANDROIDFP_H
#define ANDROIDFP_H

#include <QObject>
#include "biometry.h"

class AndroidFP : public QObject
{
    Q_OBJECT
public:
    explicit AndroidFP(QObject *parent = nullptr);
    void enroll(uid_t user_id);
    void remove(uid_t finger);

signals:
    void failed(const QString& message);

private:
    UHardwareBiometry m_biometry = nullptr;
    UHardwareBiometryParams fp_params;

    static void enrollresult_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *);
    static void acquired_cb(uint64_t, UHardwareBiometryFingerprintAcquiredInfo, int32_t, void *);
    static void authenticated_cb(uint64_t, uint32_t, uint32_t, void *);
    static void removed_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *);
    static void enumerate_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context);
    static void error_cb(uint64_t, UHardwareBiometryFingerprintError error, int32_t vendorCode, void *context);
};

#endif // ANDROIDFP_H
