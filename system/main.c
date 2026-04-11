/*
 * QEMU System Emulator
 *
 * Copyright (c) 2003-2020 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "qemu/osdep.h"
#include "qemu-main.h"
#include "qemu/main-loop.h"
#include "system/replay.h"
#include "system/system.h"
#include "system/cpus.h"
#include "monitor/monitor.h"
#include "qapi/error.h"

#ifdef CONFIG_SDL
/*
 * SDL insists on wrapping the main() function with its own implementation on
 * some platforms; it does so via a macro that renames our main function, so
 * <SDL.h> must be #included here even with no SDL code called from this file.
 */
#include <SDL.h>
#endif

#ifdef CONFIG_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

extern const char *periodic_screenshot;
extern void take_screenshot(void);

static void *qemu_default_main(void *opaque)
{
    int status;

    replay_mutex_lock();
    bql_lock();
    status = qemu_main_loop();

    if (periodic_screenshot) {
        take_screenshot();
    }

    qemu_cleanup(status);
    bql_unlock();
    replay_mutex_unlock();

    exit(status);
}

int (*qemu_main)(void);

#ifdef CONFIG_DARWIN
static int os_darwin_cfrunloop_main(void)
{
    CFRunLoopRun();
    g_assert_not_reached();
}
int (*qemu_main)(void) = os_darwin_cfrunloop_main;
#endif

int main(int argc, char **argv)
{
    qemu_init(argc, argv);

    /*
     * qemu_init acquires the BQL and replay mutex lock. BQL is acquired when
     * initializing cpus, to block associated threads until initialization is
     * complete. Replay_mutex lock is acquired on initialization, because it
     * must be held when configuring icount_mode.
     *
     * On MacOS, qemu main event loop runs in a background thread, as main
     * thread must be reserved for UI. Thus, we need to transfer lock ownership,
     * and the simplest way to do that is to release them, and reacquire them
     * from qemu_default_main.
     */
    bql_unlock();
    replay_mutex_unlock();

    if (qemu_main) {
        QemuThread main_loop_thread;
        qemu_thread_create(&main_loop_thread, "qemu_main",
                           qemu_default_main, NULL, QEMU_THREAD_DETACHED);
        return qemu_main();
    } else {
        qemu_default_main(NULL);
        g_assert_not_reached();
    }
}

static QemuThread s_main_loop_thread;
volatile bool g_main_loop_thread_inited = false;

int respawn_main_thread(void)
{
    Error *err = NULL;

#ifdef CONFIG_POSIX
    /*
     * This thread is spawned from the old CPU thread after fork, which has
     * its signals disabled. Reset the signal mask so QEMU reacts to signals.
     */
    sigset_t set;
    sigemptyset(&set);
    pthread_sigmask(SIG_SETMASK, &set, NULL);
#endif

    bdrv_reopen_fds();
    rcu_reset();
    monitor_resurrect();
    qemu_set_io_thread_self();

    if (qemu_init_main_loop_reinit(&err)) {
        error_report_err(err);
        abort();
    }

    os_setup_signal_handling();

    qemu_thread_create(&s_main_loop_thread, "qemu_main",
                       qemu_default_main, NULL, QEMU_THREAD_DETACHED);

    g_main_loop_thread_inited = true;
    return 0;
}