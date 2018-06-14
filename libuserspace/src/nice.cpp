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

#include <libuserspace/getpid.h>
#include <libuserspace/nice.h>
#include <libuserspace/syscalls.h>

uint8_t nice(uint16_t pid) {
    auto c = prioritize_syscall(pid, 0);
    if (c & 1) return 0;
    return (c >> 1);
}
uint8_t renice(uint16_t pid, uint8_t prio) {
    auto c = prioritize_syscall(pid, prio);
    if (c & 1) return 0;
    return c >> 1;
}

uint8_t nice() {
    return nice(getpid());
}
uint8_t renice(uint8_t prio) {
    return renice(getpid(), prio);
}

