#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "errors.h"
#include "io.h"

int rflag;
int hflag;
char *program_name = "test_large_file";

int
main(void)
{
    char path[] = "/tmp/hfdisk-large-file.XXXXXX";
    char write_buf[PBLOCK_SIZE];
    char read_buf[PBLOCK_SIZE];
    uint64_t block = UINT64_C(4194304); /* byte offset: exactly 2 GiB */
    int fd = mkstemp(path);

    assert(fd >= 0);
    memset(write_buf, 0xa5, sizeof(write_buf));
    assert(write_block(fd, block, write_buf) == 1);
    assert(lseek(fd, 0, SEEK_END) == (off_t)((block + 1) * PBLOCK_SIZE));
    memset(read_buf, 0, sizeof(read_buf));
    assert(read_block(fd, block, read_buf, 0) == 1);
    assert(memcmp(write_buf, read_buf, sizeof(write_buf)) == 0);
    assert(close(fd) == 0);
    assert(unlink(path) == 0);
    return 0;
}
