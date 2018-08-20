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

#include <newlib/stdio.h>
#include <newlib/stdlib.h>
#include <newlib/string.h>
#include <newlib/ctype.h>
#include <newlib/strings.h>

void printbuf(char* buf, int count) {
    for (auto i = 0; i < count; ++i) {
        printf("%02x ", buf[i]);
    }
    printf("    ");
    for (auto i = 0; i < count; ++i) {
        if (isprint(buf[i]))
            printf("%c", buf[i]);
        else
            printf(".");
    }
    printf("\n");
}

void dump(FILE* f) {
    char buf[16] = {0};
    while(true) {
        bzero(&buf[0], sizeof(buf));
        int count = fread(&buf[0], 1, sizeof(buf), f);
        if (0 == count) break;
        printbuf(&buf[0], count);
    }
    fclose(f);
}

void cat(const char* path) {
    printf("Printout of %s\n", path);
    printf("==============================================================================\n");
    auto fd = fopen(path, "r'");
    if (fd == nullptr) {
        printf("could not open %s - exiting\n", path);
        exit(1);
    }
    dump(fd);
    printf("==============================================================================\n");    
}

int main(int argc, const char** argv) {
    for (auto i = 1; i < argc; ++i) {
        cat(argv[i]);
    }

    return 0;
}
