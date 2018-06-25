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

#include <libuserspace/exit.h>
#include <libuserspace/file.h>
#include <libuserspace/printf.h>
#include <muzzle/string.h>

static void usage() {
    printf("echo can be used in one of the following modes:\n");
    printf("  echo <words> <words> ... ==> will print its arguments to the console\n");
    printf("  echo --file <file> <words> ... ==> will print its arguments to the provided file\n");
    printf("  echo --file <file> --append <words> ... ==> will append its arguments to the provided file\n");
    exit(1);
}

int main(int argc, const char** argv) {
    if (argc == 0) return 0;

    const char* file = nullptr;
    bool append = false;
    auto fd = gInvalidFd;

    for (auto i = 0; i < argc; ++i) {
        if (0 == strcmp(argv[i], "--file")) {
            if (i + 1 >= argc) {
                usage();
            } else {
                file = argv[++i];
                continue;
            }
        } else if (0 == strcmp(argv[i], "--append")) {
            append = true;
        } else {
            if (fd == gInvalidFd) {
                if (file == nullptr) {
                    printf("%s\n", argv[i]);
                    continue;
                } else {
                    fd = open(file, FILE_OPEN_WRITE | (append ? FILE_OPEN_APPEND : FILE_OPEN_NEW));
                    if (fd == gInvalidFd) usage();
                }
            }
            write(fd, strlen(argv[i]), (unsigned char*)argv[i]);
            write(fd, 1, (unsigned char*)"\n");
        }
    }

    if (fd != gInvalidFd) close(fd);

    return 0;
}

