/******************************************************************************
 WHISPERC - A compiler for whisper programs
 Copyright (C) 2009  Iulian Popa

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
/*
 * brlo_stmts.c - Implements the semantics for whisper programs statements.
 */

#include "brlo_stmts.h"
#include "expression.h"
#include "statement.h"
#include "opcodes.h"
#include "wlog.h"

void
begin_if_stmt (struct ParserState* const pState,
               YYSTYPE                   expression,
               enum BRANCH_TYPE          branchType)
{
  struct Statement* const    pStmt        = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream  = stmt_query_instrs (pStmt);
  struct WArray* const       pBranchStack = stmt_query_branch_stack (pStmt);
  struct Branch              branch;

  if ( ! translate_bool_exp (pState, expression))
    {
      /* errors encountered */
      assert (pState->abortError == TRUE);
      return;
    }

  /* reserve space for latter */
  if ((w_opcode_encode (pCodeStream, W_JFC) == NULL) ||
      (wh_ostream_wint32 (pCodeStream, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }

  branch.type     = branchType;
  branch.startPos = wh_ostream_size (pCodeStream) - sizeof (uint32_t) - 1;
  branch.elsePos  = 0;

  if (wh_array_add (pBranchStack, &branch) == NULL)
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }
}

void
begin_else_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt        = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream  = stmt_query_instrs (pStmt);
  struct WArray* const       pBranchStack = stmt_query_branch_stack (pStmt);
  uint_t                     branchId     = wh_array_count (pBranchStack) - 1;
  struct Branch*             pBranchIt    = wh_array_get (pBranchStack, branchId);
  int32_t                    jumpOffest   = 0;

