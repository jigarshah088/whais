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

#ifndef PS_BTREE_FIELDS_H_
#define PS_BTREE_FIELDS_H_

#include "ps_btree_index.h"

namespace pastra

{

template <class DBS_T>
class T_BTreeKey : public I_BTreeKey
{
public:
  T_BTreeKey (const DBS_T &value, const D_UINT64 index) :
      m_RowPart (index),
      m_ValuePart (value)
      {
      }

  bool operator< (const T_BTreeKey &key) const
  {
    if (m_ValuePart < key.m_ValuePart)
      return true;
    else if (m_ValuePart == key.m_ValuePart)
      return m_RowPart < key.m_RowPart;

    return false;
  }

  bool operator== (const T_BTreeKey  &key) const
  {
    return (m_ValuePart == key.m_ValuePart) &&
           (m_RowPart == key.m_RowPart);
  }

  bool operator> (const T_BTreeKey  &key) const
  {
    return ! ((*this < key) || (*this == key));
  }

  const D_UINT64 m_RowPart;
  const DBS_T    m_ValuePart;
};

typedef T_BTreeKey <DBSBool>      BoolBTreeKey;
typedef T_BTreeKey <DBSChar>      CharBTreeKey;
typedef T_BTreeKey <DBSUInt8>     UInt8BTreeKey;
typedef T_BTreeKey <DBSUInt16>    UInt16BTreeKey;
typedef T_BTreeKey <DBSUInt32>    UInt32BTreeKey;
typedef T_BTreeKey <DBSUInt64>    UInt64BTreeKey;
typedef T_BTreeKey <DBSInt8>      Int8BTreeKey;
typedef T_BTreeKey <DBSInt16>     Int16BTreeKey;
typedef T_BTreeKey <DBSInt32>     Int32BTreeKey;
typedef T_BTreeKey <DBSInt64>     Int64BTreeKey;
typedef T_BTreeKey <DBSDate>      DateBTreeKey;
typedef T_BTreeKey <DBSDateTime>  DateTimeBTreeKey;
typedef T_BTreeKey <DBSHiresTime> HiresTimeBTreeKey;
typedef T_BTreeKey <DBSReal>      RealBTreeKey;
typedef T_BTreeKey <DBSRichReal>  RichRealBTreeKey;



template <class T, class DBS_T, D_UINT typeSize = sizeof (T)>
class T_BTreeNode : public I_BTreeNode
{
public:
  T_BTreeNode (I_BTreeNodeManager &nodesManager, const NODE_INDEX nodeId) :
    I_BTreeNode (nodesManager, nodeId),
    m_cpNodeData (new D_UINT8 [nodesManager.GetRawNodeSize ()])
  {
    m_Header = _RC (NodeHeader*, m_cpNodeData);
    SetNullKeysCount (0);
  }
  virtual ~T_BTreeNode ()
  {
  }

  //Implementations of I_BTreeNode interface

  virtual D_UINT GetKeysPerNode () const
  {
    assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

    D_UINT result = m_NodesManager.GetRawNodeSize () - sizeof (NodeHeader);

    if (IsLeaf())
      result /= sizeof (D_UINT64) + typeSize;
    else
      {
        result /= (sizeof (D_UINT64) + typeSize + sizeof (NODE_INDEX));

        //Ensure the right alignment for NODE_INDEX by dropping rows.
        result -= (result * typeSize) % sizeof (NODE_INDEX);
      }

    return result;
  }

  virtual KEY_INDEX GetFirstKey (const I_BTreeNode &parent) const
  {
    assert (GetKeysCount () > 0);

    KEY_INDEX result = ~0;

    parent.FindBiggerOrEqual (GetKey (0), result);

    assert (parent.IsEqual (GetKey (0), result));
    assert (GetNodeId () == parent.GetChildNode (result));

    return result;
  }

  virtual NODE_INDEX GetChildNode (const KEY_INDEX keyIndex) const
  {
    assert (keyIndex < GetKeysCount ());
    assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);
    assert (IsLeaf () == false);

