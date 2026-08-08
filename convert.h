//
// convert.h - Apple Partition Map wire-format conversion
//
// Written by Eryk Vershen (eryk@apple.com)
//
// Apple Partition Map fields are stored in big-endian order.  Keep
// in-memory values in host byte order and explicitly decode and encode
// the on-disk byte representation when reading and writing.  Do not
// overlay C structures directly on disk blocks.
//

/*
 * Copyright 1996 by Apple Computer, Inc.
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
#ifndef convert_h
#define convert_h

#include "dpme.h"

int decode_block0(Block0 *data, const uint8_t wire[BLOCK0_WIRE_SIZE]);
void encode_block0(uint8_t wire[BLOCK0_WIRE_SIZE], const Block0 *data);
int decode_dpme(DPME *data, const uint8_t wire[DPME_WIRE_SIZE]);
void encode_dpme(uint8_t wire[DPME_WIRE_SIZE], const DPME *data);
int dpme_get_bzb(const DPME *data, BZB *bzb);
int block0_get_driver(const Block0 *data, uint16_t index, DDMap *driver);
int block0_set_driver(Block0 *data, uint16_t index, const DDMap *driver);

#endif
