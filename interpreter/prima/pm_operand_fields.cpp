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

#include "pm_operand.h"

#include "pm_typemanager.h"

using namespace std;
using namespace prima;

/////////////////////////////TableOperand//////////////////////////////////////

TableOperand::~TableOperand ()
{
  m_pRefTable->DecrementRefCount ();
}

bool
TableOperand::IsNull () const
{
  return (m_pRefTable->GetTable ().GetAllocatedRows () == 0);
}

I_DBSTable&
TableOperand::GetTable ()
{
  return m_pRefTable->GetTable ();
}

/////////////////////////FieldOperand//////////////////////////////////////////

FieldOperand::FieldOperand (TableOperand& tableOp, const FIELD_INDEX field)
  : I_Operand (),
    m_pRefTable (&tableOp.GetTableRef ()),
    m_Field (field)
{
  m_pRefTable->IncrementRefCount ();

  I_DBSTable&              table     = m_pRefTable->GetTable ();
  const DBSFieldDescriptor fieldDesc = table.GetFieldDescriptor (field);

  m_FieldType = fieldDesc.m_FieldType;
  assert ((m_FieldType > T_UNKNOWN) && (m_FieldType < T_UNDETERMINED));

  if (fieldDesc.isArray)
    {
      assert (m_FieldType != T_TEXT);
      MARK_ARRAY (m_FieldType);
    }
}

FieldOperand::FieldOperand (const FieldOperand& source)
  : m_pRefTable (source.m_pRefTable),
    m_Field (source.m_Field),
    m_FieldType (source.m_FieldType)
{
  if (m_pRefTable)
    m_pRefTable->IncrementRefCount ();
}

FieldOperand::~FieldOperand ()
{
  if (m_pRefTable)
    m_pRefTable->DecrementRefCount ();
}

const FieldOperand&
FieldOperand::operator= (const FieldOperand& source)
{
  if (this != &source)
    {
      if ((m_FieldType != T_UNDETERMINED) &&
          (m_FieldType != source.m_FieldType))
        {
          throw InterException (NULL,
                                _EXTRA (InterException::FIELD_TYPE_MISMATCH));
        }

      if (m_pRefTable != NULL)
        m_pRefTable->DecrementRefCount ();

      m_pRefTable = source.m_pRefTable;
      m_Field     = source.m_Field;

      if (m_pRefTable)
        m_pRefTable->IncrementRefCount ();
    }

  return *this;
}

bool
FieldOperand::IsNull () const
{
  return (m_pRefTable == NULL);
}

FIELD_INDEX
FieldOperand::GetField ()
{
  return m_Field;
}

I_DBSTable&
FieldOperand::GetTable ()
{

  return m_pRefTable->GetTable ();
}

StackValue
FieldOperand::GetValueAt (const D_UINT64 index)
{

  if ((m_FieldType == T_UNKNOWN) || (m_pRefTable == NULL))
    {
      throw InterException (NULL,
                            _EXTRA (InterException::FIELD_TYPE_MISMATCH));
    }

  if (IS_ARRAY (m_FieldType))
    return StackValue (ArrayFieldElOperand (m_pRefTable, index, m_Field));

  switch (m_FieldType)
  {
  case T_BOOL:
    return StackValue (BoolFieldElOperand (m_pRefTable, index, m_Field));
  case T_CHAR:
    return StackValue (CharFieldElOperand (m_pRefTable, index, m_Field));
  case T_DATE:
    return StackValue (DateFieldElOperand (m_pRefTable, index, m_Field));
  case T_DATETIME:
    return StackValue (DateTimeFieldElOperand (m_pRefTable, index, m_Field));
  case T_HIRESTIME:
    return StackValue (HiresTimeFieldElOperand (m_pRefTable, index, m_Field));
  case T_UINT8:
    return StackValue (UInt8FieldElOperand (m_pRefTable, index, m_Field));
  case T_UINT16:
    return StackValue (UInt16FieldElOperand (m_pRefTable, index, m_Field));
  case T_UINT32:
    return StackValue (UInt32FieldElOperand (m_pRefTable, index, m_Field));
  case T_UINT64:
    return StackValue (UInt64FieldElOperand (m_pRefTable, index, m_Field));
  case T_INT8:
    return StackValue (Int8FieldElOperand (m_pRefTable, index, m_Field));
  case T_INT16:
    return StackValue (Int16FieldElOperand (m_pRefTable, index, m_Field));
  case T_INT32:
    return StackValue (Int32FieldElOperand (m_pRefTable, index, m_Field));
  case T_INT64:
    return StackValue (Int64FieldElOperand (m_pRefTable, index, m_Field));
  case T_REAL:
    return StackValue (RealFieldElOperand (m_pRefTable, index, m_Field));
  case T_RICHREAL:
    return StackValue (RichRealFieldElOperand (m_pRefTable, index, m_Field));
  case T_TEXT:
    return StackValue (TextFieldElOperand (m_pRefTable, index, m_Field));
  }

  throw InterException (NULL, _EXTRA (InterException::INTERNAL_ERROR));
}

