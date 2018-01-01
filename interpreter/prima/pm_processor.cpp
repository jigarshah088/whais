/******************************************************************************
 PASTRA - A light database one file system and more.
 Copyright(C) 2008  Iulian Popa

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

#include <assert.h>
#include <sstream>

#include "compiler/wopcodes.h"
#include "utils/endianness.h"
#include "pm_processor.h"
#include "pm_operand_undefined.h"


using namespace std;

namespace whais {
namespace prima {


static void
op_func_ldnull(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  assert(data[0] != 0);

  SessionStack& stack = call.GetStack();
  for (uint8_t i = 0; i < *data; ++i)
    stack.Push();

  offset += sizeof(uint8_t);
}

static void
op_func_ldc(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  DChar value(load_le_int32(data));

  call.GetStack().Push(value);

  offset += sizeof(uint32_t);
}

static void
op_func_ldi8(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  DUInt8 value( *data);

  call.GetStack().Push(value);

  offset += sizeof(uint8_t);
}

static void
op_func_ldi16(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  DUInt16 value(load_le_int16(data));

  call.GetStack().Push(value);

  offset += sizeof(uint16_t);
}

static void
op_func_ldi32(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  DUInt32 value(load_le_int32(data));

  call.GetStack().Push(value);

  offset += sizeof(uint32_t);
}

static void
op_func_ldi64(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  DUInt64 value(load_le_int64(data));

  call.GetStack().Push(value);

  offset += sizeof(uint64_t);
}


static void
op_func_ldd(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint8_t day = data[0];
  const uint8_t month = data[1];
  const int16_t year = load_le_int16(data + 2);
  DDate value(year, month, day);

  call.GetStack().Push(value);

  offset += 2 + sizeof(uint16_t);
}


static void
op_func_lddt(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint8_t sec = data[0];
  const uint8_t min = data[1];
  const uint8_t hour = data[2];
  const uint8_t day = data[3];
  const uint8_t month = data[4];
  const int16_t year = load_le_int16(data + 5);
  DDateTime value(year, month, day, hour, min, sec);

  call.GetStack().Push(value);

  offset += 5 + sizeof(uint16_t);
}


static void
op_func_ldht(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint32_t usec = load_le_int32(data);
  const uint8_t sec = data[4];
  const uint8_t min = data[5];
  const uint8_t hour = data[6];
  const uint8_t day = data[7];
  const uint8_t month = data[8];
  const int16_t year = load_le_int16(data + 9);
  DHiresTime value(year, month, day, hour, min, sec, usec);

  call.GetStack().Push(value);

  offset += sizeof(uint32_t) + 5 + sizeof(uint16_t);
}


static void
op_func_ldrr(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const int64_t intPart = load_le_int64(data);
  const int64_t fracPart = load_le_int64(data + sizeof(uint64_t));

  const RICHREAL_T basicValue(intPart, fracPart, W_LDRR_PRECISSION);
  DRichReal value(basicValue);

  call.GetStack().Push(value);

  offset += sizeof(uint64_t) + sizeof(uint64_t);
}


static void
op_func_ldt(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint32_t textOff = load_le_int32(data);
  const uint8_t* text = call.GetUnit().GetConstData(textOff);

  assert(text != nullptr);

  DText value(text);

  call.GetStack().Push(value);

  offset += sizeof(uint32_t);
}


static void
op_func_ldbt(ProcedureCall& call, int64_t& offset)
{
  call.GetStack().Push(DBool(true));
}


static void
op_func_ldbf(ProcedureCall& call, int64_t& offset)
{
  call.GetStack().Push(DBool(false));
}


static void
op_func_ldlo8(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint8_t localIndex = *data;

  LocalOperand localOp(call.GetStack(), call.StackBegin() + localIndex);

  call.GetStack().Push(StackValue(localOp));

  offset += sizeof(uint8_t);
}


static void
op_func_ldlo16(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint16_t localIndex = load_le_int16(data);

  assert(localIndex < call.LocalsCount());

  LocalOperand localOp(call.GetStack(), call.StackBegin() + localIndex);

  call.GetStack().Push(StackValue(localOp));

  offset += sizeof(uint16_t);
}


static void
op_func_ldlo32(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint32_t localIndex = load_le_int32(data);

  LocalOperand localOp(call.GetStack(), call.StackBegin() + localIndex);

  call.GetStack().Push(StackValue(localOp));

  offset += sizeof(uint32_t);
}


static void
op_func_ldgb8(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  const uint8_t globalIndex = *data;
  const uint32_t glbId = call.GetUnit().GetGlobalId(globalIndex);
  StackValue glbValue = call.GetSession().GetGlobalValue(glbId);

  call.GetStack().Push(move(glbValue));

  offset += sizeof(uint8_t);
}


static void
op_func_ldgb16(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  const uint16_t globalIndex = load_le_int16(data);
  const uint32_t glbId = call.GetUnit().GetGlobalId(globalIndex);
  StackValue glbValue = call.GetSession().GetGlobalValue(glbId);

  call.GetStack().Push(move(glbValue));

  offset += sizeof(uint16_t);
}


static void
op_func_ldgb32(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  const uint32_t globalIndex = load_le_int32(data);
  const uint32_t glbId = call.GetUnit().GetGlobalId(globalIndex);
  StackValue glbValue = call.GetSession().GetGlobalValue(glbId);

  call.GetStack().Push(move(glbValue));

  offset += sizeof(uint32_t);
}


static void
op_func_cts(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= call.GetStack().Size());

  call.GetStack().Pop(*data);
  offset += sizeof (uint8_t);
}


template <class T> static void
op_func_stXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& src = stack[stackSize - 1].Operand();
  IOperand& dest = stack[stackSize - 2].Operand();

  T value;

  src.GetValue(value);
  dest.SetValue(value);

  stack.Pop(1);
}


static void
op_func_stta(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  BaseOperand& src = _SC(BaseOperand&, stack[stackSize - 1].Operand());
  BaseOperand& dest = _SC(BaseOperand&, stack[stackSize - 2].Operand());

  if (src.GetType() == T_UNKNOWN)
  {
    assert(src.IsNull());
    assert(IS_TABLE(dest.GetType()));

    TableReference& ref = dest.GetTableReference();
    dest.CopyTableOp(TableOperand(ref.GetTable().Spawn(), true));
  }
  else
    dest.CopyTableOp(src.GetTableOp());

  stack.Pop(1);
}


static void
op_func_stf(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  BaseOperand& src = _SC(BaseOperand&, stack[stackSize - 1].Operand());
  BaseOperand& dest = _SC(BaseOperand&, stack[stackSize - 2].Operand());

  if (src.GetType() == T_UNKNOWN)
  {
    assert(src.IsNull());
    assert(IS_FIELD(dest.GetType()));

    dest.CopyFieldOp(FieldOperand(GET_FIELD_TYPE(dest.GetType())));
  }
  else
    dest.CopyFieldOp(src.GetFieldOp());

  stack.Pop(1);
}


static void
op_func_stud(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  auto& op = _SC(BaseOperand&, stack[stackSize-2].Operand());
  StackValue source = stack[stackSize-1].Operand().Clone();
  op.RedifineValue(source);

  stack.Pop(1);
}


static void
op_func_inull(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& source = stack[stackSize - 1].Operand();
  DBool result(source.IsNull());

  stack.Pop(1);
  stack.Push(result);
}


static void
op_func_nnull(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& source = stack[stackSize - 1].Operand();
  DBool result( !source.IsNull());

  stack.Pop(1);
  stack.Push(result);
}


static void
op_func_call(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

  const uint32_t procIndex = load_le_int32(data);

  offset += sizeof(uint32_t);

  Session& session = call.GetSession();
  SessionStack& stack = call.GetStack();
  const uint32_t procId = call.GetUnit().GetProcedureId(procIndex);

  ProcedureCall(session, stack, session.GetProcedure(procId));
}


static void
op_func_ret(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount()) <= stackSize);

  StackValue result = stack[stackSize - 1].Operand().Clone();

  stack.Pop(stackSize - call.StackBegin());

  if (result.Operand().GetType() == T_UNKNOWN)
  {
    assert(result.Operand().IsNull());
    stack.Push(StackValue(call.GetLocalDefault(0)));
  }
  else
    stack.Push(move(result));

  offset = call.CodeSize(); //Signal the procedure return
}


template <class DBS_T> static void
op_func_addXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  const DBS_T result(firstOp.mValue + secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


static void
op_func_addt(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DText firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);

  DText secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);

  DText result;
  if (firstOp.IsNull())
    result = secondOp;

  else if (secondOp.IsNull())
    result = firstOp;

  else
  {
    result = firstOp;
    result.Append(secondOp);
  }

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_andXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T result(firstOp.mValue & secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_divXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  if (secondOp == DBS_T(0))
    throw InterException(_EXTRA(InterException::DIVIDE_BY_ZERO));

  DBS_T result(firstOp.mValue / secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_eqXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);

  DBool result(firstOp == secondOp);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_geXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBool result((firstOp < secondOp) == false);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_gtXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBool result(((firstOp < secondOp) || (firstOp == secondOp)) == false);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_leXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBool result((firstOp < secondOp) || (firstOp == secondOp));

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_ltXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBool());
    return;
  }

  DBool result(firstOp < secondOp);

  stack.Pop(2);
  stack.Push(result);
}


static void
op_func_mod(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DInt64 firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DInt64());
    return;
  }

  DInt64 secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DInt64());
    return;
  }

  if (secondOp == DInt64(0))
    throw InterException(_EXTRA(InterException::DIVIDE_BY_ZERO));

  DInt64 result(firstOp.mValue % secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


static void
op_func_modu(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DUInt64 firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DUInt64());
    return;
  }

  DUInt64 secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DUInt64());
    return;
  };

  if (secondOp == DUInt64(0))
    throw InterException(_EXTRA(InterException::DIVIDE_BY_ZERO));

  DUInt64 result(firstOp.mValue % secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}



template <class DBS_T> static void
op_func_mulXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T result(firstOp.mValue * secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_neXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);

  DBool result((firstOp == secondOp) == false);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_notXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBS_T operand;
  stack[stackSize - 1].Operand().GetValue(operand);

  DBS_T result;
  if (operand.IsNull() == false)
    result = DBS_T( ~operand.mValue);

  stack.Pop(1);
  stack.Push(result);
}


template <> void
op_func_notXX<DBool> (ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBool operand;
  stack[stackSize - 1].Operand().GetValue(operand);

  DBool result;
  if (operand.IsNull() == false)
    result = DBool( !operand.mValue);

  stack.Pop(1);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_orXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T result(firstOp.mValue | secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_subXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  const DBS_T result(firstOp.mValue - secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


template <class DBS_T> static void
op_func_xorXX(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  DBS_T firstOp;
  stack[stackSize - 2].Operand().GetValue(firstOp);
  if (firstOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T secondOp;
  stack[stackSize - 1].Operand().GetValue(secondOp);
  if (secondOp.IsNull())
  {
    stack.Pop(2);
    stack.Push(DBS_T());
    return;
  }

  DBS_T result(firstOp.mValue ^ secondOp.mValue);

  stack.Pop(2);
  stack.Push(result);
}


static void
op_func_jf(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBool firstOp;
  stack[stackSize - 1].Operand().GetValue(firstOp);

  if ((firstOp.IsNull() == false) && (firstOp.mValue == false))
  {
    const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

    const int32_t jmpOffset = _SC(int32_t, load_le_int32(data));

    offset = jmpOffset;
  }
  else
    offset += sizeof(uint32_t);
}


static void
op_func_jfc(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBool firstOp;
  stack[stackSize - 1].Operand().GetValue(firstOp);
  stack.Pop(1);

  if ((firstOp.IsNull() == false) && (firstOp.mValue == false))
  {
    const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

    const int32_t jmpOffset = _SC(int32_t, load_le_int32(data));
    offset = jmpOffset;
  }
  else
    offset += sizeof(uint32_t);
}


static void
op_func_jt(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBool firstOp;
  stack[stackSize - 1].Operand().GetValue(firstOp);

  if ((firstOp.IsNull() == false) && firstOp.mValue)
  {
    const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

    const int32_t jmpOffset = _SC(int32_t, load_le_int32(data));
    offset = jmpOffset;
  }
  else
    offset += sizeof(uint32_t);
}


static void
op_func_jtc(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DBool firstOp;
  stack[stackSize - 1].Operand().GetValue(firstOp);
  stack.Pop(1);

  if ((firstOp.IsNull() == false) && firstOp.mValue)
  {
    const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;

    const int32_t jmpOffset = _SC(int32_t, load_le_int32(data));
    offset = jmpOffset;
  }
  else
    offset += sizeof(uint32_t);
}


static void
op_func_jmp(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const int32_t jmpOffset = _SC(int32_t, load_le_int32(data));

  offset = jmpOffset;
}


template <uint_t EXCEPTION_CODE> void
op_func_ind(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DUInt64 index;
  stack[stackSize - 1].Operand().GetValue(index);

  if (index.IsNull())
    throw InterException(_EXTRA(EXCEPTION_CODE));

  StackValue result = stack[stackSize - 2].Operand().GetValueAt(index.mValue);

  stack.Pop(2);
  stack.Push(move(result));
}


static void
op_func_indta(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  DUInt64 index;
  stack[stackSize - 1].Operand().GetValue(index);

  if (index.IsNull())
    throw InterException(_EXTRA(InterException::ROW_INDEX_NULL));

  BaseOperand& op = _SC(BaseOperand&, stack[stackSize - 2].Operand());

  const uint8_t* const pData = call.Code() + call.CurrentOffset() + offset;
  const uint32_t textOff = load_le_int32(pData);
  const uint8_t* const text = call.GetUnit().GetConstData(textOff);

  offset += sizeof(uint32_t);

  FIELD_INDEX field = op.GetTable().RetrieveField(_RC(const char*, text));
  FieldOperand fieldOp(op.GetTableReference(), field);
  StackValue result = fieldOp.GetValueAt(index.mValue);

  stack.Pop(2);
  stack.Push(move(result));
}


static void
op_func_self(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  BaseOperand& op = _SC(BaseOperand&, stack[stackSize - 1].Operand());

  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint32_t textOff = load_le_int32(data);
  const uint8_t* const text = call.GetUnit().GetConstData(textOff);

  offset += sizeof(uint32_t);

  FIELD_INDEX field = op.GetTable().RetrieveField(_RC(const char*, text));

  FieldOperand fieldOp(op.GetTableReference(), field);
  StackValue result(fieldOp);

  stack.Pop(1);
  stack.Push(move(result));
}


static void
op_func_bsync(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint8_t syncStmt = *data;

  offset += sizeof(uint8_t);

  call.AquireSync(syncStmt);
}


static void
op_func_esync(ProcedureCall& call, int64_t& offset)
{
  const uint8_t* const data = call.Code() + call.CurrentOffset() + offset;
  const uint8_t syncStmt = *data;

  offset += sizeof(uint8_t);

  call.ReleaseSync(syncStmt);
}


template <class DBS_T> static void
op_func_sadd(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfAdd(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_ssub(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfSub(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_smul(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfMul(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_sdiv(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfDiv(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_smod(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfMod(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_sand(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfAnd(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_sxor(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfXor(delta);

  stack.Pop(1);
}


template <class DBS_T> static void
op_func_sor(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 2) <= stackSize);

  IOperand& destOp = stack[stackSize - 2].Operand();

  DBS_T delta;
  stack[stackSize - 1].Operand().GetValue(delta);

  destOp.SelfOr(delta);

  stack.Pop(1);
}


template<bool reverse> static void
op_start_iterate(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& container = stack[stackSize - 1].Operand();
  if (container.IsNull())
  {
    stack.Push(DBool(false));
    return;
  }

  StackValue it{NullOperand{}};;
  const bool started = container.StartIterate(reverse, it);

  stack.Pop(1);
  stack.Push(move(it));
  stack.Push(DBool(started));
}


template<bool reverse> static void
op_iterate(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& iteratorOp = stack[stackSize - 1].Operand();

  const bool started = iteratorOp.Iterate(reverse);
  stack.Push(DBool(started));
}

static void
op_iterator_offset(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& iteratorOp = stack[stackSize - 1].Operand();

  const uint64_t itOffset = iteratorOp.IteratorOffset();

  stack.Pop(1);
  stack.Push(DUInt64(itOffset));
}


static void
op_field_id(ProcedureCall& call, int64_t& offset)
{
  SessionStack& stack = call.GetStack();
  const size_t stackSize = stack.Size();

  assert((call.StackBegin() + call.LocalsCount() - 1 + 1) <= stackSize);

  IOperand& field = stack[stackSize - 1].Operand();
  if (field.IsNullExpression() || field.IsNull())
  {
    call.GetSession().LogMessage("Cannot get a field index due to NULL field value, "
                                 "returning NULL also!");

    stack.Pop(1);
    stack.Push(DUInt64());
    return ;
  }

  DUInt64 result(field.GetField());
  stack.Pop(1);
  stack.Push(result);
}


typedef void(*OP_FUNC) (ProcedureCall& call, int64_t& ioOffset);



static OP_FUNC operations[] = {
                                nullptr,
                                op_func_ldnull,
                                op_func_ldc,
                                op_func_ldi8,
                                op_func_ldi16,
                                op_func_ldi32,
                                op_func_ldi64,
                                op_func_ldd,
                                op_func_lddt,
                                op_func_ldht,
                                op_func_ldrr,
                                op_func_ldt,
                                op_func_ldbt,
                                op_func_ldbf,
                                op_func_ldlo8,
                                op_func_ldlo16,
                                op_func_ldlo32,
                                op_func_ldgb8,
                                op_func_ldgb16,
                                op_func_ldgb32,
                                op_func_cts,

                                op_func_stXX<DBool>,
                                op_func_stXX<DChar>,
                                op_func_stXX<DDate>,
                                op_func_stXX<DDateTime>,
                                op_func_stXX<DHiresTime>,
                                op_func_stXX<DInt8>,
                                op_func_stXX<DInt16>,
                                op_func_stXX<DInt32>,
                                op_func_stXX<DInt64>,
                                op_func_stXX<DReal>,
                                op_func_stXX<DRichReal>,
                                op_func_stXX<DText>,
                                op_func_stXX<DUInt8>,
                                op_func_stXX<DUInt16>,
                                op_func_stXX<DUInt32>,
                                op_func_stXX<DUInt64>,
                                op_func_stta,
                                op_func_stf,
                                op_func_stXX<DArray>,
                                op_func_stud,

                                op_func_inull,
                                op_func_nnull,

                                op_func_call,
                                op_func_ret,

                                op_func_addXX<DInt64>,
                                op_func_addXX<DRichReal>,
                                op_func_addt,

                                op_func_andXX<DInt64>,
                                op_func_andXX<DBool>,

                                op_func_divXX<DInt64>,
                                op_func_divXX<DUInt64>,
                                op_func_divXX<DRichReal>,

                                op_func_eqXX<DInt64>,
                                op_func_eqXX<DBool>,
                                op_func_eqXX<DChar>,
                                op_func_eqXX<DDate>,
                                op_func_eqXX<DDateTime>,
                                op_func_eqXX<DHiresTime>,
                                op_func_eqXX<DRichReal>,
                                op_func_eqXX<DText>,

                                op_func_geXX<DInt64>,
                                op_func_geXX<DUInt64>,
                                op_func_geXX<DChar>,
                                op_func_geXX<DDate>,
                                op_func_geXX<DDateTime>,
                                op_func_geXX<DHiresTime>,
                                op_func_geXX<DRichReal>,

                                op_func_gtXX<DInt64>,
                                op_func_gtXX<DUInt64>,
                                op_func_gtXX<DChar>,
                                op_func_gtXX<DDate>,
                                op_func_gtXX<DDateTime>,
                                op_func_gtXX<DHiresTime>,
                                op_func_gtXX<DRichReal>,

                                op_func_leXX<DInt64>,
                                op_func_leXX<DUInt64>,
                                op_func_leXX<DChar>,
                                op_func_leXX<DDate>,
                                op_func_leXX<DDateTime>,
                                op_func_leXX<DHiresTime>,
                                op_func_leXX<DRichReal>,

                                op_func_ltXX<DInt64>,
                                op_func_ltXX<DUInt64>,
                                op_func_ltXX<DChar>,
                                op_func_ltXX<DDate>,
                                op_func_ltXX<DDateTime>,
                                op_func_ltXX<DHiresTime>,
                                op_func_ltXX<DRichReal>,

                                op_func_mod,
                                op_func_modu,

                                op_func_mulXX<DInt64>,
                                op_func_mulXX<DUInt64>,
                                op_func_mulXX<DRichReal>,

                                op_func_neXX<DInt64>,
                                op_func_neXX<DBool>,
                                op_func_neXX<DChar>,
                                op_func_neXX<DDate>,
                                op_func_neXX<DDateTime>,
                                op_func_neXX<DHiresTime>,
                                op_func_neXX<DRichReal>,
                                op_func_neXX<DText>,

                                op_func_notXX<DInt64>,
                                op_func_notXX<DBool>,

                                op_func_orXX<DInt64>,
                                op_func_orXX<DBool>,

                                op_func_subXX<DInt64>,
                                op_func_subXX<DRichReal>,

                                op_func_xorXX<DInt64>,
                                op_func_xorXX<DBool>,

                                op_func_jf,
                                op_func_jfc,
                                op_func_jt,
                                op_func_jtc,
                                op_func_jmp,

                                op_func_ind<InterException::TEXT_INDEX_NULL>,
                                op_func_ind<InterException::ARRAY_INDEX_NULL>,
                                op_func_ind<InterException::ROW_INDEX_NULL>,
                                op_func_indta,
                                op_func_self,

                                op_func_bsync,
                                op_func_esync,

                                op_func_sadd<DInt64>,
                                op_func_sadd<DRichReal>,
                                op_func_sadd<DChar>,
                                op_func_sadd<DText>,

                                op_func_ssub<DInt64>,
                                op_func_ssub<DRichReal>,

                                op_func_smul<DInt64>,
                                op_func_smul<DUInt64>,
                                op_func_smul<DRichReal>,

                                op_func_sdiv<DInt64>,
                                op_func_sdiv<DUInt64>,
                                op_func_sdiv<DRichReal>,

                                op_func_smod<DInt64>,
                                op_func_smod<DUInt64>,

                                op_func_sand<DInt64>,
                                op_func_sand<DBool>,

                                op_func_sxor<DInt64>,
                                op_func_sxor<DBool>,

                                op_func_sor<DInt64>,
                                op_func_sor<DBool>,

                                op_start_iterate<false>,
                                op_start_iterate<true>,
                                op_iterate<false>,
                                op_iterate<true>,

                                op_iterator_offset,
                                op_field_id
};



ProcedureCall::ProcedureCall(Session& session, SessionStack& stack, const Procedure& procedure)
  : mProcedure(procedure),
    mSession(session),
    mStack(stack),
    mCode(mProcedure.mNativeCode
           ? nullptr
           : procedure.mProcMgr->Code(procedure, nullptr)),
    mStackBegin(stack.Size() - procedure.mArgsCount),
    mCodePos(0),
    mAquiredSync(NO_INDEX)
{
  if (mProcedure.mNativeCode != nullptr)
  {
    const WLIB_STATUS status = procedure.mNativeCode(stack, session);
    if (status != WOP_OK)
    {
      throw InterException(_EXTRA(InterException::NATIVE_CALL_FAILED),
                           "Native procedure '%s' execution returned unexpected error code '%d'.",
                           mProcedure.mProcMgr->Name(mProcedure.mId),
                           status);
    }

    if (stack.Size() != mStackBegin + 1)
    {
      std::ostringstream log;
      log << "Native procedure called '"
          << _RC(const char*, mProcedure.mProcMgr->Name(mProcedure.mId)) << "'.";

      throw InterException(_EXTRA(InterException::STACK_CORRUPTED),
                           "Stack corruption detected after native procedure '%s' execution.",
                           mProcedure.mProcMgr->Name(mProcedure.mId));
    }
    return;
  }
  else
  {
    if (stack.Size() + LocalsCount() > session.MaxStackCount())
    {
      throw InterException(_EXTRA(InterException::STACK_TOO_BIG),
                           "Failed to call procedure '%s' as it will exceed the maximum stack "
                           "count of %u elements limit.",
                           mProcedure.mProcMgr->Name(mProcedure.mId),
                           session.MaxStackCount());
    }

    //Fill the stack with default values for the local values(don't
    //include the arguments as they should be on the stack already and the
    uint32_t local = mProcedure.mArgsCount + 1;
    if (local < LocalsCount())
    {
      const StackValue *localValue = &GetLocalDefault(local);
      while (local < LocalsCount())
      {
        stack.Push(StackValue(*localValue));
        ++local, ++localValue;
      }
    }

    //Count only procedure's parameters and local values,
    //but not the result value too.
    if (LocalsCount() - 1 > stack.Size())
    {
      throw InterException(_EXTRA(InterException::STACK_CORRUPTED),
                           "Stack corruption detected after procedure '%s' execution.",
                           mProcedure.mProcMgr->Name(mProcedure.mId));
    }

    try
    {
      Run();
    }
    catch (Exception& e)
    {
      const std::string message = e.Message();

      if ( !message.empty())
      {
        e.Message("%s\n\tCalled from '%s' (PC: %04u).",
                  message.c_str(),
                  mProcedure.mProcMgr->Name(mProcedure.mId),
                  mCodePos);
      }
      else
      {
        e.Message("Current procedure '%s'(PC: %04u).",
                  mProcedure.mProcMgr->Name(mProcedure.mId),
                  mCodePos);
      }
      throw;
    }
  }

  assert(mAquiredSync == NO_INDEX);
}

void
ProcedureCall::Run()
{
  W_OPCODE opcode;

  try
  {
    while (mCodePos < CodeSize())
    {
      if (mSession.IsServerShoutdowing())
        throw InterException(_EXTRA(InterException::SERVER_STOPPED));

      int64_t offset = wh_compiler_decode_op(mCode + mCodePos, &opcode);

      assert(opcode < _SC(int, (sizeof operations / sizeof operations[0])));
      assert(opcode != 0);
      assert((offset > 0) && (offset < 3));

      operations[opcode]( *this, offset);

      mCodePos += offset;

      assert((mCodePos <= CodeSize()) || (_SC(uint64_t, offset) == CodeSize()));
    }

    if (mAquiredSync != NO_INDEX)
      ReleaseSync(mAquiredSync);
  }
  catch (...)
  {
    if (mAquiredSync != NO_INDEX)
      ReleaseSync(mAquiredSync);

    throw;
  }

  //After a procedure execution, only the return value should be present
  //on the stack
  assert(mStack.Size() == (mStackBegin + 1));
}

void
ProcedureCall::AquireSync(const uint8_t sync)
{
  if (mAquiredSync != NO_INDEX)
    throw InterException(_EXTRA(InterException::NEESTED_SYNC_REQ));

  mProcedure.mProcMgr->AquireSync(mProcedure, sync);
  mAquiredSync = sync;
}


void
ProcedureCall::ReleaseSync(const uint8_t sync)
{
  if (mAquiredSync != sync)
    throw InterException(_EXTRA(InterException::SYNC_NOT_AQUIRED));

  mProcedure.mProcMgr->ReleaseSync(mProcedure, sync);
  mAquiredSync = NO_INDEX;
}

} //namespace prima
} //namespace whais
