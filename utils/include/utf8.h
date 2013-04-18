/******************************************************************************
 PASTRA - A light database one file system and more.
 Copyright (C) 2008  Iulian Popa

 Address: Str Olimp nr. 6
 Pantelimon Ilfov,
 Romania
 Phone:   +40721939650
 e-mail:  popaiulian@gmail.com

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef UTF8_H_
#define UTF8_H_

#include "whisper.h"

#ifdef __cplusplus
extern "C" {
#endif

static const uint8_t UTF8_7BIT_MASK  = 0x00;
static const uint8_t UTF8_11BIT_MASK = 0xC0;
static const uint8_t UTF8_16BIT_MASK = 0xE0;
static const uint8_t UTF8_21BIT_MASK = 0xF0;
static const uint8_t UTF8_26BIT_MASK = 0xF8;
static const uint8_t UTF8_31BIT_MASK = 0xFC;
static const uint8_t UTF8_37BIT_MASK = 0xFE;

static const uint8_t UTF8_EXTRA_BYTE_SIG   = 0x80;
static const uint8_t UTF8_EXTRA_BYTE_MASK  = 0xC0;
static const uint8_t UTF8_MAX_BYTES_COUNT  = 0x08;

/* Get the code units count of an UTF-8 encoded char using the
 * first code unit. */
uint_t
wh_utf8_cu_count (const uint8_t codeUnit);

/* Get the Unicode code point of the first UTF-8 encoded char. */
uint_t
wh_load_utf8_cp (const uint8_t* const utf8Str, uint32_t* const outCodePoint);

/* Store a Unicode code point using the UTF-8 encoding. */
uint_t
wh_store_utf8_cp (uint32_t codePoint, uint8_t *dest);

/* Get the reuquired code  unit to store this Unicode code point using
 * the UTF-8 encoding. */
uint_t
wh_utf8_store_size (const uint32_t codePoint);

/*  Get the Unicode code points count from an UTF-8 encoded
 *  string (null terminated). */
int
wh_utf8_strlen (const uint8_t* const utf8Str);

#ifdef __cplusplus
}
#endif

#endif /* UTF8_H_ */