  /* last if or elseif statement needs to know where to exit */
  if ((w_opcode_encode (pCodeStream, W_JMP) == NULL) ||
      (wh_ostream_wint32 (pCodeStream, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }

  pBranchIt->elsePos = wh_ostream_size (pCodeStream) - sizeof (uint32_t) - 1;
  jumpOffest         = wh_ostream_size (pCodeStream) - pBranchIt->startPos;
  memcpy (wh_ostream_data (pCodeStream) + pBranchIt->startPos + 1,
          &jumpOffest,
          sizeof jumpOffest);
}

void
begin_elseif_stmt (struct ParserState *const state, YYSTYPE exp)
{
  begin_else_stmt (state);

  if (!state->abortError)
    {
      begin_if_stmt (state, exp, BT_ELSEIF);
    }
}

void
finalize_if_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt        = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream  = stmt_query_instrs (pStmt);
  struct WArray* const       pBranchStack = stmt_query_branch_stack (pStmt);
  uint_t                     branchId     = wh_array_count (pBranchStack);
  const struct Branch*       pBranchIt    = NULL;

  do
    {
      int32_t jumpOffset = wh_ostream_size (pCodeStream);

      pBranchIt = wh_array_get (pBranchStack, --branchId);

      if (pBranchIt->elsePos == 0)
        {
          /* handle the lack of an else statement */
          assert (pBranchIt->startPos > 0);

          jumpOffset -= pBranchIt->startPos;
          assert (jumpOffset > 0);
          memcpy (wh_ostream_data (pCodeStream) + pBranchIt->startPos + 1, &jumpOffset,
                  sizeof jumpOffset);
        }
      else
        {
          /*handle the skip of the else or elseif statements */
          jumpOffset -= pBranchIt->elsePos;
          assert (jumpOffset > 0);
          memcpy (wh_ostream_data (pCodeStream) + pBranchIt->elsePos + 1,
                  &jumpOffset,
                  sizeof jumpOffset);
        }

    }
  while (pBranchIt->type == BT_ELSEIF);

  /* update the stack */
  wh_array_resize (pBranchStack, branchId);
}

void
begin_while_stmt (struct ParserState* const pState, YYSTYPE exp)
{
  struct Statement* const    pStmt       = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream = stmt_query_instrs (pStmt);
  struct WArray* const       pLoopStack  = stmt_query_loop_stack (pStmt);
  struct Loop                loop;

  loop.type     = LE_WHILE_BEGIN;
  loop.startPos = wh_ostream_size (pCodeStream);

  if ( ! translate_bool_exp (pState, exp))
    {
      /* errors encountered */
      assert (pState->abortError == TRUE);
      return;
    }

  loop.endPos = wh_ostream_size (pCodeStream);
  if ((w_opcode_encode (pCodeStream, W_JFC) == NULL) ||
      (wh_ostream_wint32 (pCodeStream, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }

  if (wh_array_add (pLoopStack, &loop) == NULL)
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }
}

void
finalize_while_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt           = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream     = stmt_query_instrs (pStmt);
  uint8_t* const             pCode           = wh_ostream_data (pCodeStream);
  struct WArray* const       pLoopStack      = stmt_query_loop_stack (pStmt);
  uint_t                     loopId          = wh_array_count (pLoopStack);
  const struct Loop*         pLoopIt         = NULL;
  int32_t                    endWhileLoop    = wh_ostream_size (pCodeStream);
  int32_t                    endWhileStmtPos = 0;
  int32_t                    offset          = 0;

  if ((w_opcode_encode (pCodeStream, W_JMP) == NULL) ||
      (wh_ostream_wint32 (pCodeStream, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }

  endWhileStmtPos = wh_ostream_size (pCodeStream);

  do
    {
      pLoopIt = wh_array_get (pLoopStack, --loopId);
      switch (pLoopIt->type)
        {
        case LE_BREAK:
        case LE_WHILE_BEGIN:
          offset = endWhileStmtPos - pLoopIt->endPos;
          break;

        default:
          /* Continue statements should be already handled in case
           * of while statements */
          assert (0);
        }
      /* make the jump corrections */
      memcpy (pCode + pLoopIt->endPos + 1, &offset, sizeof offset);
    }
  while ((pLoopIt->type == LE_CONTINUE) || (pLoopIt->type == LE_BREAK));

  assert (pLoopIt->type == LE_WHILE_BEGIN);
  /* path the loop jump */
  offset = pLoopIt->startPos - endWhileLoop;
  memcpy (pCode + endWhileLoop + 1, &offset, sizeof offset);

  /* update the loop stack */
  wh_array_resize (pLoopStack, loopId);
}

void
begin_until_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt       = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream = stmt_query_instrs (pStmt);
  struct WArray* const       pLoopStack  = stmt_query_loop_stack (pStmt);
  struct Loop                loop;

  loop.type     = LE_UNTIL_BEGIN;
  loop.startPos = wh_ostream_size (pCodeStream);
  loop.endPos   = 0;

  if (wh_array_add (pLoopStack, &loop) == NULL)
    w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);
}

void
finalize_until_stmt (struct ParserState* const pState, YYSTYPE exp)
{
  struct Statement* const    pStmt           = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream     = stmt_query_instrs (pStmt);
  uint8_t *const             pCode           = wh_ostream_data (pCodeStream);
  struct WArray* const       pLoopStack      = stmt_query_loop_stack (pStmt);
  uint_t                     loopId          = wh_array_count (pLoopStack);
  struct Loop*               pLoopIt         = NULL;
  int32_t                    untilExpPos     = wh_ostream_size (pCodeStream);
  int32_t                    endUntilStmtPos = 0;
  int32_t                    offset          = 0;

  if ( ! translate_bool_exp (pState, exp))
    {
      /* errors encountered */
      assert (pState->abortError == TRUE);
      return;
    }

  if ((w_opcode_encode (pCodeStream, W_JTC) == NULL) ||
      (wh_ostream_wint32 (pCodeStream, 0) == NULL))
    {
      /* errors encountered */
      assert (pState->abortError == TRUE);
      return;
    }
  endUntilStmtPos = wh_ostream_size (pCodeStream);

  do
    {
      pLoopIt = wh_array_get (pLoopStack, --loopId);
      switch (pLoopIt->type)
        {
        case LE_CONTINUE:
          offset = untilExpPos - pLoopIt->endPos;
          break;

        case LE_BREAK:
          offset = endUntilStmtPos - pLoopIt->endPos;
          break;

        case LE_UNTIL_BEGIN:
          /* JCT 0x01020304 should be encoded on 5 bytes */
          pLoopIt->endPos = endUntilStmtPos - 5;
          offset          = pLoopIt->startPos - pLoopIt->endPos;
          break;

        default:
          assert (0);
        }
      memcpy (pCode + pLoopIt->endPos + 1, &offset, sizeof offset);
    }
  while ((pLoopIt->type == LE_CONTINUE) || (pLoopIt->type == LE_BREAK));

  /* update the loop stack */
  wh_array_resize (pLoopStack, loopId);
}

void
handle_break_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt      = pState->pCurrentStmt;
  struct WOutputStream* const pCode      = stmt_query_instrs (pStmt);
  struct WArray* const       pLoopStack = stmt_query_loop_stack (pStmt);
  struct Loop                loop;

