#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "convert.h"

int
main(void)
{
    uint8_t wire[DPME_WIRE_SIZE] = {0};
    uint8_t encoded[DPME_WIRE_SIZE];
    uint8_t block[BLOCK0_WIRE_SIZE] = {0};
    DPME dpme;
    Block0 block0;
    DDMap driver = { 0x10203040, 0x5060, 0x7080 };
    DDMap decoded_driver;

    wire[0] = 0x50;
    wire[1] = 0x4d;
    wire[4] = 0x12;
    wire[5] = 0x34;
    wire[6] = 0x56;
    wire[7] = 0x78;
    wire[8] = 0x80;
    wire[9] = 0x00;
    wire[10] = 0x00;
    wire[11] = 0x00;
    wire[12] = 0x7f;
    wire[13] = 0xff;
    wire[14] = 0xff;
    wire[15] = 0xff;
    memcpy(wire + 16, "partition", 9);
    memcpy(wire + 48, "Apple_HFS", 9);
    wire[88] = 0x01;
    wire[89] = 0x02;
    wire[90] = 0x03;
    wire[91] = 0x04;
    wire[136] = 0xab;
    wire[137] = 0xad;
    wire[138] = 0xba;
    wire[139] = 0xbe;
    wire[144] = 0x11;
    wire[145] = 0x22;
    wire[146] = 0x33;
    wire[147] = 0x44;

    assert(decode_dpme(&dpme, wire) == 0);
    assert(dpme.dpme_signature == DPME_SIGNATURE);
    assert(dpme.dpme_map_entries == UINT32_C(0x12345678));
    assert(dpme.dpme_pblock_start == UINT32_C(0x80000000));
    assert(dpme.dpme_pblocks == UINT32_C(0x7fffffff));
    assert(dpme.dpme_flags == UINT32_C(0x01020304));
    {
        BZB bzb;
        assert(dpme_get_bzb(&dpme, &bzb) == 1);
        assert(bzb.bzb_magic == BZBMAGIC);
        assert(bzb.bzb_flags == UINT32_C(0x11223344));
    }
    encode_dpme(encoded, &dpme);
    assert(memcmp(encoded, wire, sizeof(wire)) == 0);

    assert(decode_block0(&block0, block) == 0);
    assert(block0_set_driver(&block0, 0, &driver) == 0);
    block0.sbDrvrCount = 1;
    encode_block0(block, &block0);
    assert(block[BLOCK0_DRIVER_MAP_OFFSET] == 0x10);
    assert(block[BLOCK0_DRIVER_MAP_OFFSET + 1] == 0x20);
    assert(block[BLOCK0_DRIVER_MAP_OFFSET + 6] == 0x70);
    assert(block[BLOCK0_DRIVER_MAP_OFFSET + 7] == 0x80);
    assert(decode_block0(&block0, block) == 0);
    assert(block0_get_driver(&block0, 0, &decoded_driver) == 0);
    assert(memcmp(&driver, &decoded_driver, sizeof(driver)) == 0);
    assert(block0_get_driver(&block0, 1, &decoded_driver) != 0);
    assert(block0_set_driver(&block0, BLOCK0_MAX_DRIVERS, &driver) != 0);
    return 0;
}