    const D_UINT64 *const   pRowParts   = _RC (const D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
    const D_UINT8 *const    pValueParts = _RC (const D_UINT8*, pRowParts + GetKeysPerNode ());
    const NODE_INDEX *const pChildNodes = _RC (const NODE_INDEX*,
                                               pValueParts + GetKeysPerNode () * typeSize);

    return pChildNodes [keyIndex];
  }

  virtual void ResetKeyNode (const I_BTreeNode &childNode, const KEY_INDEX keyIndex)
  {
    const T_BTreeNode &node = _SC (const T_BTreeNode&, childNode);

    SetKey (node.GetKey (0), keyIndex);
  }

  virtual void SetChildNode (const KEY_INDEX keyIndex, const NODE_INDEX childNode)
  {
    assert (keyIndex < GetKeysCount ());
    assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);
    assert (IsLeaf () == false);

    D_UINT64 *const   pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
    D_UINT8 *const    pValueParts = _RC (D_UINT8*, pRowParts + GetKeysPerNode ());
    NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*,
                                         pValueParts + GetKeysPerNode () * typeSize);

    pChildNodes [keyIndex] = childNode;
  }

  virtual KEY_INDEX InsertKey (const I_BTreeKey &key)
  {
    assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

    const T_BTreeKey<DBS_T> &theKey  = _SC (const T_BTreeKey<DBS_T>&, key);
    KEY_INDEX               keyIndex = ~0;

    if (GetKeysCount () == 0)
      {
        SetKeysCount (1);
        if (theKey.m_ValuePart.IsNull ())
          SetNullKeysCount (1);

        SetKey (theKey, 0);
        return 0;
      }
    else if (FindBiggerOrEqual (key, keyIndex) == false)
      keyIndex = 0;
    else
      ++keyIndex;

    D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
    T *const        pValueParts = _RC (T*, pRowParts + GetKeysPerNode ());
    const D_UINT    lastKey     = GetKeysCount () - 1;

    make_array_room (pRowParts, lastKey, keyIndex, 1);
    make_array_room (pValueParts, lastKey, keyIndex, 1);

    if (IsLeaf () == false)
      {
        NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pValueParts + GetKeysPerNode ());
        make_array_room (pChildNodes, lastKey, keyIndex, 1);
      }

    if (theKey.m_ValuePart.IsNull ())
      SetNullKeysCount (GetNullKeysCount () + 1);

    SetKeysCount (GetKeysCount () + 1);
    SetKey (theKey, keyIndex);

    return keyIndex;
  }

  virtual void RemoveKey (const KEY_INDEX keyIndex)
  {
    assert (keyIndex < GetKeysCount ());
    assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

    const D_UINT      lastKey     = GetKeysCount () - 1;
    D_UINT64 *const   pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
    T *const          pValuePart  = _RC (T*, pRowParts + GetKeysPerNode ());
    NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pValuePart + GetKeysPerNode ());

    remove_array_elemes (pRowParts, lastKey, keyIndex, 1);
    remove_array_elemes (pValuePart, lastKey, keyIndex, 1);

    if (IsLeaf () == false)
      remove_array_elemes (pChildNodes, lastKey, keyIndex, 1);

    if (keyIndex <= (GetKeysCount () - GetNullKeysCount ()))
      SetNullKeysCount (GetNullKeysCount () - 1);

    SetKeysCount (GetKeysCount () - 1);
  }

  virtual void Split (const NODE_INDEX parentId)
  {
    assert (NeedsSpliting ());

    const KEY_INDEX          splitKeyIndex   = GetKeysCount () / 2;
    const T_BTreeKey<DBS_T>  splitKey        = GetKey (splitKeyIndex);
    BTreeNodeHandler         parentNode (m_NodesManager.RetrieveNode (parentId));
    const KEY_INDEX          insertPosition  = parentNode->InsertKey (splitKey);
    const NODE_INDEX         allocatedNodeId = m_NodesManager.AllocateNode (parentId,
                                                                            insertPosition);
    BTreeNodeHandler         allocatedNode (m_NodesManager.RetrieveNode (allocatedNodeId));

    allocatedNode->SetLeaf (IsLeaf ());
    allocatedNode->MarkAsUsed ();
    allocatedNode->SetKeysCount (GetKeysCount () - splitKeyIndex);

    if (GetKeysCount () - GetNullKeysCount () < splitKeyIndex)
      allocatedNode->SetNullKeysCount (GetKeysCount () - splitKeyIndex);
    else
      allocatedNode->SetNullKeysCount (GetNullKeysCount ());

    SetNullKeysCount (GetNullKeysCount () - allocatedNode->GetNullKeysCount ());
    assert (GetNullKeysCount () <= GetKeysCount ());
    assert (allocatedNode->GetNullKeysCount () <= allocatedNode->GetNullKeysCount ());

    for (KEY_INDEX index = splitKeyIndex; index < GetKeysCount (); ++index)
      _SC (T_BTreeNode*, &(*allocatedNode))->SetKey ( GetKey (index), index - splitKeyIndex);

    if (IsLeaf () == false)
      for (KEY_INDEX index = splitKeyIndex; index < GetKeysCount (); ++index)
        _SC (T_BTreeNode*, &(*allocatedNode))->T_BTreeNode::SetChildNode (
                                                  T_BTreeNode::GetChildNode (index),
                                                  index - splitKeyIndex);

    SetKeysCount (splitKeyIndex);
    assert (GetNullKeysCount () <= GetKeysCount ());

    allocatedNode->SetNext (GetNodeId());
    allocatedNode->SetPrev (GetPrev());
    SetPrev (allocatedNodeId);
    if (allocatedNode->GetPrev() != NIL_NODE)
      {
        BTreeNodeHandler prevNode (m_NodesManager.RetrieveNode (allocatedNode->GetPrev()));
        prevNode->SetNext (allocatedNodeId);
      }
  }

  virtual void Join (bool toRight)
  {
    if (toRight)
      {
        assert (GetNext() != NIL_NODE);
        BTreeNodeHandler    nextNode (m_NodesManager.RetrieveNode (GetNext ()));
        T_BTreeNode *const pNextNode    = _SC (T_BTreeNode*,  &(*nextNode));
        const KEY_INDEX     oldKeysCount = pNextNode->GetKeysCount ();

        pNextNode->SetKeysCount (oldKeysCount + GetKeysCount ());
        pNextNode->SetNullKeysCount (pNextNode->GetNullKeysCount () + GetNullKeysCount ());

        for (KEY_INDEX index = 0; index < oldKeysCount; ++index)
          pNextNode->SetKey (GetKey (index), index + oldKeysCount);

        if (IsLeaf () == false)
          for (KEY_INDEX index = 0; index < oldKeysCount; ++index)
            pNextNode->T_BTreeNode::SetChildNode (T_BTreeNode::GetChildNode (index),
                                                   index + oldKeysCount);

        assert ((pNextNode->GetNullKeysCount() == 0) ||
            ( GetKeysCount () == GetNullKeysCount ()));

        nextNode->SetPrev (GetPrev ());
        if (GetPrev () != NIL_NODE)
          {
            BTreeNodeHandler prevNode (m_NodesManager.RetrieveNode (GetPrev ()));
            prevNode->SetNext (GetNext ());
          }

        assert (pNextNode->GetNullKeysCount () <= pNextNode->GetKeysCount ());
        assert (pNextNode->GetKeysCount () <= pNextNode->GetKeysPerNode());
      }
    else
      {
        assert (GetPrev () != NIL_NODE);

        BTreeNodeHandler    prevNode (m_NodesManager.RetrieveNode (GetNext ()));
        T_BTreeNode *const pPrevNode    = _SC (T_BTreeNode*, &(*prevNode));
        const KEY_INDEX     oldKeysCount = GetKeysCount ();

        SetKeysCount (oldKeysCount + pPrevNode->GetKeysCount ());
        SetNullKeysCount (GetNullKeysCount () + pPrevNode->GetKeysCount());

        for (KEY_INDEX index =0; index < pPrevNode->GetKeysCount (); ++index)
          SetKey (pPrevNode->GetKey (index), index + oldKeysCount);

        if (IsLeaf () == false)
          for (KEY_INDEX index = 0; index < oldKeysCount; ++index)
            SetChildNode (pPrevNode->T_BTreeNode::GetChildNode (index),
                          index + oldKeysCount);

        SetPrev (prevNode->GetPrev ());
        if (GetPrev () != NIL_NODE)
          {
            BTreeNodeHandler prevNode (m_NodesManager.RetrieveNode (GetPrev ()));
            prevNode->SetNext (GetNodeId ());
          }

        assert (GetNullKeysCount () <= GetKeysCount() );
        assert (GetKeysCount () <= GetKeysPerNode());
      }
  }

  virtual bool IsLess (const I_BTreeKey &key, KEY_INDEX keyIndex) const
  {
    assert (keyIndex  < GetKeysCount ());
    const T_BTreeKey<DBS_T> &theKey = _SC (const T_BTreeKey<DBS_T>&, key);

    return theKey < GetKey (keyIndex);
  }

  virtual bool IsEqual (const I_BTreeKey &key, KEY_INDEX keyIndex) const
  {
    assert (keyIndex  < GetKeysCount ());
    const T_BTreeKey<DBS_T> &theKey = _SC (const T_BTreeKey<DBS_T>&, key);

    return theKey == GetKey (keyIndex);
  }

  virtual bool IsBigger (const I_BTreeKey &key, KEY_INDEX keyIndex) const
  {
    assert (keyIndex  < GetKeysCount ());
    const T_BTreeKey<DBS_T> &theKey = _SC (const T_BTreeKey<DBS_T>&, key);

    return theKey > GetKey (keyIndex);
  }

  virtual const I_BTreeKey& GetSentinelKey () const
  {
    static T_BTreeKey<DBS_T> _sentinel (DBS_T (false, ~0), ~0);

    return _sentinel;
  }