  if (wh_array_count (pLoopStack) == 0)
    {
      w_log_msg (pState, pState->bufferPos, MSG_BREAK_NOLOOP);

      return;
    }

  loop.type     = LE_BREAK;
  loop.startPos = 0;
  loop.endPos   = wh_ostream_size (pCode);

  if (wh_array_add (pLoopStack, &loop) == NULL)
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }

  if ((w_opcode_encode (pCode, W_JMP) == NULL) ||
      (wh_ostream_wint32 (pCode, 0) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);
    }
}

void
handle_continue_stmt (struct ParserState* const pState)
{
  struct Statement* const    pStmt       = pState->pCurrentStmt;
  struct WOutputStream* const pCodeStream = stmt_query_instrs (pStmt);
  struct WArray* const       pLoopStack  = stmt_query_loop_stack (pStmt);
  uint_t                     loopId      = wh_array_count (pLoopStack);
  const struct Loop*         pLoopIt     = NULL;
  struct Loop                loop;

  while (loopId != 0)
    {
      pLoopIt = wh_array_get (pLoopStack, --loopId);
      if ((pLoopIt->type != LE_CONTINUE) && (pLoopIt->type != LE_BREAK))
        break;
      else
        pLoopIt = NULL;
    }

  if (pLoopIt == NULL)
    {
      w_log_msg (pState, pState->bufferPos, MSG_CONTINUE_NOLOOP);

      return;
    }

  loop.type   = LE_CONTINUE;
  loop.endPos = wh_ostream_size (pCodeStream);
  switch (pLoopIt->type)
    {
    case LE_UNTIL_BEGIN:
      if ((w_opcode_encode (pCodeStream, W_JMP) == NULL) ||
          (wh_ostream_wint32 (pCodeStream, 0) == NULL))
        {
          w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

          return;
        }

      if (wh_array_add (pLoopStack, &loop) == NULL)
        w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);
      break;

    case LE_WHILE_BEGIN:
      if ((w_opcode_encode (pCodeStream, W_JMP) == NULL) ||
          (wh_ostream_wint32 (pCodeStream, pLoopIt->startPos - loop.endPos) == NULL))
        {
          w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

          return;
        }
      /* this statement won't need jump correction latter */
      break;
    default:
      assert (0);
    }
}

void
begin_sync_stmt (struct ParserState* const pState)
{
  struct Statement *const    pStmt       = pState->pCurrentStmt;
  struct WOutputStream *const pCodeStream = stmt_query_instrs (pStmt);
  const uint_t               stmtsCount  = pStmt->spec.proc.syncTracker / 2;

  if (pStmt->spec.proc.syncTracker & 1)
    {
      w_log_msg (pState, pState->bufferPos, MSG_SYNC_NA);

      return;
    }
  else if (stmtsCount > 255)
    {
      w_log_msg (pState, pState->bufferPos, MSG_SYNC_MANY);

      return;
    }

  if ((w_opcode_encode (pCodeStream, W_BSYNC) == NULL) ||
      (wh_ostream_wint8 (pCodeStream, stmtsCount) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }
  /* update the keeper */
  pStmt->spec.proc.syncTracker++;
}

void
finalize_sync_stmt (struct ParserState* const pState)
{
  struct Statement *const    pStmt       = pState->pCurrentStmt;
  struct WOutputStream *const pCodeStream = stmt_query_instrs (pStmt);
  const uint_t               stmtsCount  = pStmt->spec.proc.syncTracker / 2;

  assert (pStmt->spec.proc.syncTracker & 1);

  if ((w_opcode_encode (pCodeStream, W_ESYNC) == NULL) ||
      (wh_ostream_wint8 (pCodeStream, stmtsCount) == NULL))
    {
      w_log_msg (pState, IGNORE_BUFFER_POS, MSG_NO_MEM);

      return;
    }
  /* update the keeper */
  pStmt->spec.proc.syncTracker++;
}
