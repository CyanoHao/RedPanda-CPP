/*
 *  This file is part of Red Panda C++
 *  Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
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

// C++
#include <map>
#include <string>

// C
#include <stdio.h>

// Win32
#include <windows.h>
#include <psapi.h>
#include <processthreadsapi.h>
#include <conio.h>
#include <versionhelpers.h>

// 3rd party
#include <CLI/CLI.hpp>

using std::string;
using std::vector;
using std::wstring;

#ifndef WINBOOL
#define WINBOOL BOOL
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_PROCESSED_OUTPUT
#define ENABLE_PROCESSED_OUTPUT 0x0001
#endif
#define MAX_COMMAND_LENGTH 32768
#define MAX_ERROR_LENGTH 2048

#define EXIT_WRONG_ARGUMENTS           -1
#define EXIT_COMMAND_TOO_LONG          -2
#define EXIT_CREATE_JOB_OBJ_FAILED     -3
#define EXIT_SET_JOB_OBJ_INFO_FAILED   -4
#define EXIT_CREATE_PROCESS_FAILED     -5
#define EXIT_ASSGIN_PROCESS_JOB_FAILED -6

enum class RedirectInputMode {
    None,  // no redirection
    Pipe,  // IDE handles redirection, stdin is a pipe
    File,  // console pauser handles redirection
};

HANDLE hJob;
bool enableJobControl = IsWindowsXPOrGreater();

bool pauseBeforeExit = false;

LONGLONG GetClockTick() {
    LARGE_INTEGER dummy;
    QueryPerformanceCounter(&dummy);
    return dummy.QuadPart;
}

LONGLONG GetClockFrequency() {
    LARGE_INTEGER dummy;
    QueryPerformanceFrequency(&dummy);
    return dummy.QuadPart;
}

template <typename... Ts>
void PrintToStream(HANDLE hStream, const wchar_t *fmt, Ts &&...args)
{
    constexpr size_t buffer_size = 64 * 1024;
    static wchar_t buffer[buffer_size];
    size_t length = _snwprintf(buffer, buffer_size, fmt, std::forward<Ts>(args)...);
    WriteConsoleW(hStream, buffer, length, NULL, NULL);
}

template <typename... Ts>
void PrintToStdout(const wchar_t *fmt, Ts &&...args)
{
    PrintToStream(GetStdHandle(STD_OUTPUT_HANDLE), fmt, std::forward<Ts>(args)...);
}

template <typename... Ts>
void PrintToStderr(const wchar_t *fmt, Ts &&...args)
{
    PrintToStream(GetStdHandle(STD_ERROR_HANDLE), fmt, std::forward<Ts>(args)...);
}

void PrintSplitLine(HANDLE hStream, bool lineBreakBeforePrint)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    int width = 80;
    if (GetConsoleScreenBufferInfo(hStream, &info))
        width = info.dwSize.X;

    wstring content;
    if (lineBreakBeforePrint)
        content.push_back(L'\n');
    content.append(width, L'-');
    content.push_back(L'\n');
    WriteConsoleW(hStream, content.c_str(), content.size(), NULL, NULL);
}

void PrintSplitLineToStdout(bool lineBreakBeforePrint = true)
{
    PrintSplitLine(GetStdHandle(STD_OUTPUT_HANDLE), lineBreakBeforePrint);
}

void PrintSplitLineToStderr(bool lineBreakBeforePrint = true)
{
    PrintSplitLine(GetStdHandle(STD_ERROR_HANDLE), lineBreakBeforePrint);
}

wstring GetErrorMessage(DWORD errorCode) {
    wstring result(MAX_ERROR_LENGTH,0);
    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, LANG_USER_DEFAULT, result.data(), result.size(), NULL);

    // Clear newlines at end of string
    while (!result.empty() && (result.back() == 0 || iswspace(result.back())))
        result.pop_back();
    return result;
}

void PauseExit(int exitcode, RedirectInputMode reInp) {
    if (pauseBeforeExit) {
        HANDLE hInp = NULL;
        if (reInp == RedirectInputMode::Pipe) {
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;

            hInp = CreateFileA("CONIN$", GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_READ , &sa, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/0, NULL);
        } else {
            hInp = GetStdHandle(STD_INPUT_HANDLE);
        }
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
        PrintToStdout(L"\nPress any key to exit...");
        wchar_t buffer[2];
        DWORD nRead;
        ReadConsoleW(hInp, buffer, 1, &nRead, NULL);
        if (reInp == RedirectInputMode::Pipe) {
            CloseHandle(hInp);
        }
    }
    exit(exitcode);
}

wstring EscapeArgument(const wstring &arg)
{
    // reduced version of `escapeArgumentImplWindowsCreateProcess` in `RedPandaIDE/utils/escape.cpp`
    // see also https://learn.microsoft.com/en-gb/archive/blogs/twistylittlepassagesallalike/everyone-quotes-command-line-arguments-the-wrong-way .

    if (!arg.empty() &&
        arg.find_first_of(L" \t\n\v\"") == wstring::npos)
        return arg;

    wstring result = L"\"";
    for (auto it = arg.begin(); ; ++it) {
        int nBackSlash = 0;
        while (it != arg.end() && *it == L'\\') {
            ++it;
            ++nBackSlash;
        }
        if (it == arg.end()) {
            // escape all backslashes, but leave the terminating double quote unescaped
            result.append(nBackSlash * 2, L'\\');
            break;
        } else if (*it == L'"') {
            // escape all backslashes and the following double quote
            result.append(nBackSlash * 2 + 1, L'\\');
            result.push_back(*it);
        } else {
            // backslashes aren't special here
            result.append(nBackSlash, L'\\');
            result.push_back(*it);
        }
    }
    return result;
}

wstring GetCommand(const wstring &program, const vector<wstring> &args, RedirectInputMode reInp) {
    wstring result = EscapeArgument(program);
    for (const wstring &arg : args) {
        result.append(L" ");
        result.append(EscapeArgument(arg));
    }

    if(result.length() > MAX_COMMAND_LENGTH) {
        PrintSplitLineToStderr();
        PrintToStderr(L"Error: Length of command line string is over %d characters\n",MAX_COMMAND_LENGTH);
        PauseExit(EXIT_COMMAND_TOO_LONG,reInp);
    }

    return result;
}

DWORD ExecuteCommand(wstring& command,RedirectInputMode reInp, LONGLONG &peakMemory, LONGLONG &execTime) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    memset(&si,0,sizeof(si));
    si.cb = sizeof(si);
    memset(&pi,0,sizeof(pi));

    DWORD dwCreationFlags = CREATE_BREAKAWAY_FROM_JOB;


    if(!CreateProcessW(NULL, command.data(), NULL, NULL, true, dwCreationFlags, NULL, NULL, &si, &pi)) {
        DWORD errorCode = GetLastError();
        PrintSplitLineToStderr();
        PrintToStderr(L"Failed to execute \"%ls\":\n", command.c_str());
        PrintToStderr(L"Error %lu: %ls\n", errorCode, GetErrorMessage(errorCode).c_str());
        PauseExit(EXIT_CREATE_PROCESS_FAILED,reInp);
    }
    if (enableJobControl) {
        WINBOOL bSuccess = AssignProcessToJobObject( hJob, pi.hProcess );
        if ( bSuccess == FALSE ) {
            PrintToStderr(L"AssignProcessToJobObject failed: error %lu\n", GetLastError());
            PauseExit(EXIT_ASSGIN_PROCESS_JOB_FAILED,reInp);
        }
    }

    WaitForSingleObject(pi.hProcess, INFINITE); // Wait for it to finish

    peakMemory = 0;
    PROCESS_MEMORY_COUNTERS counter;
    counter.cb = sizeof(counter);
    if (GetProcessMemoryInfo(pi.hProcess,&counter,
                                 sizeof(counter))){
        peakMemory = counter.PeakPagefileUsage/1024;
    }
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    execTime=0;
    if (GetProcessTimes(pi.hProcess,&creationTime,&exitTime,&kernelTime,&userTime)) {
        execTime=((LONGLONG)kernelTime.dwHighDateTime<<32)
                +((LONGLONG)userTime.dwHighDateTime<<32)
                +(kernelTime.dwLowDateTime)+(userTime.dwLowDateTime);
    }
    DWORD result = 0;
    GetExitCodeProcess(pi.hProcess, &result);
    return result;
}

void EnableVtSequence() {
    DWORD mode;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(hConsole, &mode))
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);

    hConsole = GetStdHandle(STD_ERROR_HANDLE);
    if (GetConsoleMode(hConsole, &mode))
        SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
}

int wmain(int argc, wchar_t** argv) {
    CLI::App app{"Run program and pause console after exit."};

    app.add_flag("--pause-console", pauseBeforeExit, "Pause console after exit.");

    std::map<string, RedirectInputMode> redirectInputModeMap = {
        {"none", RedirectInputMode::None},
        {"pipe", RedirectInputMode::Pipe},
        {"file", RedirectInputMode::File},
    };
    RedirectInputMode redirectInputMode = RedirectInputMode::None;
    app.add_option("--redirect-input", redirectInputMode, "Redirect stdin.")
        ->transform(CLI::CheckedTransformer(redirectInputModeMap));

    wstring redirectedInputFile;
    app.add_option("--redirect-input-file", redirectedInputFile, "Redirect stdin to file.");

    wstring sharedMemoryName;
    app.add_option("--shared-memory-name", sharedMemoryName, "Communicate to Red Panda C++ with the shared memory object.");

    bool enableVisualTerminalSeq = false;
    app.add_flag("--enable-virtual-terminal-sequence", enableVisualTerminalSeq, "Enable virtual terminal sequence (ANSI escape sequences).");

    wstring program;
    app.add_option("program", program, "Program to run.")->required();

    vector<wstring> args;
    app.add_option("args", args, "Arguments to pass to program.");

    app.failure_message(CLI::FailureMessage::help);
    CLI11_PARSE(app, argc, argv);

    // Make us look like the paused program
    SetConsoleTitleW(program.c_str());

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    // Then build the to-run application command
    wstring command = GetCommand(program, args, redirectInputMode);

    if (enableJobControl) {
        hJob= CreateJobObject( &sa, NULL );

        if ( hJob == NULL ) {
            DWORD ec = GetLastError();
            PrintToStderr(L"CreateJobObject failed with %lu: %ls\n", ec, GetErrorMessage(ec).c_str());
            PauseExit(EXIT_CREATE_JOB_OBJ_FAILED, redirectInputMode);
        }

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
        memset(&info,0,sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
        info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        WINBOOL bSuccess = SetInformationJobObject( hJob, JobObjectExtendedLimitInformation, &info, sizeof( info ) );
        if ( bSuccess == FALSE ) {
            DWORD ec = GetLastError();
            PrintToStderr(L"SetInformationJobObject failed with %lu: %ls\n", ec, GetErrorMessage(ec).c_str());
            PauseExit(EXIT_SET_JOB_OBJ_INFO_FAILED, redirectInputMode);
        }
    }

    HANDLE hOutput = NULL;
    if (redirectInputMode == RedirectInputMode::Pipe) {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        hOutput = CreateFileA("CONOUT$", GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_WRITE , &sa, OPEN_EXISTING, /*FILE_ATTRIBUTE_NORMAL*/0, NULL);
        SetStdHandle(STD_OUTPUT_HANDLE, hOutput);
        SetStdHandle(STD_ERROR_HANDLE, hOutput);
        freopen("CONOUT$","w+",stdout);
        freopen("CONOUT$","w+",stderr);
    } else {
        FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
    }
    if (enableVisualTerminalSeq) {
        EnableVtSequence();
    }

    HANDLE hSharedMemory=INVALID_HANDLE_VALUE;
    int BUF_SIZE=1024;
    char* pBuf=nullptr;
    
    if (!sharedMemoryName.empty()) {
        hSharedMemory = OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            sharedMemoryName.c_str()
            );
        if (hSharedMemory != NULL)
        {
            pBuf = (char*) MapViewOfFile(hSharedMemory,   // handle to map object
                FILE_MAP_ALL_ACCESS, // read/write permission
                0,
                0,
                BUF_SIZE);
        } else {
            DWORD ec = GetLastError();
            PrintToStderr(L"OpenFileMappingW failed with %lu: %ls\n", ec, GetErrorMessage(ec).c_str());
        }
    }

    // Save starting timestamp
    LONGLONG starttime = GetClockTick();

    LONGLONG peakMemory=0;
    LONGLONG execTime=0;
    // Then execute said command
    DWORD returnvalue = ExecuteCommand(command, redirectInputMode,peakMemory,execTime);

    // Get ending timestamp
    LONGLONG endtime = GetClockTick();
    double seconds = (endtime - starttime) / (double)GetClockFrequency();
    double execSeconds = (double)execTime/10000;

    if (pBuf) {
        strcpy(pBuf,"FINISHED");
        UnmapViewOfFile(pBuf);
    }
    if (hSharedMemory != NULL && hSharedMemory!=INVALID_HANDLE_VALUE) {
        CloseHandle(hSharedMemory);
    }

    // Done? Print return value of executed program
    PrintSplitLineToStdout();
    PrintToStdout(L"Process exited after %.4g seconds with return value %lu (%.4g ms cpu time, %lld KB mem used).\n",seconds,returnvalue, execSeconds, peakMemory);
    PauseExit(returnvalue, redirectInputMode);
    return 0;
}

