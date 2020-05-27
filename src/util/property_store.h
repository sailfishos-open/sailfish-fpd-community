/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 *
 */

#ifndef BIOMETRY_UTIL_PROPERTY_STORE_H_
#define BIOMETRY_UTIL_PROPERTY_STORE_H_


#include <string>

namespace util
{
/// @brief PropertyStore provides access to system-wide properties.
class PropertyStore
{
public:
    /// @brief get returns the value associated with keys.
    /// @throws std::out_of_range if no value is known for key.
    virtual std::string get(const std::string& key) const = 0;
};

/// @brief AndroidPropertyStore queries properties from the android property system.
class AndroidPropertyStore : public PropertyStore
{
public:
    // From PropertyStore.
    std::string get(const std::string &key) const override;
};
}


#endif // BIOMETRY_UTIL_PROPERTY_STORE_H_
