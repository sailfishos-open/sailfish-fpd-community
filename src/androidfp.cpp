#include "androidfp.h"
#include "util/property_store.h"

#include <QDebug>

#include <limits.h>
#include <cstring>

std::string IntToStringFingerprintError(int error, int vendorCode){
    switch(error) {
        case ERROR_NO_ERROR: return "ERROR_NO_ERROR";
        case ERROR_HW_UNAVAILABLE: return "ERROR_HW_UNAVAILABLE";
        case ERROR_UNABLE_TO_PROCESS: return "ERROR_UNABLE_TO_PROCESS";
        case ERROR_TIMEOUT: return "ERROR_TIMEOUT";
        case ERROR_NO_SPACE: return "ERROR_NO_SPACE";
        case ERROR_CANCELED: return "ERROR_CANCELED";
        case ERROR_UNABLE_TO_REMOVE: return "ERROR_UNABLE_TO_REMOVE";
        case ERROR_LOCKOUT: return "ERROR_LOCKOUT";
        case ERROR_VENDOR: return "ERROR_VENDOR: " + std::to_string(vendorCode);
        default:
            return "ERROR_NO_ERROR";
    }
}

std::string IntToStringRequestStatus(int error){
    switch(error) {
        case SYS_UNKNOWN: return "SYS_UNKNOWN";
        case SYS_OK: return "SYS_OK";
        case SYS_ENOENT: return "SYS_ENOENT";
        case SYS_EINTR: return "SYS_EINTR";
        case SYS_EIO: return "SYS_EIO";
        case SYS_EAGAIN: return "SYS_EAGAIN";
        case SYS_ENOMEM: return "SYS_ENOMEM";
        case SYS_EACCES: return "SYS_EACCES";
        case SYS_EFAULT: return "SYS_EFAULT";
        case SYS_EBUSY: return "SYS_EBUSY";
        case SYS_EINVAL: return "SYS_EINVAL";
        case SYS_ENOSPC: return "SYS_ENOSPC";
        case SYS_ETIMEDOUT: return "SYS_ETIMEDOUT";
        default:
            return "SYS_OK";
    }
}

AndroidFP::AndroidFP(QObject *parent) : QObject(parent), m_biometry(u_hardware_biometry_new())
{
    //Set up the callbacks
    fp_params.enrollresult_cb = enrollresult_cb;
    fp_params.acquired_cb = acquired_cb;
    fp_params.authenticated_cb = authenticated_cb;
    fp_params.error_cb = error_cb;
    fp_params.removed_cb = removed_cb;
    fp_params.enumerate_cb = enumerate_cb;
    fp_params.context = this;
    u_hardware_biometry_setNotify(m_biometry, &fp_params);
}

QString AndroidFP::getDefaultGroupPath(uint32_t uid)
{
    util::AndroidPropertyStore store;
    std::string api_level = store.get("ro.product.first_api_level");
    if (api_level.empty()) {
      api_level = store.get("ro.build.version.sdk");
    }
    if (atoi(api_level.c_str()) <= 27) {
      return QStringLiteral("/data/system/users/%1/fpdata").arg(uid);
    }
    return QStringLiteral("/data/vendor_de/%1/fpdata").arg(uid);
}

void AndroidFP::setGroup(uint32_t gid, const QString &storePath)
{
    qDebug() << Q_FUNC_INFO << gid << storePath;
    UHardwareBiometryRequestStatus ret = SYS_OK;
    char path[PATH_MAX];

    if (storePath.isEmpty()) {
        qWarning() << "Cannot set empty active group path";
        failed(QStringLiteral("Cannot set empty active group path"));
        return;
    }

    std::strncpy(path, storePath.toLocal8Bit().data(), sizeof(path));
    path[ sizeof(path)-1 ] = 0;
    ret = u_hardware_biometry_setActiveGroup(m_biometry, gid, path);

    if (ret != SYS_OK) {
        qWarning() << "setActiveGroup failed: " << IntToStringRequestStatus(ret).c_str();
        failed(QStringLiteral("setActiveGroup failed: %1").arg(IntToStringRequestStatus(ret).c_str()));
    } else {
        qInfo() << "setActiveGroup to" << path;
    }
}

