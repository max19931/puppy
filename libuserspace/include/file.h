/*
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBUSERSPACE_FILE
#define LIBUSERSPACE_FILE

#include <stdint.h>
#include <kernel/syscalls/types.h>

static constexpr uint32_t gModeRead = FILE_OPEN_READ;
static constexpr uint32_t gModeWrite = FILE_OPEN_WRITE;
static constexpr uint32_t gModeAppend = FILE_OPEN_WRITE | FILE_OPEN_APPEND;
static constexpr uint32_t gModeCreate = FILE_OPEN_NEW;

static constexpr uint32_t gInvalidFd = -1;

extern "C"
uint32_t open(const char* path, uint32_t mode);

extern "C"
bool del(const char* path);

extern "C"
void close(uint32_t fid);

extern "C"
size_t read(uint32_t fid, uint32_t size, unsigned char* buffer);

extern "C"
size_t write(uint32_t fid, uint32_t size, unsigned char* buffer);

extern "C"
uintptr_t ioctl(uint32_t fid, uintptr_t, uintptr_t);

extern "C"
bool fsize(uint32_t fid, uint32_t& sz);

extern "C"
bool mkdir(const char* path);

#endif