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

#ifndef PROCESS_TABLE
#define PROCESS_TABLE

#include <kernel/sys/stdint.h>
#include <kernel/panic/panic.h>

class process_t;

template<size_t NumProcesses, typename PidType = uint16_t>
class ProcessTable {
    public:
        ProcessTable() {
            bzero((uint8_t*)&mTable[0], sizeof(mTable));
        }

        void set(process_t* proc) {
            if (proc == nullptr) {
                PANIC("null process insertion in process table");
            }
            if (mTable[proc->pid]) {
                PANIC("process slot already occupied");
            }
            mTable[proc->pid] = proc;
        }

        void free(process_t* proc) {
            mTable[proc->pid] = nullptr;
        }

        process_t* get(PidType pid) {
            return mTable[pid];
        }
    private:
        process_t* mTable[NumProcesses];
};

#endif
