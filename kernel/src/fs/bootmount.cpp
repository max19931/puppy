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

#include <boot/phase.h>
#include <log/log.h>
#include <drivers/pci/bus.h>
#include <drivers/pci/ide.h>
#include <fs/vfs.h>
#include <fs/vol/diskscanner.h>
#include <drivers/pci/diskfile.h>
#include <drivers/pci/volumefile.h>
#include <fs/devfs/devfs.h>
#include <panic/panic.h>

namespace boot::mount {
    uint32_t init() {
    	auto& pci(PCIBus::get());
    	auto& vfs(VFS::get());

        DevFS *devfs = (DevFS*)vfs.findfs("devices");
        if (devfs == nullptr) {
            PANIC("cannot found /devices");
        }

        auto ctrlid = 0u;

        for (auto b = pci.begin(); b != pci.end(); ++b, ++ctrlid) {
            auto&& pcidev(*b);
            if (pcidev && pcidev->getkind() == PCIBus::PCIDevice::kind::IDEDiskController) {
                DiskScanner scanner((IDEController*)pcidev);
                for (uint8_t ch = 0u; ch < 2; ++ch) {
                    for (uint8_t bs = 0u; bs < 2; ++bs) {
                        LOG_DEBUG("running disk scanning on controller %p ch=%u bs=%u", scanner.controller(), ch, bs);
                        auto num = scanner.parseDisk(ch, bs);
                        if (num == 0) {
                            LOG_DEBUG("no volumes found");
                            continue;
                        }
                        bool addedDisk = false;
                        for(auto&& vol : scanner) {
                            if (!addedDisk) {
                                auto diskFile = new IDEDiskFile(scanner.controller(), vol->disk(), ctrlid);
                                LOG_DEBUG("adding disk block file %s", diskFile->name());
                                devfs->add(diskFile);
                                addedDisk = true;
                            }
                            auto volumeFile = new IDEVolumeFile(vol, ctrlid);
                            LOG_DEBUG("adding partition block file %s", volumeFile->name());
                            bootphase_t::printf("Found new volume /devices/%s\n", volumeFile->name());
                            devfs->add(volumeFile);
                        }
                        scanner.clear();
                    }
                }
            }
        }

        return 0;
    }
}
