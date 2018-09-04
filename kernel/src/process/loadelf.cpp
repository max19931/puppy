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

#include <kernel/process/elf.h>
#include <kernel/log/log.h>
#include <kernel/mm/virt.h>
#include <kernel/libc/string.h>
#include <kernel/libc/memory.h>
#include <kernel/process/current.h>

LOG_TAG(LOADELF, 1);

#define UNHAPPY(cause) { \
    TAG_ERROR(LOADELF, "failed to load ELF image: " #cause); \
    return loadinfo; \
}

namespace {
    struct program_args_t {
        char name[384];
        char arguments[3712];
    };
    static_assert(sizeof(program_args_t) == VirtualPageManager::gPageSize);
}

elf_load_result_t load_elf_image(elf_header_t* header) {
    ph_load_info loaded_headers[10];
    bzero(&loaded_headers[0], sizeof(loaded_headers));
    int loaded_header_idx = 0;

    elf_load_result_t result{
        ok : true,
        error : nullptr,
        max_load_addr : 0
    };
    auto maxprogaddr = header->entry;

    auto&& vmm(VirtualPageManager::get());
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    for (auto i = 0u; i < header->phnum; ++i) {
        auto&& pref = header->program(i);
        
        TAG_DEBUG(LOADELF, "program header idx = %u, offset = %p, vaddr = %p, paddr = %p, filesz = %u, memsz = %u, flags = %u, align = %u",
            i, pref.offset, pref.vaddr, pref.paddr, pref.filesz, pref.memsz, pref.flags, pref.align);
        
        if (!pref.loadable()) {
            TAG_DEBUG(LOADELF, "program header is of type %u which is not loadable", pref.type);
            continue;
        }

        if (pref.align != VirtualPageManager::gPageSize) {
            result.ok = false;
            result.error = "ELF binary non page aligned";
            return result;
        }

        auto mapopts = VirtualPageManager::map_options_t().user(true).clear(true);

        auto vaddr = (uint8_t*)pref.vaddr;

        loaded_headers[loaded_header_idx].start = pref.vaddr;
        loaded_headers[loaded_header_idx].offset = pref.offset;
        loaded_headers[loaded_header_idx].size = pref.memsz;
        loaded_headers[loaded_header_idx].virt_start = (uintptr_t)vaddr;

        auto len = pref.memsz;
        auto flen = pref.filesz;
        auto src = pref.content(header);
        auto vaddr0 = (uintptr_t)vaddr; // low address for this section
        auto vaddr1 = (uintptr_t)vaddr; // high address for this section
        while(len != 0) {
            TAG_DEBUG(LOADELF, "start by mapping a page at %p", vaddr);

            vmm.mapAnyPhysicalPage((uintptr_t)vaddr, mapopts.rw(true));
            vaddr1 += VirtualPageManager::gPageSize;

            auto chunk = len;
            if (chunk < VirtualPageManager::gPageSize) {
                TAG_DEBUG(LOADELF, "chunk size %u is < page size", chunk);
            } else if (chunk > VirtualPageManager::gPageSize) {
                TAG_DEBUG(LOADELF, "chunk size %u is > page size - truncating to page size", chunk);
                chunk = VirtualPageManager::gPageSize;
            }
            if (chunk <= flen) {
                TAG_DEBUG(LOADELF, "chunk size %u is <= flen %u - copying entire chunk from file", chunk, flen);
                memcopy(src, vaddr, chunk);
                flen -= chunk;
            } else {
                if (flen > 0) {
                    TAG_DEBUG(LOADELF, "chunk size %u is > flen %u - copying flen from disk", chunk, flen);                        
                    memcopy(src, vaddr, flen);
                    flen = 0;
                } else {
                    TAG_DEBUG(LOADELF, "chunk size %u is > flen %u", chunk, flen);                                                
                }
            }

            vmm.mapped((uintptr_t)vaddr, &mapopts);
            mapopts.rw(pref.writable());
            vmm.newoptions((uintptr_t)vaddr, mapopts);

            len -= chunk;
            src += chunk;
            vaddr += chunk;
            if ((uintptr_t)vaddr > maxprogaddr) {
                maxprogaddr = (uintptr_t)vaddr;
            }
            TAG_DEBUG(LOADELF, "len = %u flen = %u vaddr = %p src = %p", len, flen, vaddr, src);
        }
        
        memmgr->addMappedRegion(vaddr0, vaddr1-1);
        loaded_headers[loaded_header_idx++].virt_end = (uintptr_t)vaddr1-1;
    }

    result.max_load_addr = maxprogaddr;
    return result;
}

extern "C"
process_loadinfo_t load_main_binary(elf_header_t* header, size_t stacksize) {
    auto&& memmgr(gCurrentProcess->getMemoryManager());

    process_loadinfo_t loadinfo{
        eip : 0,
        stack : 0,
    };

    if (header == nullptr || !header->sanitycheck()) UNHAPPY("invalid ELF header");
    if (header->isDylib()) UNHAPPY("ELF is a shared library");

    auto elf_load_info = load_elf_image(header);
    if (elf_load_info.ok == false) {
        UNHAPPY(elf_load_info.error);
    }

    auto maxprogaddr = VirtualPageManager::page(elf_load_info.max_load_addr);
    TAG_DEBUG(LOADELF, "memory setup - max program address is %p", maxprogaddr);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    auto stackpermission = VirtualPageManager::map_options_t::userspace().clear(true);
    auto stackregion = memmgr->findAndZeroPageRegion(stacksize, stackpermission);
    TAG_DEBUG(LOADELF, "stack is begin = %p, end = %p", stackregion.to, stackregion.from);
    const auto stackbegin = stackregion.to;

    maxprogaddr = VirtualPageManager::page(stackbegin + 1);
    memmgr->addUnmappedRegion(maxprogaddr, maxprogaddr + VirtualPageManager::gPageSize - 1);

    // gcc tends to expect ESP+4 to be available; we could leave a 4-byte gap
    // but we're going to be pushing the args pointer below, so leave 16 now,
    // and then with the pointer to args, we'll be at 8-byte aligned
    loadinfo.stack = stackbegin - 16;

    uint32_t *stack = (uint32_t*)loadinfo.stack;
    if (gCurrentProcess->args || gCurrentProcess->path) {
        auto mapopts = VirtualPageManager::map_options_t().rw(true).user(true).clear(true);
        auto cmdlinergn = memmgr->findAndMapRegion(VirtualPageManager::gPageSize, mapopts);
        program_args_t* args_ptr = (program_args_t*)cmdlinergn.from;
        TAG_DEBUG(LOADELF, "cmdline pointer at %p", args_ptr);
        if (gCurrentProcess->path) {
            strncpy(args_ptr->name, gCurrentProcess->path, sizeof(args_ptr->name));
        }
        if (gCurrentProcess->args) {
            strncpy(args_ptr->arguments, gCurrentProcess->args, sizeof(args_ptr->arguments));
        }

        TAG_DEBUG(LOADELF, "args_ptr: name=%p (%s) arguments=%p (%s) - pushing on stack at %p",
            args_ptr->name, args_ptr->name,
            args_ptr->arguments, args_ptr->arguments,
            stack);

        *stack = (uintptr_t)&args_ptr->arguments[0];
        --stack;
        *stack = (uintptr_t)&args_ptr->name[0];
    } else {
        *stack = 0;
        --stack;
        *stack = 0;
    }
    --stack;
    loadinfo.stack = (uint32_t)stack;
    TAG_DEBUG(LOADELF, "loadinfo.stack = %p", loadinfo.stack);

    loadinfo.eip = header->entry;

    return loadinfo;
}
