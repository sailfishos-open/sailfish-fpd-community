/*
 * Copyright © 2020 UBports foundation Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Erfan Abdi <erfangplus@gmail.com>
 */
#include <biometry.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>

// android stuff
#include <android/hardware/biometrics/fingerprint/2.1/IBiometricsFingerprint.h>
#include <android/hardware/gatekeeper/1.0/IGatekeeper.h>

#include <utils/Log.h>

using android::OK;
using android::sp;
using android::wp;
using android::status_t;

using android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint;
using android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using android::hardware::gatekeeper::V1_0::IGatekeeper;
using android::hardware::gatekeeper::V1_0::GatekeeperStatusCode;
using android::hardware::gatekeeper::V1_0::GatekeeperResponse;
using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;

using android::hidl::base::V1_0::IBase;

sp<IBiometricsFingerprint> fpHal = nullptr;

struct UHardwareBiometry_
{
    UHardwareBiometry_();
    ~UHardwareBiometry_();

    bool init();

    uint64_t setNotify();
    uint64_t preEnroll();
    UHardwareBiometryRequestStatus enroll(uint32_t gid, uint32_t timeoutSec, uint32_t uid);
    UHardwareBiometryRequestStatus postEnroll();
    uint64_t getAuthenticatorId();
    UHardwareBiometryRequestStatus cancel();
    UHardwareBiometryRequestStatus enumerate();
    UHardwareBiometryRequestStatus remove(uint32_t gid, uint32_t fid);
    UHardwareBiometryRequestStatus setActiveGroup(uint32_t gid, char *storePath);
    UHardwareBiometryRequestStatus authenticate(uint64_t operationId, uint32_t gid);
};

struct UHardwareBiometryCallback_
{
    UHardwareBiometryCallback_(UHardwareBiometryParams* params);
    
    UHardwareBiometryEnrollResult enrollresult_cb;
    UHardwareBiometryAcquired acquired_cb;
    UHardwareBiometryAuthenticated authenticated_cb;
    UHardwareBiometryError error_cb;
    UHardwareBiometryRemoved removed_cb;
    UHardwareBiometryEnumerate enumerate_cb;
    
    void* context;
};


namespace
{
UHardwareBiometry hybris_fp_instance = NULL;
UHardwareBiometryCallback hybris_fp_instance_cb = NULL;

struct BiometricsFingerprintClientCallback : public IBiometricsFingerprintClientCallback {
    Return<void> onEnrollResult(uint64_t deviceId, uint32_t fingerId,
                                uint32_t groupId, uint32_t remaining) override;
    Return<void> onAcquired(uint64_t deviceId, FingerprintAcquiredInfo acquiredInfo,
                            int32_t vendorCode) override;
    Return<void> onAuthenticated(uint64_t deviceId, uint32_t fingerId, uint32_t groupId,
                                 const hidl_vec<uint8_t>& token) override;
    Return<void> onError(uint64_t deviceId, FingerprintError error,
                         int32_t vendorCode) override;
    Return<void> onRemoved(uint64_t deviceId, uint32_t fingerId,
                           uint32_t groupId, uint32_t remaining) override;
    Return<void> onEnumerate(uint64_t deviceId, uint32_t fingerId,
                             uint32_t groupId, uint32_t remaining) override;
};
    
UHardwareBiometryFingerprintAcquiredInfo HIDLToUFingerprintAcquiredInfo(FingerprintAcquiredInfo info) {
    switch(info) {
        case FingerprintAcquiredInfo::ACQUIRED_GOOD: return ACQUIRED_GOOD;
        case FingerprintAcquiredInfo::ACQUIRED_PARTIAL: return ACQUIRED_PARTIAL;
        case FingerprintAcquiredInfo::ACQUIRED_INSUFFICIENT: return ACQUIRED_INSUFFICIENT;
        case FingerprintAcquiredInfo::ACQUIRED_IMAGER_DIRTY: return ACQUIRED_IMAGER_DIRTY;
        case FingerprintAcquiredInfo::ACQUIRED_TOO_SLOW: return ACQUIRED_TOO_SLOW;
        case FingerprintAcquiredInfo::ACQUIRED_TOO_FAST: return ACQUIRED_TOO_FAST;
        case FingerprintAcquiredInfo::ACQUIRED_VENDOR: return ACQUIRED_VENDOR;
        default:
            return ACQUIRED_GOOD;
    }
}
    
UHardwareBiometryFingerprintError HIDLToUFingerprintError(FingerprintError error) {
    switch(error) {
        case FingerprintError::ERROR_NO_ERROR: return ERROR_NO_ERROR;
        case FingerprintError::ERROR_HW_UNAVAILABLE: return ERROR_HW_UNAVAILABLE;
        case FingerprintError::ERROR_UNABLE_TO_PROCESS: return ERROR_UNABLE_TO_PROCESS;
        case FingerprintError::ERROR_TIMEOUT: return ERROR_TIMEOUT;
        case FingerprintError::ERROR_NO_SPACE: return ERROR_NO_SPACE;
        case FingerprintError::ERROR_CANCELED: return ERROR_CANCELED;
        case FingerprintError::ERROR_UNABLE_TO_REMOVE: return ERROR_UNABLE_TO_REMOVE;
        case FingerprintError::ERROR_LOCKOUT: return ERROR_LOCKOUT;
        case FingerprintError::ERROR_VENDOR: return ERROR_VENDOR;
        default:
            return ERROR_NO_ERROR;
    }
}

Return<void> BiometricsFingerprintClientCallback::onEnrollResult(uint64_t deviceId,
    uint32_t fingerId, uint32_t groupId, uint32_t remaining)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->enrollresult_cb) {
        hybris_fp_instance_cb->enrollresult_cb(deviceId, fingerId, groupId, remaining, hybris_fp_instance_cb->context);
    }
    return Void();
}

