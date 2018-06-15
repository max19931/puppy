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

#include <kernel/log/log.h>
#include <kernel/libc/sprint.h>
#include <kernel/drivers/serial/serial.h>
#include <kernel/drivers/pit/pit.h>
#include <kernel/sys/config.h>

void logimpl(const char* filename, size_t line, const char* msg, va_list args) {
    if (gKernelConfiguration()->logging.value == kernel_config_t::config_logging::gNoLogging)
        return;

    char gBuffer[1027] = {0};

    size_t idx = sprint(gBuffer, 1024, "[%llu] %s:%lu ", PIT::getUptime(), filename, line);
    idx += vsprint(gBuffer+idx, 1024-idx, msg, args);
    gBuffer[idx++] = '\n'; gBuffer[idx++] = '\0';

    // TODO write somewhere other than COM1
    Serial::get().write(gBuffer);
}