protected:
  const T_BTreeKey<DBS_T> GetKey (const KEY_INDEX keyIndex) const
  {
    assert (keyIndex < GetKeysCount ());
    assert (GetKeysCount () <= GetKeysCount ());

    const D_UINT64 *const pRowParts = _RC (const D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

    if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
      return T_BTreeKey<DBS_T> (DBS_T(true), pRowParts [keyIndex]);

    const T *const pValueParts = _RC (const T*, pRowParts + GetKeysPerNode ());

    return T_BTreeKey<DBS_T> (DBS_T (false, pValueParts[keyIndex]), pRowParts [keyIndex]);
  }

  void SetKey (const T_BTreeKey<DBS_T> &key,const KEY_INDEX keyIndex)
  {
    assert (keyIndex < GetKeysCount ());
    assert (GetKeysCount () <= GetKeysCount ());

    D_UINT64 *const pRowParts = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

    if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
      SetNullKeysCount (GetNullKeysCount () + 1);
    else
      {
        T *const pValueParts = _RC (T*, pRowParts + GetKeysPerNode ());
        pValueParts[keyIndex] = key.m_ValuePart.m_Value;
      }

    pRowParts[keyIndex] = key.m_RowPart;
  }


  D_UINT8 * const m_cpNodeData;
};

