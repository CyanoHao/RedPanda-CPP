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
#include "builtinterminalrunner.h"

#include <QDebug>

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

BuiltInTerminalRunner::BuiltInTerminalRunner(int fdShm, char *shm, QString sharedMemoryId, QObject *parent):
    Runner({}, {}, {}, parent),
    mFdShm(fdShm),
    mShm(shm),
    mSharedMemoryId(sharedMemoryId)
{
    setWaitForFinishTime(500);
}

BuiltInTerminalRunner::~BuiltInTerminalRunner()
{
    if (mShm)
        munmap(mShm, sharedMemorySize);
    if (mFdShm >= 0) {
        shm_unlink(mSharedMemoryId.toLocal8Bit().data());
    }
}

void BuiltInTerminalRunner::run()
{
    while (true) {
        if (mStop)
            break;

        if (mShm && strncmp(mShm, "FINISHED", sizeof("FINISHED")) == 0)
            break;

        QThread::msleep(waitForFinishTime());
    }
}