Return<void> BiometricsFingerprintClientCallback::onAcquired(uint64_t deviceId, FingerprintAcquiredInfo acquiredInfo,
    int32_t vendorCode)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->acquired_cb) {
        hybris_fp_instance_cb->acquired_cb(deviceId, HIDLToUFingerprintAcquiredInfo(acquiredInfo), vendorCode, hybris_fp_instance_cb->context);
    }
    return Void();
}

Return<void> BiometricsFingerprintClientCallback::onAuthenticated(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, const hidl_vec<uint8_t>& token)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->authenticated_cb) {
        hybris_fp_instance_cb->authenticated_cb(deviceId, fingerId, groupId, hybris_fp_instance_cb->context);
    }
    return Void();
}

Return<void> BiometricsFingerprintClientCallback::onError(uint64_t deviceId, FingerprintError error, int32_t vendorCode)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->error_cb) {
        hybris_fp_instance_cb->error_cb(deviceId, HIDLToUFingerprintError(error), vendorCode, hybris_fp_instance_cb->context);
    }
    return Void();
}

Return<void> BiometricsFingerprintClientCallback::onRemoved(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->removed_cb) {
        hybris_fp_instance_cb->removed_cb(deviceId, fingerId, groupId, remaining, hybris_fp_instance_cb->context);
    }
    return Void();
}

Return<void> BiometricsFingerprintClientCallback::onEnumerate(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining)
{
    if (hybris_fp_instance_cb && hybris_fp_instance_cb->enumerate_cb) {
        hybris_fp_instance_cb->enumerate_cb(deviceId, fingerId, groupId, remaining, hybris_fp_instance_cb->context);
    }
    return Void();
}

}

UHardwareBiometry_::UHardwareBiometry_()
{

}

UHardwareBiometry_::~UHardwareBiometry_()
{
    if (fpHal != nullptr) {
        fpHal->cancel();
    }
}

UHardwareBiometryCallback_::UHardwareBiometryCallback_(UHardwareBiometryParams* params)
    : enrollresult_cb(params->enrollresult_cb),
      acquired_cb(params->acquired_cb),
      authenticated_cb(params->authenticated_cb),
      error_cb(params->error_cb),
      removed_cb(params->removed_cb),
      enumerate_cb(params->enumerate_cb),
      context(params->context)
{

}

