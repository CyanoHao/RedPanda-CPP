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

#include "sharedmemory.h"

#include <QCoreApplication>
#include <QDebug>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif


SharedMemory::SharedMemory()
{
    int id = nextSharedMemoryId();
    mData = nullptr;
    mName =
#ifdef Q_OS_WIN
        QStringLiteral("io.redpanda.shmem_%1_%2")
#elif defined (Q_OS_MACOS)
        // macOS has limit PSHMNAMLEN = 31
        QStringLiteral("/rshm_%1_%2")
#else
        QStringLiteral("/io.redpanda.shmem_%1_%2")
#endif
            .arg(QCoreApplication::applicationPid())
            .arg(id);

#ifdef Q_OS_WIN
    mHandle = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        bufferSize,
        (const wchar_t *)mName.constData());
    if (mHandle == nullptr) {
        qWarning() << "CreateFileMappingW failed:" << GetLastError();
        return;
    }
    mData = (char *)MapViewOfFile(
        mHandle,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        bufferSize);
    if (mData == nullptr) {
        qWarning() << "MapViewOfFile failed:" << GetLastError();
        return;
    }
    mData[0] = 0;
#else
    int fd = shm_open(mName.toUtf8().constData(), O_RDWR | O_CREAT, S_IRWXU);
    if (fd == -1) {
        qWarning() << "shm_open failed:" << strerror(errno);
        mName = QString();
        return;
    }
    if (ftruncate(fd, bufferSize) == -1) {
        qWarning() << "ftruncate failed:" << strerror(errno);
        close(fd);
        return;
    }
    mData = (char *)mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (mData == MAP_FAILED) {
        qWarning() << "mmap failed:" << strerror(errno);
        mData = nullptr;
        return;
    }
    mData[0] = 0;
#endif
}

SharedMemory::~SharedMemory()
{
#ifdef Q_OS_WIN
    if (mData != nullptr)
        UnmapViewOfFile(mData);
    if (mHandle != nullptr)
        CloseHandle(mHandle);
#else
    if (mData != nullptr)
        munmap(mData, bufferSize);
    if (!mName.isEmpty())
        shm_unlink(mName.toUtf8().constData());
#endif
}

char *SharedMemory::data() const
{
    return mData;
}

const QString &SharedMemory::name() const
{
    return mName;
}

int SharedMemory::nextSharedMemoryId()
{
    return ++sharedMemoryIdCounter;
}

std::atomic<int> SharedMemory::sharedMemoryIdCounter = 0;
