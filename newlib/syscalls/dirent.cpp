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

#include <newlib/sys/dirent.h>
#include <newlib/sys/errno.h>
#include <kernel/syscalls/types.h>
#include <newlib/syscalls.h>
#include <newlib/stdlib.h>
#include <newlib/strings.h>
#include <newlib/string.h>

extern "C" int  errno;

DIR* opendir(const char* path) {
    int od = fopendir_syscall((uint32_t)path);
    if (od & 1) return nullptr;
    DIR* d = (DIR*)malloc(sizeof(DIR));
    d->fhnd = od >> 1;
    return d;
}

int closedir(DIR* d) {
    bool ok = false;
    if (d) {
        ok = (0 == fclose_syscall(d->fhnd));
    }
    free(d);
    return ok ? 0 : 1;
}

int readdir_r(DIR*, struct dirent*, struct dirent**) {
    errno = EINVAL;
    return -1;
}

struct dirent* readdir(DIR* dir) {
    if (!dir) return nullptr;
    bzero(&dir->current, sizeof(dirent));

    file_info_t fi;

    if (0 == freaddir_syscall(dir->fhnd, &fi)) {
        dir->current.d_reclen = sizeof(dir->current);
        strncpy(dir->current.d_name, fi.name, gMaxPathSize);
        dir->current.d_ino = 0;
        dir->current.d_size = fi.size;
        switch (fi.kind) {
            case file_kind_t::blockdevice:
                dir->current.d_type = DT_BLK;
                break;
            case file_kind_t::directory:
                dir->current.d_type = DT_DIR;
                break;
            case file_kind_t::file:
                dir->current.d_type = DT_REG;
                break;
        }
        return &dir->current;
    } else {
        return nullptr;
    }
}