UHardwareBiometryRequestStatus HIDLToURequestStatus(RequestStatus req) {
    switch(req) {
        case RequestStatus::SYS_UNKNOWN: return SYS_UNKNOWN;
        case RequestStatus::SYS_OK: return SYS_OK;
        case RequestStatus::SYS_ENOENT: return SYS_ENOENT;
        case RequestStatus::SYS_EINTR: return SYS_EINTR;
        case RequestStatus::SYS_EIO: return SYS_EIO;
        case RequestStatus::SYS_EAGAIN: return SYS_EAGAIN;
        case RequestStatus::SYS_ENOMEM: return SYS_ENOMEM;
        case RequestStatus::SYS_EACCES: return SYS_EACCES;
        case RequestStatus::SYS_EFAULT: return SYS_EFAULT;
        case RequestStatus::SYS_EBUSY: return SYS_EBUSY;
        case RequestStatus::SYS_EINVAL: return SYS_EINVAL;
        case RequestStatus::SYS_ENOSPC: return SYS_ENOSPC;
        case RequestStatus::SYS_ETIMEDOUT: return SYS_ETIMEDOUT;
        default:
            return SYS_UNKNOWN;
    }
}

bool UHardwareBiometry_::init()
{
    /* Initializes the FP service handle. */
    fpHal = IBiometricsFingerprint::getService();

    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return false;
    }

    return true;
}

uint64_t UHardwareBiometry_::setNotify()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return 0;
    }

    sp<IBiometricsFingerprintClientCallback> fpCbIface = new BiometricsFingerprintClientCallback();
    return fpHal->setNotify(fpCbIface);
}

uint64_t UHardwareBiometry_::preEnroll()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return 0;
    }

    return fpHal->preEnroll();
}

UHardwareBiometryRequestStatus UHardwareBiometry_::enroll(uint32_t gid, uint32_t timeoutSec, uint32_t user_id)
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    uint8_t *auth_token;
    uint32_t auth_token_len;
    int ret = 0;
    uint64_t challange = fpHal->preEnroll();
    std::string Password = "default_password";
    bool request_reenroll = false;
    sp<IGatekeeper> gk_device;
    gk_device = IGatekeeper::getService();
    if (gk_device == nullptr) {
        ALOGE("Unable to get Gatekeeper service\n");
        return SYS_UNKNOWN;
    }
    hidl_vec<uint8_t> curPwdHandle;
    hidl_vec<uint8_t> enteredPwd;
    enteredPwd.setToExternal(const_cast<uint8_t *>((const uint8_t *)Password.c_str()), Password.size());

    Return<void> hwRet =
    gk_device->enroll(user_id, NULL,
                      NULL,
                      enteredPwd,
                      [&ret, &curPwdHandle]
                      (const GatekeeperResponse &rsp) {
                          ret = static_cast<int>(rsp.code); // propagate errors
                          if (rsp.code >= GatekeeperStatusCode::STATUS_OK) {
                              curPwdHandle.setToExternal(const_cast<uint8_t *>((const uint8_t *)rsp.data.data()), rsp.data.size());
                              ret = 0; // all success states are reported as 0
                          } else if (rsp.code == GatekeeperStatusCode::ERROR_RETRY_TIMEOUT && rsp.timeout > 0) {
                              ret = rsp.timeout;
                          }
                      }
                      );
    if (!hwRet.isOk()) {
        ALOGE("Unable to Enroll on Gatekeeper\n");
        return SYS_UNKNOWN;
    }

    hwRet =
    gk_device->verify(user_id, challange,
                      curPwdHandle,
                      enteredPwd,
                      [&ret, &request_reenroll, &auth_token, &auth_token_len]
                      (const GatekeeperResponse &rsp) {
                          ret = static_cast<int>(rsp.code); // propagate errors
                          if (rsp.code >= GatekeeperStatusCode::STATUS_OK) {
                              auth_token = new uint8_t[rsp.data.size()];
                              auth_token_len = rsp.data.size();
                              memcpy(auth_token, rsp.data.data(), auth_token_len);
                              request_reenroll = (rsp.code == GatekeeperStatusCode::STATUS_REENROLL);
                              ret = 0; // all success states are reported as 0
                          } else if (rsp.code == GatekeeperStatusCode::ERROR_RETRY_TIMEOUT && rsp.timeout > 0) {
                              ret = rsp.timeout;
                          }
                      }
                      );
    if (!hwRet.isOk()) {
        ALOGE("Unable to Verify on Gatekeeper\n");
        return SYS_UNKNOWN;
    }

    return HIDLToURequestStatus(fpHal->enroll(auth_token, gid, timeoutSec));
}