//Specializations for DBSDate
template <> inline const DateBTreeKey
T_BTreeNode <void, DBSDate, 4>::GetKey (const KEY_INDEX keyIndex) const
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  const D_UINT64 *const pRowParts = _RC (const D_UINT64*, m_cpNodeData + sizeof (NodeHeader));


  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    return DateBTreeKey (DBSDate(true), pRowParts [keyIndex]);

  const D_INT16 *const  pYearParts  = _RC (const D_INT16*, pRowParts + GetKeysPerNode ());
  const D_UINT8 *const  pMonthParts = _RC (const D_UINT8*, pYearParts + GetKeysPerNode ());
  const D_UINT8 *const  pDayParts   = _RC (const D_UINT8*, pMonthParts + GetKeysPerNode ());

  return DateBTreeKey (DBSDate (false,
                                pYearParts [keyIndex],
                                pMonthParts [keyIndex],
                                pDayParts [keyIndex]),
                       pRowParts [keyIndex]);
}

template<> inline void
T_BTreeNode <void, DBSDate, 4>::SetKey (const DateBTreeKey &key,const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  D_UINT64 *const pRowParts = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    SetNullKeysCount (GetNullKeysCount () + 1);
  else
    {
      D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
      D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
      D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());

      pYearParts[keyIndex]  = key.m_ValuePart.m_Year;
      pMonthParts[keyIndex] = key.m_ValuePart.m_Month;
      pDayParts[keyIndex]   = key.m_ValuePart.m_Day;
    }

  pRowParts[keyIndex] = key.m_RowPart;
}

