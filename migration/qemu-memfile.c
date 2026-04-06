/*
 * QEMUFile backend for in-memory snapshots
 *
 * Copyright (c) 2018 Cyberhaven
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
#include "qapi/error.h"
#include "io/channel-buffer.h"
#include "qemu-file.h"
#include "qemu-memfile.h"

QEMUFile *qemu_memfile_open(void)
{
    QIOChannelBuffer *bioc = qio_channel_buffer_new(0);
    QEMUFile *f = qemu_file_new_output(QIO_CHANNEL(bioc));
    object_unref(OBJECT(bioc));
    return f;
}

static int fill_buffer(QIOChannelBuffer *bioc, QEMUMemFileReadCb cb)
{
    uint8_t buffer[0x10000];
    size_t pos = 0;
    int ret = 0;

    do {
        ret = cb(buffer, pos, sizeof(buffer));
        if (ret > 0) {
            struct iovec iov = { .iov_base = buffer, .iov_len = (size_t)ret };
            Error *err = NULL;
            if (qio_channel_writev_all(QIO_CHANNEL(bioc), &iov, 1, &err) < 0) {
                error_free(err);
                return -1;
            }
            pos += ret;
        }
    } while (ret > 0);

    if (ret < 0) {
        fprintf(stderr, "qemu-memfile: could not read data\n");
        return ret;
    }

    return 0;
}

QEMUFile *qemu_memfile_open_ro(QEMUMemFileReadCb cb)
{
    QIOChannelBuffer *bioc = qio_channel_buffer_new(0);

    if (fill_buffer(bioc, cb) < 0) {
        object_unref(OBJECT(bioc));
        return NULL;
    }

    bioc->offset = 0;

    QEMUFile *f = qemu_file_new_input(QIO_CHANNEL(bioc));
    object_unref(OBJECT(bioc));
    return f;
}

void *qemu_memfile_get_buffer(QEMUFile *f)
{
    size_t size;
    return qemu_file_get_internal_storage(f, &size);
}
