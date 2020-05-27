#include "androidfp.h"
#include "util/property_store.h"
#include <QDebug>

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
    util::AndroidPropertyStore store;
    UHardwareBiometryRequestStatus ret = SYS_OK;
    std::string api_level = store.get("ro.product.first_api_level");
    if (api_level.empty())
        api_level = store.get("ro.build.version.sdk");
    if (atoi(api_level.c_str()) <= 27)
        ret = u_hardware_biometry_setActiveGroup(m_biometry, 0, (char*)"/data/system/users/0/fpdata/");
    else
        ret = u_hardware_biometry_setActiveGroup(m_biometry, 0, (char*)"/data/vendor_de/0/fpdata/");
    if (ret != SYS_OK)
        printf("setActiveGroup failed: %s\n", IntToStringRequestStatus(ret).c_str());

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

void AndroidFP::enroll(uid_t user_id)
{
    qDebug() << "AndroidFP::enroll:" << user_id;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_enroll(m_biometry, 0, 60, user_id);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::remove(uid_t finger)
{
    qDebug() << "AndroidFP::remove:" << finger;
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_remove(m_biometry, 0, finger);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::cancel()
{
    qDebug() << "AndroidFP::cancel";
    u_hardware_biometry_cancel(m_biometry);
}

void AndroidFP::authenticate()
{
    qDebug() << "AndroidFP::authenticate";
    UHardwareBiometryRequestStatus ret = u_hardware_biometry_authenticate(m_biometry, 0, 0);
    if (ret != SYS_OK) {
        failed(QString::fromUtf8(IntToStringRequestStatus(ret).data()));
    }
}

void AndroidFP::enrollresult_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *)
{

}

void AndroidFP::acquired_cb(uint64_t, UHardwareBiometryFingerprintAcquiredInfo, int32_t, void *)
{

}

void AndroidFP::authenticated_cb(uint64_t, uint32_t fingerId, uint32_t, void *context)
{
    qDebug() << "AndroidFP::authenticated_cb:" << fingerId;

    if (fingerId != 0)
        static_cast<AndroidFP*>(context)->authenticated(fingerId);
    else {
        static_cast<AndroidFP*>(context)->failed("FINGER_NOT_RECOGNIZED");
    }
}

void AndroidFP::removed_cb(uint64_t, uint32_t, uint32_t, uint32_t, void *)
{

}

void AndroidFP::enumerate_cb(uint64_t, uint32_t fingerId, uint32_t, uint32_t remaining, void *context)
{

}

void AndroidFP::error_cb(uint64_t, UHardwareBiometryFingerprintError error, int32_t vendorCode, void *context)
{
    qDebug() << "AndroidFP::error_cb:" << error << vendorCode;
    if (error == 0) {
        return;
    }
    static_cast<AndroidFP*>(context)->failed(QString::fromUtf8(IntToStringFingerprintError(error, vendorCode).data()));
}