template <> inline KEY_INDEX
T_BTreeNode <void, DBSDate, 4>::InsertKey (const I_BTreeKey &key)
{
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const DateBTreeKey &theKey  = _SC (const DateBTreeKey&, key);
  KEY_INDEX          keyIndex = ~0;

  if (GetKeysCount () == 0)
    {
      SetKeysCount (1);
      if (theKey.m_ValuePart.IsNull ())
        SetNullKeysCount (1);

      SetKey (theKey, 0);
      return 0;
    }
  else if (FindBiggerOrEqual (key, keyIndex) == false)
    keyIndex = 0;
  else
    ++keyIndex;

  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());

  const D_UINT    lastKey     = GetKeysCount () - 1;

  make_array_room (pRowParts, lastKey, keyIndex, 1);
  make_array_room (pYearParts, lastKey, keyIndex, 1);
  make_array_room (pMonthParts, lastKey, keyIndex, 1);
  make_array_room (pDayParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pDayParts + GetKeysPerNode ());
      make_array_room (pChildNodes, lastKey, keyIndex, 1);
    }

  if (theKey.m_ValuePart.IsNull ())
    SetNullKeysCount (GetNullKeysCount () + 1);

  SetKeysCount (GetKeysCount () + 1);
  SetKey (theKey, keyIndex);

  return keyIndex;
}

template <> inline void
T_BTreeNode <void, DBSDate, 4>::RemoveKey (const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const D_UINT    lastKey     = GetKeysCount () - 1;
  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());

  remove_array_elemes (pRowParts, lastKey, keyIndex, 1);
  remove_array_elemes (pYearParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMonthParts, lastKey, keyIndex, 1);
  remove_array_elemes (pDayParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pDayParts + GetKeysPerNode ());
      remove_array_elemes (pChildNodes, lastKey, keyIndex, 1);
    }

  if (keyIndex <= (GetKeysCount () - GetNullKeysCount ()))
    SetNullKeysCount (GetNullKeysCount () - 1);

  SetKeysCount (GetKeysCount () - 1);
}

//Specialization for DateTime

template <> inline const DateTimeBTreeKey
T_BTreeNode <void, DBSDateTime, 7>::GetKey (const KEY_INDEX keyIndex) const
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  const D_UINT64 *const pRowParts = _RC (const D_UINT64*, m_cpNodeData + sizeof (NodeHeader));


  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    return DateTimeBTreeKey (DBSDateTime(true), pRowParts [keyIndex]);

  const D_INT16 *const  pYearParts  = _RC (const D_INT16*, pRowParts + GetKeysPerNode ());
  const D_UINT8 *const  pMonthParts = _RC (const D_UINT8*, pYearParts + GetKeysPerNode ());
  const D_UINT8 *const  pDayParts   = _RC (const D_UINT8*, pMonthParts + GetKeysPerNode ());
  const D_UINT8 *const  pHourParts  = _RC (const D_UINT8*, pDayParts + GetKeysPerNode ());
  const D_UINT8 *const  pMinParts   = _RC (const D_UINT8*, pHourParts + GetKeysPerNode ());
  const D_UINT8 *const  pSecParts   = _RC (const D_UINT8*, pHourParts + GetKeysPerNode ());

  return DateTimeBTreeKey (DBSDateTime (false,
                                        pYearParts[keyIndex],
                                        pMonthParts[keyIndex],
                                        pDayParts[keyIndex],
                                        pHourParts[keyIndex],
                                        pMinParts[keyIndex],
                                        pSecParts[keyIndex]),
                           pRowParts [keyIndex]);
}

