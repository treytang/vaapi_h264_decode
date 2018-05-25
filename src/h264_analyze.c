#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "h264_stream.h"
#include "h264_log.h"

#define BUFSIZE 32*1024*1024

static int decode(char *filename)
{
    FILE* h264_file;
    uint8_t* tmp_buf;
    h264_stream_t* stream;
    int ret = 0;

    size_t read_sz = 0;
    size_t sz = 0;
    int64_t off = 0;
    uint8_t* buf_pointer;
    int nal_start, nal_end;

    h264_file = fopen(filename, "rb");
    if (h264_file == NULL) {
        ERR("could not open file: %s.\n", strerror(errno));
        return -1;
    }

    stream = h264_new();
    if (stream == NULL) {
        ERR("new h264 stream failed.");
        ret = -1;
        goto close_file;
    }

    tmp_buf = (uint8_t *)malloc(BUFSIZE);
    if (tmp_buf == NULL) {
        ERR("alloc buf failed.");
        ret = -1
        goto free_stream;
    }

    buf_pointer = tmp_buf;

    while (!feof(h264_file))
    {
        read_sz = fread(tmp_buf + sz, 1, BUFSIZE - sz, h264_file);
        if (read_sz == 0)
        {
            if (ferror(h264_file)) {
                ERR("read failed: %s \n", strerror(errno));
                break;
            }
            break;
        }

        sz += read_sz;

        while (find_nal_unit(p, sz, &nal_start, &nal_end) > 0)
        {
           fprintf( h264_dbgfile, "!! Found NAL at offset %lld (0x%04llX), size %lld (0x%04llX) \n",
                  (long long int)(off + (p - buf) + nal_start),
                  (long long int)(off + (p - buf) + nal_start),
                  (long long int)(nal_end - nal_start),
                  (long long int)(nal_end - nal_start) );

            p += nal_start;
            read_nal_unit(stream, p, nal_end - nal_start);

            if (stream->nal->nal_unit_type == NAL_UNIT_TYPE_SPS)
            {
                // print codec parameter.
                int constraint_byte = stream->sps->constraint_set0_flag << 7;
                constraint_byte = stream->sps->constraint_set1_flag << 6;
                constraint_byte = stream->sps->constraint_set2_flag << 5;
                constraint_byte = stream->sps->constraint_set3_flag << 4;
                constraint_byte = stream->sps->constraint_set4_flag << 3;
                constraint_byte = stream->sps->constraint_set4_flag << 3;

                fprintf( h264_dbgfile, "codec: avc1.%02X%02X%02X\n",stream->sps->profile_idc, constraint_byte, stream->sps->level_idc );

                break; // we've seen enough, bailing out.
            }

            debug_nal(stream, stream->nal);

            p += (nal_end - nal_start);
            sz -= nal_end;
        }

        memmove(buf, p, sz);
        off += p - buf;
        p = buf;
    }

clear:
    free(tmp_buf);
free_stream:
    h264_free(stream);
close_file:
    fclose(h264_file);

    return ret;
}

int main(int argc, char *argv[])
{
    if (argc != 1) {
        ERR("invalid args. Usage:"
            "\th264_decode filename\n");
        return -1;
    }

    return decode(h264_file);
}

