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

#include <kernel/fs/initrd/file.h>
#include <kernel/libc/memory.h>

bool InitrdFile::seek(size_t) {
    return false;
}

bool InitrdFile::read(size_t n, char* buf) {
    if (n > mSizeLeft) return false;
    memcopy(mPointer, (uint8_t*)buf, n);
    mSizeLeft -= n;
    return true;
}

bool InitrdFile::write(size_t, char*) {
    return false;
}

bool InitrdFile::stat(Filesystem::File::stat_t& stat) {
    stat.size = mFileSize;
    return true;
}

InitrdFile::InitrdFile(uint8_t *base, uint32_t size) : mPointer(base), mSizeLeft(size), mFileSize(size) {}