UHardwareBiometryRequestStatus UHardwareBiometry_::postEnroll()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->postEnroll());
}

uint64_t UHardwareBiometry_::getAuthenticatorId()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return 0;
    }
    
    return fpHal->getAuthenticatorId();
}

UHardwareBiometryRequestStatus UHardwareBiometry_::cancel()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->cancel());
}

UHardwareBiometryRequestStatus UHardwareBiometry_::enumerate()
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->enumerate());
}

UHardwareBiometryRequestStatus UHardwareBiometry_::remove(uint32_t gid, uint32_t fid)
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->remove(gid, fid));
}

UHardwareBiometryRequestStatus UHardwareBiometry_::setActiveGroup(uint32_t gid, char *storePath)
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->setActiveGroup(gid, storePath));
}

UHardwareBiometryRequestStatus UHardwareBiometry_::authenticate(uint64_t operationId, uint32_t gid)
{
    if (fpHal == nullptr) {
        ALOGE("Unable to get FP service\n");
        return SYS_UNKNOWN;
    }
    
    return HIDLToURequestStatus(fpHal->authenticate(operationId, gid));
}

/////////////////////////////////////////////////////////////////////
// Implementation of the C API

UHardwareBiometry u_hardware_biometry_new()
{
    if (hybris_fp_instance != NULL)
        return NULL;

    UHardwareBiometry u_hardware_biometry = new UHardwareBiometry_();
    hybris_fp_instance = u_hardware_biometry;

    // Try ten times to initialize the FP HAL interface,
    // sleeping for 200ms per iteration in case of issues.
    for (unsigned int i = 0; i < 50; i++)
        if (u_hardware_biometry->init())
            return hybris_fp_instance = u_hardware_biometry;
        else
            // Sleep for some time and leave some time for the system
            // to finish initialization.
            ::usleep(200 * 1000);

    // This is the error case, as we did not succeed in initializing the FP interface.
    delete u_hardware_biometry;
    return hybris_fp_instance;
}

uint64_t u_hardware_biometry_setNotify(UHardwareBiometry self, UHardwareBiometryParams *params)
{
    UHardwareBiometryCallback u_hardware_biometry_cb = new UHardwareBiometryCallback_(params);
    hybris_fp_instance_cb = u_hardware_biometry_cb;

    return self->setNotify();
}

uint64_t u_hardware_biometry_preEnroll(UHardwareBiometry self)
{
    return self->preEnroll();
}

UHardwareBiometryRequestStatus u_hardware_biometry_enroll(UHardwareBiometry self, uint32_t gid, uint32_t timeoutSec, uint32_t uid)
{
    return self->enroll(gid, timeoutSec, uid);
}

UHardwareBiometryRequestStatus u_hardware_biometry_postEnroll(UHardwareBiometry self)
{
    return self->postEnroll();
}

uint64_t u_hardware_biometry_getAuthenticatorId(UHardwareBiometry self)
{
    return self->getAuthenticatorId();
}

UHardwareBiometryRequestStatus u_hardware_biometry_cancel(UHardwareBiometry self)
{
    return self->cancel();
}

UHardwareBiometryRequestStatus u_hardware_biometry_enumerate(UHardwareBiometry self)
{
    return self->enumerate();
}

UHardwareBiometryRequestStatus u_hardware_biometry_remove(UHardwareBiometry self, uint32_t gid, uint32_t fid)
{
    return self->remove(gid, fid);
}

UHardwareBiometryRequestStatus u_hardware_biometry_setActiveGroup(UHardwareBiometry self, uint32_t gid, char *storePath)
{
    return self->setActiveGroup(gid, storePath);
}

UHardwareBiometryRequestStatus u_hardware_biometry_authenticate(UHardwareBiometry self, uint64_t operationId, uint32_t gid)
{
    return self->authenticate(operationId, gid);
}
