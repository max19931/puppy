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
#include <libuserspace/stdio.h>
#include <libuserspace/string.h>
#include <muzzle/stdlib.h>

int main(int argc, const char** argv) {
    if (argc != 1) {
        printf("mkramdisk <size>\n");
        exit(1);
    }
    
    auto arg = argv[0];
    auto size = atoi(arg);
    printf("About to create a RAM disk volume of %u bytes...\n", size);

    auto fd = open("/devices/ramdisk/new", gModeRead);
    if (fd == gInvalidFd) {
        printf("error: cannot open ramdisk device\n");
        exit(2);
    }

    int result = ioctl(fd, 0x4449534b, size);
    if (result >= 0) {
        printf("/devices/ramdisk/vol%u should now be a valid RAM disk\n", result);
    } else {
        printf("error: ramdisk device returned 0x%x\n", result);
        exit(3);
    }

    return 0;
}
