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

#ifndef VFS_FILESYSTEM
#define VFS_FILESYSTEM

#include <kernel/sys/stdint.h>
#include <kernel/libc/str.h>
#include <kernel/syscalls/types.h>

class Filesystem {
    public:
        class FilesystemObject {
            public:
                static constexpr size_t gFilenameSize = 32;
                typedef unsigned char filename_t[gFilenameSize];
                enum class kind_t {
                    directory,
                    file,
                    blockdevice, /** see blockdevice_ioctl */
                };
                virtual ~FilesystemObject() = default;

                struct info_t {
                    uint8_t name[64] = {0};
                    kind_t kind;
                    size_t size;
                };

                kind_t kind() const;
                void kind(kind_t);
            protected:
                FilesystemObject(kind_t);
            private:
                kind_t mKind;
        };
        class File : public FilesystemObject {
            public:
                struct stat_t {
                    uint32_t size;
                };

                virtual bool seek(size_t) = 0;
                virtual size_t read(size_t, char*) = 0;
                virtual size_t write(size_t, char*) = 0;
                virtual bool stat(stat_t&) = 0;
                virtual uintptr_t ioctl(uintptr_t, uintptr_t);

                virtual ~File() = default;

            protected:
                File();
        };

        class Directory : public FilesystemObject {
            public:
                struct fileinfo_t : public File::stat_t {
                    string name;
                    using kind_t = Filesystem::FilesystemObject::kind_t;
                    kind_t kind;
                };

                virtual bool next(fileinfo_t&) = 0;

                virtual ~Directory() = default;

            protected:
                Directory();
        };

        virtual ~Filesystem() = default;

        virtual File* open(const char* path, uint32_t mode) = 0;
        virtual bool del(const char* path);
        virtual Directory* opendir(const char* path) = 0;
        virtual bool mkdir(const char* path);

        virtual void close(FilesystemObject*) = 0;
};

#endif