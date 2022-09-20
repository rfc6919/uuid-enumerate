// compile with -O3 for a ~4x speedup
//
// usage: uuid-enumerate [shard_id/shard_count]
//
// if present, shard_id and shard_count allow sharding for work distribution
// shard_id is the zero-based shard number
// shard_count is the number of shards (must be a power of 2)
//
// I should probably improve error handling in the sharding code :(

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    uint64_t hi, lo, hi_reset = 0, lo_reset = 0;
    uint64_t shard_mask = 0xffffffffffffffff;
    uint64_t shard_id = 0;
    int shard_bits = 0;
    int str_len = strlen("00000000-0000-0000-0000-000000000000\n");
    char *hexlookup = "0123456789abcdef";
    uint8_t hilookup[] = {17, 16, 15, 14, 12, 11, 10, 9, 7, 6, 5, 4, 3, 2, 1, 0};
    uint8_t lolookup[] = {35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 22, 21, 20, 19};

    char *buf = malloc(str_len * 0x10000);

    if ( argv[0][2] == 'X' ) {
        hi_reset = 0xfffffffffffff000; // check final termination works
        lo_reset = 0xfffffffffff00000; // check lo -> hi wraparound works
    }

    if (argc == 2) {
        char *temp;
        uint64_t shard_count;

        shard_id = strtoull(argv[1], &temp, 0);
        if ( *temp != '/' ) {
            // bad form for the sharding argument
            fprintf(stderr, "EE: usage: %s [shard_id/shard_count]\n", argv[0]);
            exit(1);
        }
        shard_count = strtoull(++temp, NULL, 0);
        if ( (shard_count & (shard_count-1)) != 0 ) {
            fprintf(stderr, "EE: shard_count must be a power of 2\n");
            exit(2);
        }
        if (shard_id >= shard_count) {
            fprintf(stderr, "EE: shard_id must be less than shard_count\n");
            exit(3);
        }
        shard_bits = __builtin_ctzll(shard_count); // intrinsic, count rightmost zeros
        shard_mask = shard_mask >> shard_bits;
        fprintf(stderr, "II: sharding %llu/%llu bits:%u mask:%llx\n", shard_id, shard_count, shard_bits, shard_mask);
    }

    hi = (hi_reset & shard_mask) + (shard_id << (64-shard_bits));
    do {
        lo = lo_reset;
        do {
            char *base = buf + (lo & 0xffff) * str_len;
            for (int n = 0; n < 16; n++) {
                base[ hilookup[n] ] = hexlookup[ (hi >> (4*n)) & 0x0f ];
                base[ lolookup[n] ] = hexlookup[ (lo >> (4*n)) & 0x0f ];
            }
            base[8] = '-';
            base[13] = '-';
            base[18] = '-';
            base[23] = '-';
            base[36] = '\n';
            if ( (lo & 0xffff) == 0xffff) {
                write(1, buf, str_len * 0x10000);
            }
        } while ( ++lo );
    } while ( ++hi & shard_mask );

    free(buf); // don't want to leak our output buffer

    return 0;
}
