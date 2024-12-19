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
#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// POSIX and C
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>

// 3rd party
#include <CLI/CLI.hpp>

namespace fs = std::filesystem;
using std::string;
using std::vector;

enum class RedirectInputMode {
    None, // no redirection
    /* Pipe, */ // Unix: always create a new tty, pipe never work
    File, // console pauser handles redirection
};

void PrintSplitLine(FILE *stream, bool lineBreakBeforePrint)
{
    int width = 80;
    struct winsize ws;
    if (ioctl(fileno(stream), TIOCGWINSZ, &ws) == 0)
        width = ws.ws_col;

    if (lineBreakBeforePrint)
        fputc('\n', stream);
    for (int i = 0; i < width; i++)
        fputc('-', stream);
    fputc('\n', stream);
}

void PrintSplitLineToStdout(bool lineBreakBeforePrint = true)
{
    PrintSplitLine(stdout, lineBreakBeforePrint);
}

void ClearStdinBuffer()
{
    tcflush(fileno(stdin), TCIFLUSH);
}

void PauseExit(int exitcode)
{
    ClearStdinBuffer();

    printf("\n");
    printf("Press ANY key to exit...");
    fflush(stdout);

    // set console to raw mode so we can read a single key
    struct termios termios, saved;
    int getResult;
    if ((getResult = tcgetattr(fileno(stdin), &termios)) == 0) {
        saved = termios;
        cfmakeraw(&termios);
        tcsetattr(fileno(stdin), TCSANOW, &termios);
    }

    getchar();

    // restore console mode, in case someone run it in existing terminal
    if (getResult == 0)
        tcsetattr(fileno(stdin), TCSANOW, &saved);

    exit(exitcode);
}

int ExecuteCommand(const string &program, vector<string> &args, RedirectInputMode redirectInputMode, const string &redirectedInputFile, long int &peakMemory) {
    peakMemory = 0;
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Failed to create process -- fork() failed with %d: %s\n", errno, strerror(errno));
        return -1;
    }
    if (pid == 0) {
        vector<char *> argv(args.size() + 2, nullptr);
        string basename = fs::path(program).filename().string();
        argv[0] = basename.data();
        for (size_t i = 0; i < args.size(); i++)
            argv[i + 1] = args[i].data();
        argv[args.size() + 1] = nullptr;

        if (redirectInputMode == RedirectInputMode::File) {
            freopen(redirectedInputFile.c_str(), "r", stdin);
        }

        // child process
        execv(program.c_str(), argv.data());

        // execv returns? error occured!
        fprintf(stderr,"Failed to start command %s %s!\n", program.c_str(), basename.c_str());
        fprintf(stderr,"errno %d: %s\n",errno,strerror(errno));
        char* current_dir = getcwd(nullptr, 0);
        fprintf(stderr,"current dir: %s",current_dir);
        free(current_dir);
        exit(-1);
    } else {
        int status;
        pid_t w;
        struct rusage usage;
        w = wait4(pid, &status, WUNTRACED | WCONTINUED, &usage);
        if (w==-1) {
            fprintf(stderr,"wait4 failed!");
            exit(EXIT_FAILURE);
        }
        peakMemory = usage.ru_maxrss;
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return status;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    CLI::App app{"Run program and pause console after exit."};

    bool pauseAfterExit = 0;
    app.add_flag("--pause-console", pauseAfterExit, "Pause console after exit.");

    std::map<string, RedirectInputMode> redirectInputModeMap = {
        {"none", RedirectInputMode::None},
        {"file", RedirectInputMode::File},
    };
    RedirectInputMode redirectInputMode = RedirectInputMode::None;
    app.add_option("--redirect-input", redirectInputMode, "Redirect stdin.")
        ->transform(CLI::CheckedTransformer(redirectInputModeMap));

    string redirectedInputFile;
    app.add_option("--redirect-input-file", redirectedInputFile, "Redirect stdin to file.");

    string sharedMemoryName;
    app.add_option("--shared-memory-name", sharedMemoryName, "Communicate to Red Panda C++ with the shared memory object.");

    string program;
    app.add_option("program", program, "Program to run.")->required();

    vector<string> args;
    app.add_option("args", args, "Arguments to pass to program.");

    app.failure_message(CLI::FailureMessage::help);
    CLI11_PARSE(app, argc, argv);

    ClearStdinBuffer();

    int BUF_SIZE=1024;
    char* pBuf=nullptr;
    int fd_shm = -1;
    if (!sharedMemoryName.empty()) {
        fd_shm = shm_open(sharedMemoryName.c_str(), O_RDWR, S_IRWXU);
        if (fd_shm == -1) {
            //todo: handle error
            fprintf(stderr,"shm open failed %d:%s\n",errno,strerror(errno));
        } else {
            // `ftruncate` has already done in RedPandaIDE
            pBuf = (char*)mmap(NULL,BUF_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm,0);
            if (pBuf == MAP_FAILED) {
                fprintf(stderr,"mmap failed %d:%s\n",errno,strerror(errno));
                pBuf = nullptr;
            }
        }
    }

    // Save starting timestamp
    auto starttime = std::chrono::high_resolution_clock::now();

    // Execute the command
    long int peakMemory;
    int returnvalue = ExecuteCommand(program, args, redirectInputMode, redirectedInputFile, peakMemory);

    // Get ending timestamp
    auto endtime = std::chrono::high_resolution_clock::now();
    auto difftime = endtime - starttime;
    double seconds = difftime.count() * 1.0 / decltype(difftime)::period::den;

    if (pBuf) {
        strcpy(pBuf,"FINISHED");
        munmap(pBuf,BUF_SIZE);
    }
    if (fd_shm!=-1) {
        shm_unlink(sharedMemoryName.c_str());
    }

    // Done? Print return value of executed program
    PrintSplitLineToStdout();
    printf("Process exited after %.4g seconds with return value %d, %ld KB mem used.\n",seconds,returnvalue,peakMemory);
    if (pauseAfterExit)
        PauseExit(returnvalue);
    return 0;
}
