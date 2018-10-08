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

// This file should be includable from both kernel and userspace - it is meant to contain common macros and types for the kernel <--> userspace interface
#ifndef SYSCALLS_TYPES
#define SYSCALLS_TYPES

#include <kernel/sys/stdint.h>
using syscall_response_t = uint32_t;

struct sysinfo_t {
    struct {
        uint64_t uptime; /** uptime of the system */
        uint32_t totalmem; /** total amount of RAM */
        uint32_t freemem; /** amount of free RAM */
    } global;
    struct {
        uint64_t runtime; /** time this process has been running */
        uint32_t allocated; /** amount of memory allocated to this process */
        uint32_t committed; /** amount of actual RAM given to this process */
        uint32_t pagefaults; /** number of page faults that this process caused */
    } local;
};

enum {
    INCLUDE_GLOBAL_INFO = 1,
    INCLUDE_LOCAL_INFO = 2,
};

enum {
    FILE_OPEN_READ = 1, // allow reading
    FILE_OPEN_WRITE = 2, // allow writing
    FILE_OPEN_NEW = 4, // if the file exists, create a new one anyway
    FILE_NO_CREATE = 8, // if the file does not exist, do not create it
    FILE_OPEN_APPEND = 16 // append to the file, if it exists and has content
};

enum {
    REGION_ALLOW_WRITE = 1,
};

enum {
    PROCESS_IS_FOREGROUND = 0x1, // the new process will acquire foreground on the tty
    PROCESS_INHERITS_CWD  = 0x2, // the new process will share the parent's CWD (if not set, defaults to "/")
    PROCESS_NOT_VALID_FLAG0 = 0x4,  // this is used by newlib to mean "do not inherit environment variables"
};

struct process_exit_status_t {
    enum class reason_t : uint8_t {
        alive = 0, /** this process is still alive */
        cleanExit = 1, /** this process chose to exit of its own free will */
        exception = 2, /** this process caused a fatal exception */
        killed = 3, /** this process was killed */
        kernelError = 0xFF /** the kernel encountered an issue and terminated the process */
    } reason;
    uint8_t status; /** exit code if clean-exit, error code if exception, reserved otherwise */

    uint32_t toWord() {
        return status | (((uint8_t)reason) << 24);
    }

    process_exit_status_t(reason_t r, uint8_t s) : reason(r), status(s) {}

    explicit process_exit_status_t(uint32_t word) {
        status = word & 0xFF;
        reason = (reason_t)(word >> 24);
    }
};

// IOCTL operations that one can run on a block device file
enum class blockdevice_ioctl_t {
    IOCTL_GET_SECTOR_SIZE,   // (a=IOCTL_, b=0), returns sector size
    IOCTL_GET_NUM_SECTORS,   // (a=IOCTL_, b=0), returns number of sectors
    IOCTL_GET_CONTROLLER,    // (a=IOCTL_, b=0), returns an opaque disk controller descriptor
    IOCTL_GET_ROUTING,       // (a=IOCTL_, b=0), returns an opaque routing descriptor
    IOCTL_GET_VOLUME,        // (a=IOCTL_, b=0), returns a Volume* suitable for mounting
};

// IOCTL operations that one can run on a TTY
enum tty_ioctl_t {
        IOCTL_FOREGROUND = 1, // a2 = pid
        IOCTL_BACKGROUND = 2, // a2 = reserved
        IOCTL_MOVECURSOR = 3, // a2 = low = row, high = col
        IOCTL_VISIBLE_AREA = 4, // a2 = pointer to 0xRRRRCCCC
        IOCTL_CURSOR_POS = 5, // a2 = pointer to 0xRRRRCCCC
        IOCTL_GET_FG_COLOR = 6, // a2 = pointer to 0x00RRGGBB
        IOCTL_SET_FG_COLOR = 7, // a2 = 0x00RRGGBB
        IOCTL_GET_BG_COLOR = 8, // a2 = pointer to 0x00RRGGBB
        IOCTL_SET_BG_COLOR = 9, // a2 = 0x00RRGGBB
};

// IOCTL operations that can run on a msgqueue
enum msgqueue_ioctl_t {
    IOCTL_GET_QUEUE_SIZE = 1, // a2 = reserved, return size of queue
    IOCTL_BLOCK_ON_EMPTY = 2, // a2 = bool; should a read block if the queue is empty
    IOCTL_BLOCK_ON_FULL = 3, // a2 = bool; should a write block if the queue is full
};

struct message_t {
    uint64_t time;
    uint32_t sender;
    uint32_t arg1;
    uint32_t arg2;
};

enum class process_state_t : uint8_t {
    NEW, /** created and not schedulable */
    AVAILABLE, /** ready to be scheduled (or running, we don't distinguish yet) */
    WAITING, /** in a waitqueue */
    SLEEPING, /** waiting to be woken up at sleeptill */
    EXITED, /** waiting for parent to collect */
    COLLECTED, /** waiting to be torn down by the system */
    COLLECTING, /** waiting to collect a child */
};

static constexpr size_t gMaxPathSize = 255;
typedef char path_name_t[gMaxPathSize + 1];

enum class file_kind_t {
    directory,
    file,
    blockdevice, /** see blockdevice_ioctl */
    pipe,
    msgqueue, /** see msgqueue_ioctl */
};

struct file_stat_t {
    file_kind_t kind;
    uint32_t size;
    uint64_t time;
};

struct file_info_t {
    using kind_t = file_kind_t;
    uint32_t size;
    uint64_t time;
    path_name_t name;
    file_kind_t kind;
};

// make sure that userspace can define its own pid_t with different requirements than our version here
typedef uint16_t kpid_t;

struct process_info_t {
    static constexpr size_t gMaxCommandLineSize = 128;

    kpid_t pid;
    kpid_t ppid;
    process_state_t state;
    char path[gMaxPathSize];
    char args[gMaxCommandLineSize];

    uintptr_t vmspace;
    uintptr_t pmspace;
    uint64_t runtime;
};

struct klog_stats_t {
    uint64_t numentries;
    uint64_t totalwritten;
    size_t   largestentry;
};

static constexpr uint32_t TIOCGWINSZ = 0x40087468;
struct winsize_t { // the return of TIOCGWINSZ
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

struct exec_fileop_t {
    enum class operation : uint8_t {
        END_OF_LIST = 0, // all param values ignored; stop parsing
        CLOSE_CHILD_FD, // param1, the file descriptor to close
        DUP_PARENT_FD, // param1, the file descriptor to fdup - on return, param2 contains the descriptor in the child
        DUP_CHILD_FD, // param1, the file descriptor to fdup
    };

    operation op;
    size_t  param1;
    size_t  param2; // reserved for future evolutions
    void*     param3; // reserved for future evolutions
};

// TODO: deprecate old message_t and rename this new one
struct new_message_t {
    static constexpr size_t gTotalSize = 4096; // TODO: expose the size of a page globally
    struct header_t {
        kpid_t sender;
        uint64_t timestamp;
        size_t payload_size;
    } header;
    static constexpr size_t gBodySize = gTotalSize - sizeof(header_t);
    uint8_t payload[gBodySize];
};

static_assert(sizeof(new_message_t) == new_message_t::gTotalSize);

#endif
