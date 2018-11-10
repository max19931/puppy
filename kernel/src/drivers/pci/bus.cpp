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

#include <kernel/drivers/pci/bus.h>
#include <kernel/drivers/pci/ide.h>
#include <kernel/i386/primitives.h>
#include <kernel/log/log.h>
#include <kernel/drivers/framebuffer/fb.h>
#include <kernel/libc/sprint.h>
#include <kernel/boot/phase.h>

namespace boot::pci {
    uint32_t init() {
        LOG_DEBUG("starting PCI bus scan");
        PCIBus::get();
        LOG_DEBUG("completed PCI bus scan");
        return 0;
    }
}

PCIBus& PCIBus::get() {
    static PCIBus gBus;

    return gBus;
}

#define ON_KIND(dev, claz, subclaz, ifce, func) { \
    if ((claz == 0xFF) || (claz == dev.ident.clazz)) { \
        if ((subclaz == 0xFF) || (subclaz == dev.ident.subclazz)) { \
            if ((ifce == 0xFF) || (ifce == dev.ident.progif)) { \
                PCIDevice* newdevice = func(dev); \
                if (newdevice) { mDevices.add(newdevice); } \
            } \
        } \
    } \
}

namespace {
    void printPCIDevice(const char* kind, const PCIBus::pci_hdr_0& hdr) {
        bootphase_t::printf("PCI Bus %u Slot %u Function %u:\n", hdr.endpoint.bus, hdr.endpoint.slot, hdr.endpoint.func);
        bootphase_t::printf("                              Device kind: %s\n", kind);
        bootphase_t::printf("                              Vendor: %x Device: %x Class: %u Subclass: %u ProgIF: %u\n",
            hdr.ident.vendor, hdr.ident.device, hdr.ident.clazz, hdr.ident.subclazz, hdr.ident.progif);
        bootphase_t::printf("                              BAR: [0]=%x [1]=%x [2]=%x [3]=%x [4]=%x [5]=%x\n",
            hdr.bar0, hdr.bar1, hdr.bar2, hdr.bar3, hdr.bar4, hdr.bar5);
    }
}

static PCIBus::PCIDevice* addIDEController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("IDE Disk Controller", hdr);
    return new IDEController(hdr);
}

static PCIBus::PCIDevice* printVGAController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("VGA Adapter", hdr);
    return nullptr;
}

static PCIBus::PCIDevice* printEthernetController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("Ethernet Controller", hdr);
    return nullptr;
}

static PCIBus::PCIDevice* printXHCIUSBController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("xHCI (USB 3.0) Controller", hdr);
    return nullptr;
}

static PCIBus::PCIDevice* printEHCIUSBController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("EHCI (USB 2.0) Controller", hdr);
    return nullptr;
}

static PCIBus::PCIDevice* printOHCIUSBController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("OHCI (USB 1.0) Controller", hdr);
    return nullptr;
}

static PCIBus::PCIDevice* printUHCIUSBController(const PCIBus::pci_hdr_0& hdr) {
    printPCIDevice("UCHI (USB 1.0) Controller", hdr);
    return nullptr;
}

