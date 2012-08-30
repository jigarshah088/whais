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

#ifndef WFILE_H_
#define WFILE_H_

#include "whisper.h"

class EXCEP_SHL WFileException : public WException
{
public:
  WFileException (const D_CHAR* pMessage,
                  const D_CHAR* pFile,
                  D_UINT32      line,
                  D_UINT32      extra);

  virtual WException*     Clone () const;
  virtual EXPCEPTION_TYPE Type () const;
  virtual const D_CHAR*   Description () const;
};

class EXCEP_SHL WFile
{
public:
  explicit WFile (const D_CHAR* pFileName, D_UINT mode = 0);
  WFile (const WFile& rSource);
  ~WFile ();

  void     Read (D_UINT8* pBuffer, D_UINT size);
  void     Write (const D_UINT8* pBuffer, D_UINT size);
  void     Seek (const D_INT64 where, const D_INT whence);
  D_UINT64 Tell ();
  void     Sync ();
  D_UINT64 GetSize () const;
  void     SetSize (const D_UINT64 size);
  void     Close ();

  WFile&   operator= (const WFile&);

private:
  WH_FILE_HND m_Handle;
};

#endif /* WFILE_H_ */
