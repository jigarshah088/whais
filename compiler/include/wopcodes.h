/******************************************************************************
WHAIS - An advanced database system
Copyright(C) 2014-2018  Iulian Popa

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


#ifndef WOPCODES_H
#define WOPCODES_H

#include "whais.h"
#include "whaisc.h"


enum W_OPCODE
{
  /* one byte opcodes */
  W_NA = 0,     /* invalid opcode */

  W_LDNULL,
  W_LDC,
  W_LDI8,
  W_LDI16,
  W_LDI32,
  W_LDI64,
  W_LDD,
  W_LDDT,
  W_LDHT,
  W_LDRR,
  W_LDT,
  W_LDBT,
  W_LDBF,
  W_LDLO8,
  W_LDLO16,
  W_LDLO32,
  W_LDGB8,
  W_LDGB16,
  W_LDGB32,

  W_CTS,

  W_STB,
  W_STC,
  W_STD,
  W_STDT,
  W_STHT,
  W_STI8,
  W_STI16,
  W_STI32,
  W_STI64,
  W_STR,
  W_STRR,
  W_STT,
  W_STUI8,
  W_STUI16,
  W_STUI32,
  W_STUI64,
  W_STTA,
  W_STF,
  W_STA,
  W_STUD,

  W_INULL,
  W_NNULL,

  W_CALL,
  W_RET,

  W_ADD,
  W_ADDRR,
  W_ADDT,

  W_AND,
  W_ANDB,

  W_DIV,
  W_DIVU,
  W_DIVRR,

  W_EQ,
  W_EQB,
  W_EQC,
  W_EQD,
  W_EQDT,
  W_EQHT,
  W_EQRR,
  W_EQT,

  W_GE,
  W_GEU,
  W_GEC,
  W_GED,
  W_GEDT,
  W_GEHT,
  W_GERR,

  W_GT,
  W_GTU,
  W_GTC,
  W_GTD,
  W_GTDT,
  W_GTHT,
  W_GTRR,

  W_LE,
  W_LEU,
  W_LEC,
  W_LED,
  W_LEDT,
  W_LEHT,
  W_LERR,

  W_LT,
  W_LTU,
  W_LTC,
  W_LTD,
  W_LTDT,
  W_LTHT,
  W_LTRR,

  W_MOD,
  W_MODU,

  W_MUL,
  W_MULU,
  W_MULRR,

  W_NE,
  W_NEB,
  W_NEC,
  W_NED,
  W_NEDT,
  W_NEHT,
  W_NERR,
  W_NET,

  W_NOT,
  W_NOTB,

  W_OR,
  W_ORB,

  W_SUB,
  W_SUBRR,

  W_XOR,
  W_XORB,

  W_JF,
  W_JFC,
  W_JT,
  W_JTC,
  W_JMP,

  W_INDT,
  W_INDA,
  W_INDF,
  W_INDTA,
  W_SELF,

  W_BSYNC,
  W_ESYNC,

  W_SADD,
  W_SADDRR,
  W_SADDC,
  W_SADDT,

  W_SSUB,
  W_SSUBRR,

  W_SMUL,
  W_SMULU,
  W_SMULRR,

  W_SDIV,
  W_SDIVU,
  W_SDIVRR,

  W_SMOD,
  W_SMODU,

  W_SAND,
  W_SANDB,

  W_SXOR,
  W_SXORB,

  W_SOR,
  W_SORB,

  W_ITF,
  W_ITL,
  W_ITN,
  W_ITP,
  W_ITOFF,
  W_FID,

  W_CARR,

  W_AJOIN,
  W_AFOUT,
  W_AFIN,

  W_OP_END_MARK
};

static const int64_t W_LDRR_PRECISSION = 1000000000000000000ll;

static const uint_t CARR_FROM_FIELD = 0x80;

static const uint_t A_SELF_MASK = 0x40;
static const uint_t A_OPB_A_MASK = 0x80;


#ifdef __cplusplus
extern "C"
{
#endif


static INLINE uint_t
wh_compiler_decode_op(const uint8_t         *instrs,
                      enum W_OPCODE* const   outOpcode)
{
  return (*outOpcode = (enum W_OPCODE)*instrs, 1);
}

static INLINE uint_t
wh_compiler_encode_op(uint8_t* const        instrs,
                      const enum W_OPCODE   opcode)
{
  return (*instrs = opcode, 1);
}

#ifdef __cplusplus
}
#endif

#endif  /* WOPCODES_H */
