//
// io.c - simple io and input parsing routines
//
// Written by Eryk Vershen (eryk@apple.com)
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>

#include "hfdisk.h"
#include "io.h"
#include "errors.h"


//
// Defines
//
#define BAD_DIGIT 17	/* must be greater than any base */
#define	STRING_CHUNK	16
#define UNGET_MAX_COUNT 10

//
// Types
//


//
// Global Constants
//
const uint64_t kDefault = UINT64_MAX;

_Static_assert(sizeof(off_t) >= 8,
    "hfdisk requires a 64-bit off_t; build with large-file support");


//
// Global Variables
//


//
// Forward declarations
//


//
// Routines
//


int
get_okay(char *prompt)
{
    int result = 0;
    char* string = NULL;
    if (get_string_argument(prompt, &string, 1))
    {
	if (string[0] == 'Y' || string[0] == 'y')
	{
	    result = 1;
	}
    }
    free(string);
    return result;
}

	
int
get_command(char *prompt, int promptBeforeGet, int *command)
{
    int result = 0;
    char* string = NULL;
    (void)promptBeforeGet;
    if (get_string_argument(prompt, &string, 1))
    {
	*command = string[0];
	result = 1;
    }
    free(string);
    return result;

}

	
int
get_number_argument(const char *prompt, uint64_t *number, uint64_t default_value)
{
    char* buf = NULL;
    size_t buflen = 0;
    int result = 0;

	while (result == 0) {
	printf("%s", prompt);

	if (getline(&buf, &buflen, stdin) == -1) {
	    // EOF
	    break;
	} else if (default_value != kDefault && strcmp(buf, "\n") == 0) {
	    *number = default_value;
	    result = 1;
	} else {
	    char *end;
	    char *p = buf;
	    uintmax_t value;
	    uint64_t multiplier = 1;

	    while (isspace((unsigned char)*p)) {
		p++;
	    }
	    if (*p == '-') {
		continue;
	    }
	    errno = 0;
	    value = strtoumax(p, &end, 10);
	    if (p == end || errno == ERANGE || value > UINT64_MAX) {
		continue;
	    }
	    if (*end == 'g' || *end == 'G') {
		multiplier = UINT64_C(1024) * 1024 * 1024 / PBLOCK_SIZE;
		end++;
	    } else if (*end == 'm' || *end == 'M') {
		multiplier = UINT64_C(1024) * 1024 / PBLOCK_SIZE;
		end++;
	    } else if (*end == 'k' || *end == 'K') {
		multiplier = UINT64_C(1024) / PBLOCK_SIZE;
		end++;
	    }
	    while (isspace((unsigned char)*end)) {
		end++;
	    }
	    if (*end != '\0' || value > UINT64_MAX / multiplier) {
		continue;
	    }
	    *number = (uint64_t)value * multiplier;
	    result = 1;
	}
    }
    free(buf);
    return result;
}


int
get_string_argument(char *prompt, char **string, int reprompt)
{
    int result = 0;
    size_t buflen = 0;

    while (result == 0) {
	printf("%s", prompt);

	if (getline(string, &buflen, stdin) == -1)
	{
	    // EOF
	    break;
	}
	else if ((strncmp(*string, "\n", 1) == 0) && !reprompt)
	{
	    result = 0;
	    break;
	}
	else
	{
	    size_t len = strnlen(*string, buflen);
	    if ((*string)[len - 1] == '\n') {
		(*string)[len - 1] = '\0';
	    }
	    result = 1;
	}
    }
    return result;
}

int
number_of_digits(uint64_t value)
{
    int j;

    j = 1;
    while (value > 9) {
	j++;
	value = value / 10;
    }
    return j;
}


//
// Print a message on standard error & flush the input.
//
void
bad_input(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}


static int
block_offset(uint64_t num, off_t *offset)
{
    if (num > (uint64_t)INT64_MAX / PBLOCK_SIZE) {
	errno = EOVERFLOW;
	return 0;
    }
    *offset = (off_t)(num * PBLOCK_SIZE);
    return 1;
}

int
read_block(int fd, uint64_t num, char *buf, int quiet)
{
    off_t x;
    ssize_t t;

	if (!block_offset(num, &x)) {
	    if (!quiet) {
		error(errno, "Block offset is too large");
	    }
	    return 0;
	}
	{
	if ((x = lseek(fd, x, 0)) < 0) {
	    if (quiet == 0) {
		error(errno, "Can't seek on file");
	    }
	    return 0;
	}
	if ((t = read(fd, buf, PBLOCK_SIZE)) != PBLOCK_SIZE) {
	    if (quiet == 0) {
		error((t<0?errno:0), "Can't read block %" PRIu64 " from file", num);
	    }
	    return 0;
	}
	return 1;
    }
}


int
write_block(int fd, uint64_t num, char *buf)
{
    off_t x;
    ssize_t t;

    if (rflag) {
	printf("Can't write block %" PRIu64 " to file", num);
	return 0;
    }
	if (!block_offset(num, &x)) {
	error(errno, "Block offset is too large");
	return 0;
	}
    {
	if ((x = lseek(fd, x, 0)) < 0) {
	    error(errno, "Can't seek on file");
	    return 0;
	}
	if ((t = write(fd, buf, PBLOCK_SIZE)) != PBLOCK_SIZE) {
	    error((t<0?errno:0), "Can't write block %" PRIu64 " to file", num);
	    return 0;
	}
	return 1;
    }
}


int
close_device(int fildes)
{
	return close(fildes);
}


int
open_device(const char *path, int oflag)
{
	return open(path, oflag);
}