PCIBus::PCIBus() : mDevices() {
    endpoint_t ep{
        bus : 0,
        slot : 0,
        func : 0
    };
    do {
        for (ep.slot = 0u; ep.slot < 32; ++ep.slot) {
            for (ep.func = 0u; ep.func < 8; ++ep.func) {
                auto ident = identify(ep);
                if (ident.vendor == 0xFFFF) continue;
                LOG_DEBUG("found a PCI device: (bus %u, slot %u func %u hdr %u), vendor: 0x%p, device: 0x%p, class: %u, subclass: %u, progif: %u",
                    ep.bus, ep.slot, ep.func, ident.header,
                    ident.vendor, ident.device, ident.clazz, ident.subclazz, ident.progif);
                if (0 == ident.header) {
                    auto hdr = fill(ep, ident);
                    LOG_DEBUG("bar0: 0x%p, bar1: 0x%p, bar2: 0x%p, bar3: 0x%p, bar4: 0x%p, bar5: 0x%p IRQ line: %u, IRQ PIN: %u",
                        hdr.bar0, hdr.bar1, hdr.bar2, hdr.bar3, hdr.bar4, hdr.bar5, hdr.irql, hdr.irqp);
                    ON_KIND(hdr, 1, 1, 0xFF, addIDEController);
                    ON_KIND(hdr, 3, 0, 0, printVGAController);
                    ON_KIND(hdr, 2, 0, 0xFF, printEthernetController);
                    ON_KIND(hdr, 0xC, 0x3, 0x30, printXHCIUSBController);
                    ON_KIND(hdr, 0xC, 0x3, 0x20, printEHCIUSBController);
                    ON_KIND(hdr, 0xC, 0x3, 0x10, printOHCIUSBController);
                    ON_KIND(hdr, 0xC, 0x3, 0x00, printUHCIUSBController);
                }
            }
        }
        ++ep.bus;
    } while(ep.bus != 0);
}

#undef ON_KIND

uint32_t PCIBus::endpoint_t::address() const {
    return 0x80000000 | (bus << 16) | (slot << 11) | (func << 8);
}

uint32_t PCIBus::readword(const PCIBus::endpoint_t& endpoint, uint8_t word) {
    auto addr = endpoint.address( )| (word << 2);
    outl(gConfigAddress, addr);
    return inl(gConfigData);
}

PCIBus::ident_t PCIBus::identify(const endpoint_t& endpoint) {
    uint32_t word0 = readword(endpoint, 0);
    uint32_t word1 = readword(endpoint, 1);
    uint32_t word2 = readword(endpoint, 2);
    uint32_t word3 = readword(endpoint, 3);

    return ident_t{
        vendor : (uint16_t)(word0 & 0xFFFF),
        device : (uint16_t)((word0 & 0xFFFF0000) >> 16),
        status : (uint16_t)((word1 & 0xFFFF0000) >> 16),
        command : (uint16_t)(word1 & 0xFFFF),
        clazz : (uint8_t)((word2 & 0xFF000000) >> 24),
        subclazz : (uint8_t)((word2 & 0xFF0000) >> 16),
        progif : (uint8_t)((word2 & 0xFF00) >> 8),
        revision : (uint8_t)(word2 & 0xFF),
        bist : (uint8_t)((word3 & 0xFF000000) >> 24),
        header : (uint8_t)((word3 & 0xFF0000) >> 16),
        latency : (uint8_t)((word3 & 0xFF00) >> 8),
        cache : (uint8_t)(word3 & 0xFF),
    };
}

PCIBus::pci_hdr_0 PCIBus::fill(const endpoint_t& ep, const ident_t& id) {
    #define WORD(n) uint32_t word ## n = readword(ep, n)
    WORD(4);
    WORD(5);
    WORD(6);
    WORD(7);
    WORD(8);
    WORD(9);
    WORD(10);
    WORD(11);
    WORD(12);
    WORD(13);
    WORD(15);
    #undef WORD

    return pci_hdr_0{
        endpoint : ep,
        ident : id,
        bar0 : word4,
        bar1 : word5,
        bar2 : word6,
        bar3 : word7,
        bar4 : word8,
        bar5 : word9,
        cardbus : word10,
        subsystem : (uint16_t)((word11 & 0xFFFF0000) >> 16),
        subvendor : (uint16_t)((word11 & 0xFFFF)),
        rombase : word12,
        cap : (uint8_t)(word13 & 0xFF),
        irql : (uint8_t)(word15 & 0xFF),
        irqp : (uint8_t)((word15 & 0xFF00) >> 8),
        mingrant : (uint8_t)((word15 & 0xFF0000) >> 16),
        maxlatency : (uint8_t)((word15 & 0xFF000000) >> 24),
    };
}

slist<PCIBus::PCIDevice*>::iterator PCIBus::begin() {
    return mDevices.begin();
}
slist<PCIBus::PCIDevice*>::iterator PCIBus::end() {
    return mDevices.end();
}
