//
// convert.c - Apple Partition Map wire-format conversion
//
// Written by Eryk Vershen (eryk@apple.com)
//
// See comments in convert.h
//

/*
 * Copyright 1996,1997 by Apple Computer, Inc.
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

#include <string.h>

#include "convert.h"

static uint16_t
load_be16(const uint8_t *p)
{
    return ((uint16_t)p[0] << 8) | p[1];
}

static uint32_t
load_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
        | ((uint32_t)p[2] << 8) | p[3];
}

static void
store_be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8);
    p[1] = (uint8_t)value;
}

static void
store_be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24);
    p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8);
    p[3] = (uint8_t)value;
}

int
decode_dpme(DPME *data, const uint8_t wire[DPME_WIRE_SIZE])
{
    memset(data, 0, sizeof(*data));
    data->dpme_signature = load_be16(wire + 0);
    data->dpme_reserved_1 = load_be16(wire + 2);
    data->dpme_map_entries = load_be32(wire + 4);
    data->dpme_pblock_start = load_be32(wire + 8);
    data->dpme_pblocks = load_be32(wire + 12);
    memcpy(data->dpme_name, wire + 16, sizeof(data->dpme_name));
    memcpy(data->dpme_type, wire + 48, sizeof(data->dpme_type));
    data->dpme_lblock_start = load_be32(wire + 80);
    data->dpme_lblocks = load_be32(wire + 84);
    data->dpme_flags = load_be32(wire + 88);
    data->dpme_boot_block = load_be32(wire + 92);
    data->dpme_boot_bytes = load_be32(wire + 96);
    data->dpme_load_addr = load_be32(wire + 100);
    data->dpme_load_addr_2 = load_be32(wire + 104);
    data->dpme_goto_addr = load_be32(wire + 108);
    data->dpme_goto_addr_2 = load_be32(wire + 112);
    data->dpme_checksum = load_be32(wire + 116);
    memcpy(data->dpme_process_id, wire + 120, sizeof(data->dpme_process_id));
    memcpy(data->dpme_boot_args, wire + 136, sizeof(data->dpme_boot_args));
    memcpy(data->dpme_reserved_3, wire + 264, sizeof(data->dpme_reserved_3));
    return 0;
}

void
encode_dpme(uint8_t wire[DPME_WIRE_SIZE], const DPME *data)
{
    memset(wire, 0, DPME_WIRE_SIZE);
    store_be16(wire + 0, data->dpme_signature);
    store_be16(wire + 2, data->dpme_reserved_1);
    store_be32(wire + 4, data->dpme_map_entries);
    store_be32(wire + 8, data->dpme_pblock_start);
    store_be32(wire + 12, data->dpme_pblocks);
    memcpy(wire + 16, data->dpme_name, sizeof(data->dpme_name));
    memcpy(wire + 48, data->dpme_type, sizeof(data->dpme_type));
    store_be32(wire + 80, data->dpme_lblock_start);
    store_be32(wire + 84, data->dpme_lblocks);
    store_be32(wire + 88, data->dpme_flags);
    store_be32(wire + 92, data->dpme_boot_block);
    store_be32(wire + 96, data->dpme_boot_bytes);
    store_be32(wire + 100, data->dpme_load_addr);
    store_be32(wire + 104, data->dpme_load_addr_2);
    store_be32(wire + 108, data->dpme_goto_addr);
    store_be32(wire + 112, data->dpme_goto_addr_2);
    store_be32(wire + 116, data->dpme_checksum);
    memcpy(wire + 120, data->dpme_process_id, sizeof(data->dpme_process_id));
    memcpy(wire + 136, data->dpme_boot_args, sizeof(data->dpme_boot_args));
    memcpy(wire + 264, data->dpme_reserved_3, sizeof(data->dpme_reserved_3));
}

int
dpme_get_bzb(const DPME *data, BZB *bzb)
{
    const uint8_t *wire = data->dpme_boot_args;

    if (load_be32(wire) != BZBMAGIC) {
        return 0;
    }
    memset(bzb, 0, sizeof(*bzb));
    bzb->bzb_magic = BZBMAGIC;
    bzb->bzb_cluster = wire[4];
    bzb->bzb_type = wire[5];
    bzb->bzb_inode = load_be16(wire + 6);
    bzb->bzb_flags = load_be32(wire + 8);
    bzb->bzb_tmade = load_be32(wire + 12);
    bzb->bzb_tmount = load_be32(wire + 16);
    bzb->bzb_tumount = load_be32(wire + 20);
    bzb->bzb_abm.abm_size = load_be32(wire + 24);
    bzb->bzb_abm.abm_ents = load_be32(wire + 28);
    bzb->bzb_abm.abm_start = load_be32(wire + 32);
    memcpy(bzb->bzb_fill2, wire + 36, sizeof(bzb->bzb_fill2));
    memcpy(bzb->bzb_mount_point, wire + 64, sizeof(bzb->bzb_mount_point));
    return 1;
}

int
decode_block0(Block0 *data, const uint8_t wire[BLOCK0_WIRE_SIZE])
{
    memset(data, 0, sizeof(*data));
    data->sbSig = load_be16(wire + 0);
    data->sbBlkSize = load_be16(wire + 2);
    data->sbBlkCount = load_be32(wire + 4);
    data->sbDevType = load_be16(wire + 8);
    data->sbDevId = load_be16(wire + 10);
    data->sbData = load_be32(wire + 12);
    data->sbDrvrCount = load_be16(wire + 16);
    memcpy(data->sbMap, wire + BLOCK0_DRIVER_MAP_OFFSET, sizeof(data->sbMap));
    return 0;
}

void
encode_block0(uint8_t wire[BLOCK0_WIRE_SIZE], const Block0 *data)
{
    memset(wire, 0, BLOCK0_WIRE_SIZE);
    store_be16(wire + 0, data->sbSig);
    store_be16(wire + 2, data->sbBlkSize);
    store_be32(wire + 4, data->sbBlkCount);
    store_be16(wire + 8, data->sbDevType);
    store_be16(wire + 10, data->sbDevId);
    store_be32(wire + 12, data->sbData);
    store_be16(wire + 16, data->sbDrvrCount);
    memcpy(wire + BLOCK0_DRIVER_MAP_OFFSET, data->sbMap, sizeof(data->sbMap));
}

int
block0_get_driver(const Block0 *data, uint16_t index, DDMap *driver)
{
    const uint8_t *wire;

    if (index >= data->sbDrvrCount || index >= BLOCK0_MAX_DRIVERS) {
        return -1;
    }
    wire = data->sbMap + ((size_t)index * 8);
    driver->ddBlock = load_be32(wire);
    driver->ddSize = load_be16(wire + 4);
    driver->ddType = load_be16(wire + 6);
    return 0;
}

int
block0_set_driver(Block0 *data, uint16_t index, const DDMap *driver)
{
    uint8_t *wire;

    if (index >= BLOCK0_MAX_DRIVERS) {
        return -1;
    }
    wire = data->sbMap + ((size_t)index * 8);
    store_be32(wire, driver->ddBlock);
    store_be16(wire + 4, driver->ddSize);
    store_be16(wire + 6, driver->ddType);
    return 0;
}