void AndroidFP::enroll(uid_t user_id)
{
    qDebug() << Q_FUNC_INFO << user_id;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_enroll(m_biometry, 0, 60, user_id);
    m_enrollRemaining = 0;
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::remove(uid_t finger)
{
    qDebug() << Q_FUNC_INFO << finger;
    m_removingFinger = finger;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_remove(m_biometry, 0, finger);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::cancel()
{
    qDebug() << Q_FUNC_INFO;
    u_hardware_biometry_cancel(m_biometry);
}

void AndroidFP::authenticate()
{
    qDebug() << Q_FUNC_INFO;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_authenticate(m_biometry, 0, 0);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::enumerate()
{
    qDebug() << Q_FUNC_INFO;
    m_fingers.clear();
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_enumerate(m_biometry);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::clear()
{
    qDebug() << Q_FUNC_INFO;
    m_removingFinger = 0;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_remove(m_biometry, 0, 0);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

QList<uint32_t> AndroidFP::fingerprints() const
{
    qDebug() << Q_FUNC_INFO;
    return m_fingers;
}

void AndroidFP::enumerateCallback(uint32_t finger, uint32_t remaining)
{
    qDebug() << Q_FUNC_INFO << finger << remaining;
    if (finger != 0)
        m_fingers.push_back(finger);
    if (remaining == 0)
        emit enumerated();
}

void AndroidFP::enrollCallback(uint32_t finger, uint32_t remaining)
{
    qDebug() << Q_FUNC_INFO << finger << remaining;

    if (remaining > 0) {
        if (m_enrollRemaining == 0) {
            m_enrollRemaining = remaining + 1;
        }
        float pc = (1.0 - (remaining / m_enrollRemaining));
        emit enrollProgress(pc);
    } else {
        emit enrollProgress(1);
        UHardwareBiometryRequestStatus ret = u_hardware_biometry_postEnroll(m_biometry);
        if (ret == SYS_OK) {
            succeeded(finger);
        } else {
            failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
        }
    }
}

void AndroidFP::removeCallback(uint32_t finger, uint32_t remaining)
{
    qDebug() << Q_FUNC_INFO << finger << remaining;

    if ((finger == m_removingFinger || m_removingFinger == 0) && remaining == 0) {
        if (m_removingFinger == 0)
            finger = 0;
        emit removed(finger);
        m_removingFinger = 0;
    }
}

void AndroidFP::acquiredCallback(UHardwareBiometryFingerprintAcquiredInfo info)
{
    qDebug() << Q_FUNC_INFO << info;
    emit acquired(info);
}

void AndroidFP::enrollresult_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context)
{
    qDebug() << Q_FUNC_INFO << fingerId << remaining;
    static_cast<AndroidFP*>(context)->enrollCallback(fingerId, remaining);
}

void AndroidFP::acquired_cb(uint64_t, UHardwareBiometryFingerprintAcquiredInfo info, int32_t, void *context)
{
    qDebug() << Q_FUNC_INFO << info;
    static_cast<AndroidFP*>(context)->acquiredCallback(info);
}

void AndroidFP::authenticated_cb(uint64_t, uint32_t fingerId, uint32_t, void *context)
{
    qDebug() << Q_FUNC_INFO << fingerId;

    if (fingerId != 0)
        static_cast<AndroidFP*>(context)->authenticated(fingerId);
    else {
        static_cast<AndroidFP*>(context)->failed("FINGER_NOT_RECOGNIZED");
    }
}

void AndroidFP::removed_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context)
{
    qDebug() << Q_FUNC_INFO << fingerId << remaining;

    static_cast<AndroidFP*>(context)->removeCallback(fingerId, remaining);
}

void AndroidFP::enumerate_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context)
{
    qDebug() << Q_FUNC_INFO << fingerId << remaining;

    static_cast<AndroidFP*>(context)->enumerateCallback(fingerId, remaining);
}

void AndroidFP::error_cb(uint64_t, UHardwareBiometryFingerprintError error, int32_t vendorCode, void *context)
{
    qDebug() << Q_FUNC_INFO << error << vendorCode;
    if (error == 0) {
        return;
    }
    static_cast<AndroidFP*>(context)->failed(QString::fromUtf8(IntToStringFingerprintError(error, vendorCode).data()));
}
