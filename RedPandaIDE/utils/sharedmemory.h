/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
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

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <QString>

#include <atomic>

class SharedMemory {
public:
    SharedMemory();
    SharedMemory(const SharedMemory &other) = delete;
    SharedMemory &operator=(const SharedMemory &other) = delete;
    ~SharedMemory();

    char *data() const;
    const QString &name() const;

private:
    static int nextSharedMemoryId();

public:
    constexpr static size_t bufferSize = 1024;

private:
    QString mName;
    char *mData;
#ifdef Q_OS_WIN
    HANDLE mHandle;
#endif

    static std::atomic<int> sharedMemoryIdCounter;
};

#endif // SHARED_MEMORY_H
