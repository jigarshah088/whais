/******************************************************************************
WHISPER - An advanced database system
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
******************************************************************************/

#ifndef WHISPER_FILEIO_H_
#define WHISPER_FILEIO_H_

#define WHC_FILECREATE          0x00000001
#define WHC_FILECREATE_NEW      0x00000002
#define WHC_FILEOPEN_EXISTING   0x00000004
#define WHC_FILEDIRECT          0x00000008
#define WHC_FILESYNC            0x00000010

#define WHC_FILEREAD            0x00000100
#define WHC_FILEWRITE           0x00000200
#define WHC_FILERDWR            (WHC_FILEREAD | WHC_FILEWRITE)

#define WHC_SEEK_BEGIN          0x00000001
#define WHC_SEEK_CURR           0x00000002
#define WHC_SEEK_END            0x00000004

#ifdef __cplusplus
extern "C"
{
#endif

WH_FILE
whf_open (const char* file, uint_t mode);

WH_FILE
whf_dup (WH_FILE hnd);

bool_t
whf_read (WH_FILE hnd, uint8_t* dstBuffer, uint_t size);

bool_t
whf_write (WH_FILE hnd, const uint8_t* srcBuffer, uint_t size);

bool_t
whf_seek (WH_FILE hnd, int64_t where, int whence);

bool_t
whf_tell (WH_FILE hnd, uint64_t* outPosition);

bool_t
whf_sync (WH_FILE hnd);

bool_t
whf_tell_size (WH_FILE hnd, uint64_t* outSize);

bool_t
whf_set_size (WH_FILE, uint64_t newSize);

bool_t
whf_close (WH_FILE hnd);

uint32_t
whf_last_error ();

bool_t
whf_err_to_str (uint64_t errorCode, char* str, uint_t strSize);

bool_t
whf_remove (const char* file);

const char*
whf_dir_delim ();

const char*
whf_current_dir ();

bool_t
whf_is_absolute (const char* path);

#ifdef __cplusplus
} /* extern 'C' */
#endif

#endif /* WHISPER_FILEIO_H_ */