template<> inline void
T_BTreeNode <void, DBSDateTime, 7>::SetKey (const DateTimeBTreeKey &key,const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  D_UINT64 *const pRowParts = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    SetNullKeysCount (GetNullKeysCount () + 1);
  else
    {
      D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
      D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
      D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
      D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
      D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
      D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

      pYearParts[keyIndex]  = key.m_ValuePart.m_Year;
      pMonthParts[keyIndex] = key.m_ValuePart.m_Month;
      pDayParts[keyIndex]   = key.m_ValuePart.m_Day;
      pHourParts[keyIndex]  = key.m_ValuePart.m_Hour;
      pMinParts[keyIndex]   = key.m_ValuePart.m_Minutes;
      pSecParts[keyIndex]   = key.m_ValuePart.m_Seconds;
    }

  pRowParts[keyIndex] = key.m_RowPart;
}

template <> inline KEY_INDEX
T_BTreeNode <void, DBSDateTime, 7>::InsertKey (const I_BTreeKey &key)
{
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const DateTimeBTreeKey &theKey  = _SC (const DateTimeBTreeKey&, key);
  KEY_INDEX              keyIndex = ~0;

  if (GetKeysCount () == 0)
    {
      SetKeysCount (1);
      if (theKey.m_ValuePart.IsNull ())
        SetNullKeysCount (1);

      SetKey (theKey, 0);
      return 0;
    }
  else if (FindBiggerOrEqual (key, keyIndex) == false)
    keyIndex = 0;
  else
    ++keyIndex;

  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
  D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
  D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
  D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

  const D_UINT    lastKey     = GetKeysCount () - 1;

  make_array_room (pRowParts, lastKey, keyIndex, 1);
  make_array_room (pYearParts, lastKey, keyIndex, 1);
  make_array_room (pMonthParts, lastKey, keyIndex, 1);
  make_array_room (pDayParts, lastKey, keyIndex, 1);
  make_array_room (pHourParts, lastKey, keyIndex, 1);
  make_array_room (pMinParts, lastKey, keyIndex, 1);
  make_array_room (pSecParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pSecParts + GetKeysPerNode ());
      make_array_room (pChildNodes, lastKey, keyIndex, 1);
    }

  if (theKey.m_ValuePart.IsNull ())
    SetNullKeysCount (GetNullKeysCount () + 1);

  SetKeysCount (GetKeysCount () + 1);
  SetKey (theKey, keyIndex);

  return keyIndex;
}

template <> inline void
T_BTreeNode <void, DBSDateTime, 7>::RemoveKey (const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const D_UINT    lastKey     = GetKeysCount () - 1;
  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pRowParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
  D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
  D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
  D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

  remove_array_elemes (pRowParts, lastKey, keyIndex, 1);
  remove_array_elemes (pYearParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMonthParts, lastKey, keyIndex, 1);
  remove_array_elemes (pDayParts, lastKey, keyIndex, 1);
  remove_array_elemes (pHourParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMinParts, lastKey, keyIndex, 1);
  remove_array_elemes (pSecParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pSecParts + GetKeysPerNode ());
      remove_array_elemes (pChildNodes, lastKey, keyIndex, 1);
    }

  if (keyIndex <= (GetKeysCount () - GetNullKeysCount ()))
    SetNullKeysCount (GetNullKeysCount () - 1);

  SetKeysCount (GetKeysCount () - 1);
}

//Specialization of DBSHiresTime


