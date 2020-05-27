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

#ifndef BIOMETRY_HARDWARE_BIOMETRY_H_
#define BIOMETRY_HARDWARE_BIOMETRY_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int UHardwareBiometryFingerprintAcquiredInfo;
enum
{
    ACQUIRED_GOOD = 0,
    /** sensor needs more data, i.e. longer swipe. */
    ACQUIRED_PARTIAL = 1,
    /**
     * image doesn't contain enough detail for recognition*/
    ACQUIRED_INSUFFICIENT = 2,
    /** sensor needs to be cleaned */
    ACQUIRED_IMAGER_DIRTY = 3,
    /** mostly swipe-type sensors; not enough data collected */
    ACQUIRED_TOO_SLOW = 4,
    /** vendor-specific acquisition messages start here */
    ACQUIRED_TOO_FAST = 5,
    /** vendor-specific acquisition messages */
    ACQUIRED_VENDOR = 6
};

typedef int UHardwareBiometryFingerprintError;
enum
{
    /** Used for testing, no error returned */
    ERROR_NO_ERROR = 0,
    /** The hardware has an error that can't be resolved. */
    ERROR_HW_UNAVAILABLE = 1,
    /** Bad data; operation can't continue */
    ERROR_UNABLE_TO_PROCESS = 2,
    /** The operation has timed out waiting for user input. */
    ERROR_TIMEOUT = 3,
    /** No space available to store a template */
    ERROR_NO_SPACE = 4,
    /** The current operation has been canceled */
    ERROR_CANCELED = 5,
    /** Unable to remove a template */
    ERROR_UNABLE_TO_REMOVE = 6,
    /** The hardware is in lockout due to too many attempts */
    ERROR_LOCKOUT = 7,
    /** Vendor-specific error message */
    ERROR_VENDOR = 8
};

typedef int UHardwareBiometryRequestStatus;
enum
{
    SYS_UNKNOWN = 1,
    SYS_OK = 0,
    SYS_ENOENT = -2,
    SYS_EINTR = -4,
    SYS_EIO = -5,
    SYS_EAGAIN = -11,
    SYS_ENOMEM = -12,
    SYS_EACCES = -13,
    SYS_EFAULT = -14,
    SYS_EBUSY = -16,
    SYS_EINVAL = -22,
    SYS_ENOSPC = -28,
    SYS_ETIMEDOUT = -110,
};

typedef struct UHardwareBiometry_* UHardwareBiometry;
typedef struct UHardwareBiometryCallback_* UHardwareBiometryCallback;

typedef void (*UHardwareBiometryEnrollResult)(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining, void *context);
typedef void (*UHardwareBiometryAcquired)(uint64_t deviceId, UHardwareBiometryFingerprintAcquiredInfo acquiredInfo, int32_t vendorCode, void *context);
typedef void (*UHardwareBiometryAuthenticated)(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, void *context);
typedef void (*UHardwareBiometryError)(uint64_t deviceId, UHardwareBiometryFingerprintError error, int32_t vendorCode, void *context);
typedef void (*UHardwareBiometryRemoved)(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining, void *context);
typedef void (*UHardwareBiometryEnumerate)(uint64_t deviceId, uint32_t fingerId, uint32_t groupId, uint32_t remaining, void *context);

typedef struct
{
    UHardwareBiometryEnrollResult enrollresult_cb;
    UHardwareBiometryAcquired acquired_cb;
    UHardwareBiometryAuthenticated authenticated_cb;
    UHardwareBiometryError error_cb;
    UHardwareBiometryRemoved removed_cb;
    UHardwareBiometryEnumerate enumerate_cb;

    void* context;
} UHardwareBiometryParams;

/*
 You must create only one instance per process/application.
*/

UHardwareBiometry
u_hardware_biometry_new();

uint64_t
u_hardware_biometry_setNotify(UHardwareBiometry self, UHardwareBiometryParams *params);

uint64_t
u_hardware_biometry_preEnroll(UHardwareBiometry self);

UHardwareBiometryRequestStatus
u_hardware_biometry_enroll(UHardwareBiometry self, uint32_t gid, uint32_t timeoutSec, uint32_t uid);

UHardwareBiometryRequestStatus
u_hardware_biometry_postEnroll(UHardwareBiometry self);

uint64_t
u_hardware_biometry_getAuthenticatorId(UHardwareBiometry self);

UHardwareBiometryRequestStatus
u_hardware_biometry_cancel(UHardwareBiometry self);

UHardwareBiometryRequestStatus
u_hardware_biometry_enumerate(UHardwareBiometry self);

UHardwareBiometryRequestStatus
u_hardware_biometry_remove(UHardwareBiometry self, uint32_t gid, uint32_t fid);

UHardwareBiometryRequestStatus
u_hardware_biometry_setActiveGroup(UHardwareBiometry self, uint32_t gid, char *storePath);

UHardwareBiometryRequestStatus
u_hardware_biometry_authenticate(UHardwareBiometry self, uint64_t operationId, uint32_t gid);

#ifdef __cplusplus
}
#endif

#endif // BIOMETRY_HARDWARE_BIOMETRY_H_
