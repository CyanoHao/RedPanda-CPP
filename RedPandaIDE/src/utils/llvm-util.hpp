/*
 * Copyright (C) 2020-2026 Roy Qu (royqh1979@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILS_LLVM_UTIL_HPP
#define UTILS_LLVM_UTIL_HPP

#include <QString>
#include <QStringList>
#include <optional>

namespace sys
{
#ifdef _MSC_VER
    constexpr bool IsLittleEndianHost = true;
#else
    constexpr bool IsLittleEndianHost = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#endif
}

template<typename T>
class StringSwitch
{
    const QString str;
    std::optional<T> result;

public:
    explicit StringSwitch(QString s)
        : str(s)
        , result()
    {
    }

    StringSwitch(const StringSwitch &) = delete;
    StringSwitch &operator=(const StringSwitch &) = delete;

    StringSwitch(StringSwitch &&) = default;
    StringSwitch &operator=(StringSwitch &&) = delete;

    StringSwitch &Case(const QString &s, T value)
    {
        if (!result.has_value() && str == s)
            result = value;
        return *this;
    }

    StringSwitch &StartsWith(const QString &s, T value)
    {
        if (!result.has_value() && str.startsWith(s))
            result = value;
        return *this;
    }

    StringSwitch &Cases(const QStringList &l, T value)
    {
        if (!result.has_value()) {
            for (const auto &s : l) {
                if (str == s) {
                    result = value;
                    break;
                }
            }
        }
        return *this;
    }

    T &Default(T value)
    {
        return result.has_value() ? result.value() : value;
    }
};

#endif
