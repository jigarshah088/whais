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

#ifndef PM_UNITS_H_
#define PM_UNITS_H_

#include <vector>

#include <whisper.h>

namespace prima
{

struct UnitEntry
{
  D_UINT32 m_GlbsCount;
  D_UINT32 m_ProcsCount;
  D_UINT32 m_ConstSize;
  D_UINT8  m_UnitData[1];
};

class Session;
class UnitsManager
{
public:
  UnitsManager (Session& session) :
    m_Session (session),
    m_Units ()
  {
  }

  ~UnitsManager ();

  D_UINT32 LoadUnit (const D_UINT32 glbsCount,
                     const D_UINT32 procsCount,
                     const D_UINT8* pConstData,
                     const D_UINT32 constAreaSize);

  void     RemoveLastUnit ();

  void     SetGlobalIndex (const D_UINT32 unitIndex,
                           const D_UINT32 unitGlbIndex,
                           const D_UINT32 glbMgrIndex);
  void     SetProcIndex (const D_UINT32 unitIndex,
                         const D_UINT32 unitProcIndex,
                         const D_UINT32 procMgrIndex);
  D_UINT32 GetGlobalIndex (const D_UINT32 unitIndx,
                           const D_UINT32 unitGlbIndex);
  D_UINT32 GetProcIndex (const D_UINT32 unitIndex,
                         const D_UINT32 unitProcIndex);

protected:
  Session&                m_Session;
  std::vector<UnitEntry*> m_Units;
};


}

#endif /* PM_UNITS_H_ */