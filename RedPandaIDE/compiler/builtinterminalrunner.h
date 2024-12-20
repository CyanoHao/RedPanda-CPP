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
#ifndef BUILTINTERMINALRUNNER_H
#define BUILTINTERMINALRUNNER_H

#include "runner.h"

class BuiltInTerminalRunner : public Runner
{
    Q_OBJECT
public:
    constexpr static size_t sharedMemorySize = 1024;

    BuiltInTerminalRunner(int fdShm, char *shm, QString sharedMemoryId, QObject *parent = nullptr);
    BuiltInTerminalRunner(const BuiltInTerminalRunner &) = delete;
    BuiltInTerminalRunner &operator=(const BuiltInTerminalRunner &) = delete;

    ~BuiltInTerminalRunner() override;

protected:
    int mFdShm;
    char *mShm;
    QString mSharedMemoryId;

    // QThread interface
protected:
    void run() override;
};

#endif // BuiltInTerminalRunner_H