///////////////////////BaseFieldElOperand//////////////////////////////////////

BaseFieldElOperand::~BaseFieldElOperand ()
{
  m_pRefTable->DecrementRefCount ();
}

//////////////////////////BoolFieldElOperand///////////////////////////////////

bool
BoolFieldElOperand::IsNull () const
{
  DBSBool currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
BoolFieldElOperand::GetValue (DBSBool& outValue) const
{
  Get (outValue);
}

void
BoolFieldElOperand::SetValue (const DBSBool& value)
{
  Set (value);
}

void
BoolFieldElOperand::SelfAnd (const DBSBool& value)
{
  DBSBool currValue;
  Get (currValue);

  currValue = internal_and (currValue, value);

  Set (currValue);
}

void
BoolFieldElOperand::SelfXor (const DBSBool& value)
{
  DBSBool currValue;
  Get (currValue);

  currValue = internal_xor (currValue, value);

  Set (currValue);
}

void
BoolFieldElOperand::SelfOr (const DBSBool& value)
{
  DBSBool currValue;
  Get (currValue);

  currValue = internal_or (currValue, value);

  Set (currValue);
}

//////////////////////////CharFieldElOperand///////////////////////////////////

bool
CharFieldElOperand::IsNull () const
{
  DBSChar currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
CharFieldElOperand::GetValue (DBSChar& outValue) const
{
  Get (outValue);
}

void
CharFieldElOperand::GetValue (DBSText& outValue) const
{
  DBSChar ch;
  Get (ch);

  outValue = DBSText ();
  outValue.Append (ch);
}

void
CharFieldElOperand::SetValue (const DBSChar& value)
{
  Set (value);
}

//////////////////////////DateFieldElOperand///////////////////////////////////

bool
DateFieldElOperand::IsNull () const
{
  DBSDate currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
DateFieldElOperand::GetValue (DBSDate& outValue) const
{
  Get (outValue);
}

void
DateFieldElOperand::GetValue (DBSDateTime& outValue) const
{
  DBSDate currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSDateTime ();
  else
    outValue = DBSDateTime (currValue.m_Year,
                            currValue.m_Month,
                            currValue.m_Day,
                            0,
                            0,
                            0);
}

void
DateFieldElOperand::GetValue (DBSHiresTime& outValue) const
{
  DBSDate currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSHiresTime ();
  else
    outValue = DBSHiresTime (currValue.m_Year,
                             currValue.m_Month,
                             currValue.m_Day,
                             0,
                             0,
                             0,
                             0);
}

void
DateFieldElOperand::SetValue (const DBSDate& value)
{
  Set (value);
}

/////////////////////////DateTimeFieldElOperand////////////////////////////////

bool
DateTimeFieldElOperand::IsNull () const
{
  DBSDateTime currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
DateTimeFieldElOperand::GetValue (DBSDate& outValue) const
{
  DBSDateTime currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSDate ();
  else
    outValue = DBSDate (currValue.m_Year, currValue.m_Month, currValue.m_Day);
}

void
DateTimeFieldElOperand::GetValue (DBSDateTime& outValue) const
{
  Get (outValue);
}

void
DateTimeFieldElOperand::GetValue (DBSHiresTime& outValue) const
{
  DBSDateTime currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSHiresTime ();
  else
    outValue = DBSHiresTime (currValue.m_Year,
                             currValue.m_Month,
                             currValue.m_Day,
                             currValue.m_Hour,
                             currValue.m_Minutes,
                             currValue.m_Seconds,
                             0);
}

void
DateTimeFieldElOperand::SetValue (const DBSDateTime& value)
{
  Set (value);
}

///////////////////////HiresTimeFieldElOperand/////////////////////////////////

bool
HiresTimeFieldElOperand::IsNull () const
{
  DBSHiresTime currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
HiresTimeFieldElOperand::GetValue (DBSDate& outValue) const
{
  DBSHiresTime currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSDate ();
  else
    outValue = DBSDate (currValue.m_Year, currValue.m_Month, currValue.m_Day);
}

void
HiresTimeFieldElOperand::GetValue (DBSDateTime& outValue) const
{
  DBSHiresTime currValue;
  Get (currValue);

  if (currValue.IsNull ())
    outValue = DBSDateTime ();
  else
    outValue = DBSDateTime (currValue.m_Year,
                            currValue.m_Month,
                            currValue.m_Day,
                            currValue.m_Hour,
                            currValue.m_Minutes,
                            currValue.m_Seconds);
}

void
HiresTimeFieldElOperand::GetValue (DBSHiresTime& outValue) const
{
  Get (outValue);
}

void
HiresTimeFieldElOperand::SetValue (const DBSHiresTime& value)
{
  Set (value);
}

//////////////////////////UInt8FieldElOperand//////////////////////////////////

bool
UInt8FieldElOperand::IsNull () const
{
  DBSUInt8 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
UInt8FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  Get (outValue);
}

void
UInt8FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSUInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt8FieldElOperand::SetValue (const DBSUInt8& value)
{
  Set (value);
}

void
UInt8FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_and (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_xor (currValue, value);

  Set (currValue);
}

void
UInt8FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSUInt8 currValue;
  Get (currValue);

  currValue = internal_or (currValue, value);

  Set (currValue);
}

/////////////////////////UInt16FieldElOperand//////////////////////////////////

bool
UInt16FieldElOperand::IsNull () const
{
  DBSUInt16 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
UInt16FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  Get (outValue);
}

void
UInt16FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSUInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt16FieldElOperand::SetValue (const DBSUInt16& value)
{
  Set (value);
}

void
UInt16FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
UInt16FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSUInt16 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////UInt32FieldElOperand//////////////////////////////////

bool
UInt32FieldElOperand::IsNull () const
{
  DBSUInt32 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
UInt32FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  Get (outValue);
}

void
UInt32FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSUInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt32FieldElOperand::SetValue (const DBSUInt32& value)
{
  Set (value);
}

void
UInt32FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
UInt32FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSUInt32 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////UInt64FieldElOperand//////////////////////////////////

bool
UInt64FieldElOperand::IsNull () const
{
  DBSUInt64 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
UInt64FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSUInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
UInt64FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  Get (outValue);
}

void
UInt64FieldElOperand::SetValue (const DBSUInt64& value)
{
  Set (value);
}

void
UInt64FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
UInt64FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSUInt64 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

//////////////////////////Int8FieldElOperand//////////////////////////////////

bool
Int8FieldElOperand::IsNull () const
{
  DBSInt8 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
Int8FieldElOperand::GetValue (DBSInt8& outValue) const
{
  Get (outValue);
}

void
Int8FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSInt8 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int8FieldElOperand::SetValue (const DBSInt8& value)
{
  Set (value);
}

void
Int8FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
Int8FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSInt8 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////Int16FieldElOperand//////////////////////////////////

bool
Int16FieldElOperand::IsNull () const
{
  DBSInt16 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
Int16FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSInt16& outValue) const
{
  Get (outValue);
}

void
Int16FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSInt16 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int16FieldElOperand::SetValue (const DBSInt16& value)
{
  Set (value);
}

void
Int16FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
Int16FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSInt16 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////Int32FieldElOperand//////////////////////////////////

bool
Int32FieldElOperand::IsNull () const
{
  DBSInt32 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
Int32FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSInt32& outValue) const
{
  Get (outValue);
}

void
Int32FieldElOperand::GetValue (DBSInt64& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSInt32 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int32FieldElOperand::SetValue (const DBSInt32& value)
{
  Set (value);
}

void
Int32FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
Int32FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSInt32 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////Int64FieldElOperand//////////////////////////////////

bool
Int64FieldElOperand::IsNull () const
{
  DBSInt64 currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
Int64FieldElOperand::GetValue (DBSInt8& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSInt16& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSInt32& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSInt64& outValue) const
{
  Get (outValue);
}

void
Int64FieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSUInt8& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSUInt16& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSUInt32& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::GetValue (DBSUInt64& outValue) const
{
  DBSInt64 currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
Int64FieldElOperand::SetValue (const DBSInt64& value)
{
  Set (value);
}

void
Int64FieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfMod (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_mod (currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfAnd (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_and(currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfXor (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_xor(currValue, value);

  Set (currValue);
}

void
Int64FieldElOperand::SelfOr (const DBSInt64& value)
{
  DBSInt64 currValue;
  Get (currValue);

  currValue = internal_or(currValue, value);

  Set (currValue);
}

/////////////////////////RealFieldElOperand//////////////////////////////////

bool
RealFieldElOperand::IsNull () const
{
  DBSReal currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
RealFieldElOperand::GetValue (DBSReal& outValue) const
{
  Get (outValue);
}

void
RealFieldElOperand::GetValue (DBSRichReal& outValue) const
{
  DBSReal currValue;
  Get (currValue);

  number_convert (currValue, outValue);
}

void
RealFieldElOperand::SetValue (const DBSReal& value)
{
  Set (value);
}

void
RealFieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfAdd (const DBSRichReal& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfSub (const DBSRichReal& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfMul (const DBSRichReal& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
RealFieldElOperand::SelfDiv (const DBSRichReal& value)
{
  DBSReal currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

/////////////////////////RichRealFieldElOperand////////////////////////////////

bool
RichRealFieldElOperand::IsNull () const
{
  DBSRichReal currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
RichRealFieldElOperand::GetValue (DBSReal& outValue) const
{
  DBSRichReal currValue;
  Get (currValue);
}

void
RichRealFieldElOperand::GetValue (DBSRichReal& outValue) const
{
  Get (outValue);
}

void
RichRealFieldElOperand::SetValue (const DBSRichReal& value)
{
  Set (value);
}

void
RichRealFieldElOperand::SelfAdd (const DBSInt64& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfAdd (const DBSRichReal& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_add (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfSub (const DBSInt64& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfSub (const DBSRichReal& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_sub (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfMul (const DBSInt64& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfMul (const DBSRichReal& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_mul (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfDiv (const DBSInt64& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

void
RichRealFieldElOperand::SelfDiv (const DBSRichReal& value)
{
  DBSRichReal currValue;
  Get (currValue);

  currValue = internal_div (currValue, value);

  Set (currValue);
}

/////////////////////////TextFieldElOperand////////////////////////////////

bool
TextFieldElOperand::IsNull () const
{
  DBSText currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
TextFieldElOperand::GetValue (DBSText& outValue) const
{
  Get (outValue);
}

void
TextFieldElOperand::SetValue (const DBSText& value)
{
  Set (value);
}

void
TextFieldElOperand::SelfAdd (const DBSChar& value)
{
  DBSText currValue;
  Get (currValue);

  currValue.Append (value);

  Set (currValue);
}

void
TextFieldElOperand::SelfAdd (const DBSText& value)
{
  DBSText currValue;
  Get (currValue);

  currValue.Append (value);

  Set (currValue);
}

StackValue
TextFieldElOperand::GetValueAt (const D_UINT64 index)
{
  return StackValue (CharTextFieldElOperand (m_pRefTable,
                                             m_Row,
                                             m_Field,
                                             index));
}

///////////////////////////ArrayFieldElOperand//////////////////////////////////

bool
ArrayFieldElOperand::IsNull () const
{
  DBSArray currValue;
  Get (currValue);

  return currValue.IsNull ();
}

void
ArrayFieldElOperand::GetValue (DBSArray& outValue) const
{
  Get (outValue);
}

void
ArrayFieldElOperand::SetValue (const DBSArray& value)
{
  Set (value);
}

StackValue
ArrayFieldElOperand::GetValueAt (const D_UINT64 index)
{
  I_DBSTable&        table = m_pRefTable->GetTable ();
  DBSFieldDescriptor fd    = table.GetFieldDescriptor (m_Field);

  assert (fd.isArray);

  switch (fd.m_FieldType)
  {
  case T_BOOL:
    return StackValue (BoolArrayFieldElOperand (m_pRefTable,
                                                m_Row,
                                                m_Field,
                                                index));
  case T_CHAR:
    return StackValue (CharArrayFieldElOperand (m_pRefTable,
                                                m_Row,
                                                m_Field,
                                                index));
  case T_DATE:
    return StackValue (DateArrayFieldElOperand (m_pRefTable,
                                                m_Row,
                                                m_Field,
                                                index));
  case T_DATETIME:
    return StackValue (DateTimeArrayFieldElOperand (m_pRefTable,
                                                    m_Row,
                                                    m_Field,
                                                    index));
  case T_HIRESTIME:
    return StackValue (HiresTimeArrayFieldElOperand (m_pRefTable,
                                                     m_Row,
                                                     m_Field,
                                                     index));
  case T_UINT8:
    return StackValue (UInt8ArrayFieldElOperand (m_pRefTable,
                                                 m_Row,
                                                 m_Field,
                                                 index));
  case T_UINT16:
    return StackValue (UInt16ArrayFieldElOperand (m_pRefTable,
                                                  m_Row,
                                                  m_Field,
                                                  index));
  case T_UINT32:
    return StackValue (UInt32ArrayFieldElOperand (m_pRefTable,
                                                  m_Row,
                                                  m_Field,
                                                  index));
  case T_UINT64:
    return StackValue (UInt64ArrayFieldElOperand (m_pRefTable,
                                                  m_Row,
                                                  m_Field,
                                                  index));
  case T_INT8:
    return StackValue (Int8ArrayFieldElOperand (m_pRefTable,
                                                m_Row,
                                                m_Field,
                                                index));
  case T_INT16:
    return StackValue (Int16ArrayFieldElOperand (m_pRefTable,
                                                 m_Row,
                                                 m_Field,
                                                 index));
  case T_INT32:
    return StackValue (Int32ArrayFieldElOperand (m_pRefTable,
                                                 m_Row,
                                                 m_Field,
                                                 index));
  case T_INT64:
    return StackValue (Int64ArrayFieldElOperand (m_pRefTable,
                                                 m_Row,
                                                 m_Field,
                                                 index));
  case T_REAL:
    return StackValue (RealArrayFieldElOperand (m_pRefTable,
                                                m_Row,
                                                m_Field,
                                                index));
  case T_RICHREAL:
    return StackValue (RichRealArrayFieldElOperand (m_pRefTable,
                                                    m_Row,
                                                    m_Field,
                                                    index));
  default:
    assert (false);
  }

  throw InterException (NULL, _EXTRA (InterException::INTERNAL_ERROR));
}