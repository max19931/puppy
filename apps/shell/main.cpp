// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <newlib/stdint.h>
#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/strings.h>
#include <newlib/syscalls.h>
#include <newlib/unistd.h>
#include <newlib/sys/process.h>
#include <newlib/sys/wait.h>
#include <newlib/sys/unistd.h>

void handleExitStatus(uint16_t pid, int exitcode) {
    if (WIFEXITED(exitcode)) {
        int status = WEXITSTATUS(exitcode);
        if (status) {
            printf("[child %u] exited - status %d\n", pid, status);
        }
    } else if(WIFSIGNALED(exitcode)) {
        int sig = WTERMSIG(exitcode);
        printf("[child %u] terminated - signal %d\n", pid, sig);
    } else {
        printf("[child %u] terminated - exit status %d\n", pid, exitcode);
    }
}

void tryCollect() {
    int pid = 0;
    int exitcode = 0;
    pid = wait(&exitcode);
    if (pid != -1) {
        handleExitStatus(pid, exitcode);
    }
}

static char* trim(char* s) {
    while(s && *s == ' ') ++s;
    s[strcspn(s, "\n")] = 0; // replace \n with 0; TODO: do not replace embedded newlines
    return s;
}

static void cd_exec(const char* args) {
    if (chdir(args)) {
        printf("can't set cwd to '%s'\n", args);
    }
}

struct builtin_cmd_t {
    const char* command;
    void(*executor)(const char*);
};

builtin_cmd_t builtin_cmds[] = {
    {"cd", cd_exec},
    {nullptr, nullptr}
};

static bool tryExecBuiltin(const char* program, const char* args) {
    size_t i = 0u;

    while (builtin_cmds[i].command) {
        if (0 == strcmp(builtin_cmds[i].command, program)) {
            if (builtin_cmds[i].executor) {
                builtin_cmds[i].executor(args);
            }
            return true;
        }
        ++i;
    }

    return false;
}

int main(int, const char**) {
    klog_syscall("shell is up and running");
    char* prompt = (char*)malloc(1024);

    while(true) {
        char* buffer = nullptr;
        bzero(prompt, 1024);
        if (prompt == getcwd(prompt, 1021)) {
            auto len = strlen(prompt);
            prompt[len] = '$';
            prompt[len+1] = 0;
        } else {
            strcpy(prompt, "shell>");
        }
        printf("%s ", prompt); fflush(stdout);
        size_t n = 0;
        size_t n_read = __getline(&buffer, &n, stdin);
        if (n_read == (size_t)-1) {
            if (feof(stdin)) {
                exit(0);
            }
        }
        const char* program = trim(buffer);
        if (program != 0 && *program != 0) {
            bool letgo = ('&' == *program);
            if (letgo) ++program;
            char* args = (char*)strchr(program, ' ');
            if (args != nullptr) {
                *args = 0;
                ++args;
            }
            // background execution makes no sense for builtins, so if it was asked, just go straight to spawn
            if (letgo || !tryExecBuiltin(program, args)) {
                auto chld = spawn(program, args, PROCESS_INHERITS_CWD | (letgo ? SPAWN_BACKGROUND : SPAWN_FOREGROUND));
                if (!letgo) {
                    int exitcode = 0;
                    waitpid(chld, &exitcode, 0);
                    handleExitStatus(chld, exitcode);
                }
            }
        }
        tryCollect();
        free(buffer);
    }
}
