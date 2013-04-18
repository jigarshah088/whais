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

#include "dbs_exception.h"

#include "ps_blockcache.h"

using namespace std;
using namespace whisper;
using namespace pastra;

BlockCache::BlockCache () :
    m_pManager (NULL),
    m_ItemSize (0),
    m_MaxBlocks (0),
    m_BlockSize (0),
    m_CachedBlocks ()
{
}

BlockCache::~BlockCache ()
{
  if (m_ItemSize == 0)
    return ; //This has not been initialized. So nothing to do here!

  map<uint64_t, BlockEntry>::iterator it = m_CachedBlocks.begin ();
  while (it != m_CachedBlocks.end ())
    {
      assert (it->second.IsInUse() == false);
      assert (it->second.IsDirty () == false);

      delete [] it->second.Data ();
      ++it;
    }
}

void
BlockCache::Init (I_BlocksManager& blocksMgr,
                  const uint_t     itemSize,
                  const uint_t     blockSize,
                  const uint_t     maxBlockCount)
{
  assert (itemSize > 0);
  assert (blockSize > 0);
  assert (maxBlockCount > 0);

  m_pManager = &blocksMgr;

  if ((itemSize && maxBlockCount) == 0)
    throw DBSException (NULL, _EXTRA (DBSException::INVALID_PARAMETERS));

  if ((m_ItemSize || m_BlockSize || m_MaxBlocks) != 0)
      throw DBSException (NULL, _EXTRA (DBSException::GENERAL_CONTROL_ERROR));

  m_ItemSize  = itemSize;
  m_MaxBlocks = maxBlockCount;
  m_BlockSize = blockSize;

  if (m_BlockSize < m_ItemSize)
    m_BlockSize = m_ItemSize;
}

void
BlockCache::Flush ()
{
  assert (m_ItemSize != 0);

  map<uint64_t, BlockEntry>::iterator it = m_CachedBlocks.begin ();
  while (it != m_CachedBlocks.end ())
    {
      FlushItem (it->first);
      ++it;
    }
}

StoredItem
BlockCache::RetriveItem (const uint64_t item)
{
  const uint_t   itemsPerBlock = m_BlockSize / m_ItemSize;
  const uint64_t baseBlockItem = (item / itemsPerBlock) * itemsPerBlock;

  map<uint64_t, BlockEntry>::iterator it = m_CachedBlocks.find (baseBlockItem);

  if (it != m_CachedBlocks.end ())
    return StoredItem (it->second, (item % itemsPerBlock) * m_ItemSize);

  if (m_CachedBlocks.size () >= m_MaxBlocks)
    {
      it = m_CachedBlocks.begin ();
      while (it != m_CachedBlocks.end ())
        {
          if (it->second.IsInUse ())
            continue ;

          uint8_t* const pBlockData = it->second.Data ();

          if (it->second.IsDirty ())
            m_pManager->StoreItems (pBlockData,
                                    it->first,
                                    itemsPerBlock);

          delete [] pBlockData;
          m_CachedBlocks.erase (it++);
        }
    }

  //If we are here we have to allocate something anyway
  std::auto_ptr<uint8_t> apBlockData (new uint8_t[m_BlockSize]);
  uint8_t* const         pBlockData = apBlockData.get ();

  m_CachedBlocks.insert (pair<uint64_t, BlockEntry>
                         (baseBlockItem, BlockEntry (pBlockData)));
  apBlockData.release();

  m_pManager->RetrieveItems (pBlockData, baseBlockItem, itemsPerBlock);

  return StoredItem (m_CachedBlocks.find (baseBlockItem)->second,
                     (item % itemsPerBlock) * m_ItemSize);
}

void
BlockCache::FlushItem (const uint64_t item)
{
  const uint_t   itemsPerBlock = m_BlockSize / m_ItemSize;
  const uint64_t baseBlockItem = (item / itemsPerBlock) * itemsPerBlock;

  map<uint64_t, BlockEntry>::iterator it = m_CachedBlocks.find (baseBlockItem);

  if (it == m_CachedBlocks.end ())
    return;

  if (it->second.IsDirty ())
    {
      m_pManager->StoreItems (it->second.Data (),
                              baseBlockItem,
                              itemsPerBlock);
      it->second.MarkClean ();
    }
}

void
BlockCache::RefreshItem (const uint64_t item)
{
  const uint_t   itemsPerBlock = m_BlockSize / m_ItemSize;
  const uint64_t baseBlockItem = (item / itemsPerBlock) * itemsPerBlock;

  map<uint64_t, BlockEntry>::iterator it = m_CachedBlocks.find (baseBlockItem);

  if (it == m_CachedBlocks.end ())
    return;

  assert (it->second.IsDirty () == false);
  m_pManager->RetrieveItems (it->second.Data (),
                             baseBlockItem,
                             itemsPerBlock);
}

