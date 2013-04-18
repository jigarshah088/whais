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

#ifndef PM_GLOBALS_H_
#define PM_GLOBALS_H_

#include <vector>

#include "whisper.h"

#include "pm_operand.h"

namespace whisper {
namespace prima {

class NameSpace;

struct GlobalEntry
{
  uint32_t  m_IdOffet;
  uint32_t  m_TypeOffset;
};

class GlobalsManager
{
public:
  GlobalsManager (NameSpace& space)
    : m_Names (space),
      m_Identifiers (),
      m_Storage (),
      m_GlobalsEntrys ()
  {
  }

  ~GlobalsManager ();

  uint_t Count () const { return m_GlobalsEntrys.size (); };

  uint32_t           AddGlobal (const uint8_t*     pName,
                                const uint_t       nameLength,
                                GlobalValue&       value,
                                const uint32_t     tiOffset);
  uint32_t           FindGlobal (const uint8_t *const pName,
                                 const uint_t         nameLength);

  GlobalValue&       GetGlobal (const uint32_t glbId);
  const uint8_t*     Name (const uint_t index) const;
  const uint8_t*     GetGlobalTI (const uint32_t glbId);

  static bool IsValid (const uint32_t glbId)
  {
    return glbId != INVALID_ENTRY;
  }

  static bool IsGlobalEntry (const uint32_t glbId)
  {
    return IsValid (glbId) && ((glbId & GLOBAL_ID) != 0);
  }

  static void MarkAsGlobalEntry (uint32_t& glbId)
  {
    glbId |= GLOBAL_ID;
  }

private:
  GlobalsManager (const GlobalsManager&);
  GlobalsManager& operator= (const GlobalsManager);

  static const uint32_t GLOBAL_ID     = 0x80000000;
  static const uint32_t INVALID_ENTRY = 0xFFFFFFFF;

  NameSpace&               m_Names;
  std::vector<uint8_t>     m_Identifiers;
  std::vector<GlobalValue> m_Storage;
  std::vector<GlobalEntry> m_GlobalsEntrys;
};

} //namespace prima
} //namespace whisper


#endif /* PM_GLOBALS_H_ */