template <> inline const HiresTimeBTreeKey
T_BTreeNode <void, DBSHiresTime, 11>::GetKey (const KEY_INDEX keyIndex) const
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  const D_UINT64 *const pRowParts = _RC (const D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    return HiresTimeBTreeKey (DBSHiresTime(true), pRowParts [keyIndex]);

  const D_UINT32 *const pMicroParts = _RC (const D_UINT32*, pRowParts + GetKeysPerNode ());
  const D_INT16 *const  pYearParts  = _RC (const D_INT16*, pMicroParts + GetKeysPerNode ());
  const D_UINT8 *const  pMonthParts = _RC (const D_UINT8*, pYearParts + GetKeysPerNode ());
  const D_UINT8 *const  pDayParts   = _RC (const D_UINT8*, pMonthParts + GetKeysPerNode ());
  const D_UINT8 *const  pHourParts  = _RC (const D_UINT8*, pDayParts + GetKeysPerNode ());
  const D_UINT8 *const  pMinParts   = _RC (const D_UINT8*, pHourParts + GetKeysPerNode ());
  const D_UINT8 *const  pSecParts   = _RC (const D_UINT8*, pHourParts + GetKeysPerNode ());

  return HiresTimeBTreeKey (DBSHiresTime (false,
                                          pYearParts[keyIndex],
                                          pMonthParts[keyIndex],
                                          pDayParts[keyIndex],
                                          pHourParts[keyIndex],
                                          pMinParts[keyIndex],
                                          pSecParts[keyIndex],
                                          pMicroParts [keyIndex]),
                            pRowParts [keyIndex]);
}

template<> inline void
T_BTreeNode <void, DBSHiresTime, 11>::SetKey (const HiresTimeBTreeKey &key, const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (GetKeysCount () <= GetKeysCount ());

  D_UINT64 *const pRowParts = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));

  if (keyIndex <= GetKeysCount() - GetNullKeysCount ())
    SetNullKeysCount (GetNullKeysCount () + 1);
  else
    {
      D_UINT32 *const pMicroParts = _RC (D_UINT32*, pRowParts + GetKeysPerNode ());
      D_INT16 *const  pYearParts  = _RC (D_INT16*, pMicroParts + GetKeysPerNode ());
      D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
      D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
      D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
      D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
      D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

      pMicroParts[keyIndex] = key.m_ValuePart.m_Microsec;
      pYearParts[keyIndex]  = key.m_ValuePart.m_Year;
      pMonthParts[keyIndex] = key.m_ValuePart.m_Month;
      pDayParts[keyIndex]   = key.m_ValuePart.m_Day;
      pHourParts[keyIndex]  = key.m_ValuePart.m_Hour;
      pMinParts[keyIndex]   = key.m_ValuePart.m_Minutes;
      pSecParts[keyIndex]   = key.m_ValuePart.m_Seconds;
    }

  pRowParts[keyIndex] = key.m_RowPart;
}

template <> inline KEY_INDEX
T_BTreeNode <void, DBSHiresTime, 11>::InsertKey (const I_BTreeKey &key)
{
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const HiresTimeBTreeKey &theKey  = _SC (const HiresTimeBTreeKey&, key);
  KEY_INDEX               keyIndex = ~0;

  if (GetKeysCount () == 0)
    {
      SetKeysCount (1);
      if (theKey.m_ValuePart.IsNull ())
        SetNullKeysCount (1);

      SetKey (theKey, 0);
      return 0;
    }
  else if (FindBiggerOrEqual (key, keyIndex) == false)
    keyIndex = 0;
  else
    ++keyIndex;

  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_UINT32 *const pMicroParts = _RC (D_UINT32*, pRowParts + GetKeysPerNode ());
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pMicroParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
  D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
  D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
  D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

  const D_UINT    lastKey     = GetKeysCount () - 1;

  make_array_room (pRowParts, lastKey, keyIndex, 1);
  make_array_room (pMicroParts, lastKey, keyIndex, 1);
  make_array_room (pYearParts, lastKey, keyIndex, 1);
  make_array_room (pMonthParts, lastKey, keyIndex, 1);
  make_array_room (pDayParts, lastKey, keyIndex, 1);
  make_array_room (pHourParts, lastKey, keyIndex, 1);
  make_array_room (pMinParts, lastKey, keyIndex, 1);
  make_array_room (pSecParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pSecParts + GetKeysPerNode ());
      make_array_room (pChildNodes, lastKey, keyIndex, 1);
    }

  if (theKey.m_ValuePart.IsNull ())
    SetNullKeysCount (GetNullKeysCount () + 1);

  SetKeysCount (GetKeysCount () + 1);
  SetKey (theKey, keyIndex);

  return keyIndex;
}

