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


#ifndef TABLE_STMTS_H_
#define TABLE_STMTS_H_


#include "whais.h"
#include "../parser/parser.h"

/* careful with this to be the same as in
 * whais.y */
#ifndef YYSTYPE
#define YYSTYPE struct SemValue*
#endif



YYSTYPE
translate_row_copy (struct ParserState* const   parser,
                    YYSTYPE                     leftTable,
                    YYSTYPE                     leftIdentifierList,
                    YYSTYPE                     lefttIndex,
                    YYSTYPE                     rightTable,
                    YYSTYPE                     rightIdentifierList,
                    YYSTYPE                     rightIndex);


YYSTYPE
translate_row_copy_free (struct ParserState* const   parser,
                         YYSTYPE                     leftTable,
                         YYSTYPE                     lefttIndex,
                         YYSTYPE                     rightTable,
                         YYSTYPE                     rightIndex);

#endif /* WHISPER_COMPILER_SEMANTICS_TABLE_STMTS_H_ */
