/*
 * Copyright (C) 2020 UBports foundation Ltd
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

// C APIs
#include "../biometry.h"
#include "../hardware/android_hw_module.h"

IMPLEMENT_FUNCTION0(
UHardwareBiometry,
u_hardware_biometry_new)

IMPLEMENT_FUNCTION2(
uint64_t,
u_hardware_biometry_setNotify,
UHardwareBiometry,
UHardwareBiometryParams*)

IMPLEMENT_FUNCTION1(
uint64_t,
u_hardware_biometry_preEnroll,
UHardwareBiometry)

IMPLEMENT_FUNCTION4(
UHardwareBiometryRequestStatus,
u_hardware_biometry_enroll,
UHardwareBiometry,
uint32_t,
uint32_t,
uint32_t)

IMPLEMENT_FUNCTION1(
UHardwareBiometryRequestStatus,
u_hardware_biometry_postEnroll,
UHardwareBiometry)

IMPLEMENT_FUNCTION1(
uint64_t,
u_hardware_biometry_getAuthenticatorId,
UHardwareBiometry)

IMPLEMENT_FUNCTION1(
UHardwareBiometryRequestStatus,
u_hardware_biometry_cancel,
UHardwareBiometry)

IMPLEMENT_FUNCTION1(
UHardwareBiometryRequestStatus,
u_hardware_biometry_enumerate,
UHardwareBiometry)

IMPLEMENT_FUNCTION3(
UHardwareBiometryRequestStatus,
u_hardware_biometry_remove,
UHardwareBiometry,
uint32_t,
uint32_t)

IMPLEMENT_FUNCTION3(
UHardwareBiometryRequestStatus,
u_hardware_biometry_setActiveGroup,
UHardwareBiometry,
uint32_t,
char*)

IMPLEMENT_FUNCTION3(
UHardwareBiometryRequestStatus,
u_hardware_biometry_authenticate,
UHardwareBiometry,
uint64_t,
uint32_t)