template <> inline void
T_BTreeNode <void, DBSHiresTime, 11>::RemoveKey (const KEY_INDEX keyIndex)
{
  assert (keyIndex < GetKeysCount ());
  assert (sizeof (NodeHeader) % sizeof (D_UINT64) == 0);

  const D_UINT    lastKey     = GetKeysCount () - 1;
  D_UINT64 *const pRowParts   = _RC (D_UINT64*, m_cpNodeData + sizeof (NodeHeader));
  D_UINT32 *const pMicroParts = _RC (D_UINT32*, pRowParts + GetKeysPerNode ());
  D_INT16 *const  pYearParts  = _RC (D_INT16*, pMicroParts + GetKeysPerNode ());
  D_UINT8 *const  pMonthParts = _RC (D_UINT8*, pYearParts + GetKeysPerNode ());
  D_UINT8 *const  pDayParts   = _RC (D_UINT8*, pMonthParts + GetKeysPerNode ());
  D_UINT8 *const  pHourParts  = _RC (D_UINT8*, pDayParts + GetKeysPerNode ());
  D_UINT8 *const  pMinParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());
  D_UINT8 *const  pSecParts   = _RC (D_UINT8*, pHourParts + GetKeysPerNode ());

  remove_array_elemes (pRowParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMicroParts, lastKey, keyIndex, 1);
  remove_array_elemes (pYearParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMonthParts, lastKey, keyIndex, 1);
  remove_array_elemes (pDayParts, lastKey, keyIndex, 1);
  remove_array_elemes (pHourParts, lastKey, keyIndex, 1);
  remove_array_elemes (pMinParts, lastKey, keyIndex, 1);
  remove_array_elemes (pSecParts, lastKey, keyIndex, 1);

  if (IsLeaf () == false)
    {
      NODE_INDEX *const pChildNodes = _RC (NODE_INDEX*, pSecParts + GetKeysPerNode ());
      remove_array_elemes (pChildNodes, lastKey, keyIndex, 1);
    }

  if (keyIndex <= (GetKeysCount () - GetNullKeysCount ()))
    SetNullKeysCount (GetNullKeysCount () - 1);

  SetKeysCount (GetKeysCount () - 1);
}


typedef T_BTreeNode <D_UINT32, DBSChar>      CharBTreeNode;
typedef T_BTreeNode <D_BOOL, DBSBool>        BoolBTreeNode;
typedef T_BTreeNode <void, DBSDate, 4>       DateBTreeNode;
typedef T_BTreeNode <void, DBSDateTime, 7>   DateTimeBTreeNode;
typedef T_BTreeNode <void, DBSHiresTime, 11> HiresTimeBTreeNode;
typedef T_BTreeNode <D_UINT8, DBSUInt8>      UInt8BTreeNode;
typedef T_BTreeNode <D_UINT16, DBSUInt16>    UInt16BTreeNode;
typedef T_BTreeNode <D_UINT32, DBSUInt32>    UInt32BTreeNode;
typedef T_BTreeNode <D_UINT64, DBSUInt64>    UInt64BTreeNode;
typedef T_BTreeNode <void, DBSReal, 16>      RealBTreeNode;
typedef T_BTreeNode <void, DBSRichReal, 16>  RichRealBTreeNode;
typedef T_BTreeNode <D_INT8, DBSInt8>        Int8BTreeNode;
typedef T_BTreeNode <D_INT16, DBSInt16>      Int16BTreeNode;
typedef T_BTreeNode <D_INT32, DBSInt32>      Int32BTreeNode;
typedef T_BTreeNode <D_INT64, DBSInt64>      Int64BTreeNode;

}

#endif /* PS_BTREE_FIELDS_H_ */
