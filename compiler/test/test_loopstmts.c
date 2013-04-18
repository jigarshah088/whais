
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"
#include "../semantics/vardecl.h"
#include "../semantics/opcodes.h"
#include "../semantics/procdecl.h"

#include "custom/include/test/test_fmw.h"

extern int yyparse (struct ParserState *);

static void
init_state_for_test (struct ParserState *state, const char * buffer)
{
  state->buffer = buffer;
  state->strings = create_string_store ();
  state->bufferSize = strlen (buffer);
  wh_array_init (&state->parsedValues, sizeof (struct SemValue));

  init_glbl_stmt (&state->globalStmt);
  state->pCurrentStmt = &state->globalStmt;
}

static void
free_state (struct ParserState *state)
{
  release_string_store (state->strings);
  clear_glbl_stmt (&(state->globalStmt));
  wh_array_clean (&state->parsedValues);

}

static bool_t
check_used_vals (struct ParserState *state)
{
  int vals_count = wh_array_count (&state->parsedValues);
  while (--vals_count >= 0)
    {
      struct SemValue *val = wh_array_get (&state->parsedValues, vals_count);
      if (val->val_type != VAL_REUSE)
        {
          return TRUE;                /* found value still in use */
        }

    }

  return FALSE;                        /* no value in use */
}

char proc_decl_buffer[] =
  "PROCEDURE proc1() RETURN BOOL "
  "DO "
  "LET a,b,c AS BOOL; "
  "a  = TRUE; "
  "WHILE (a) DO "
  "IF (b) THEN "
  "BREAK; "
  "ELSEIF (c) THEN "
  "CONTINUE; "
  "END "
  "END "
  "RETURN b; "
  "ENDPROC "
  " "
  "PROCEDURE proc2() RETURN BOOL "
  "DO "
  "LET a,b,c AS BOOL; "
  "a  = TRUE; "
  "DO "
  "IF (b) THEN "
  "CONTINUE; "
  "ELSEIF (c) THEN "
  "BREAK; " "END " "UNTIL (a); " "RETURN         c; " "ENDPROC " "";

static int32_t
get_int32 (uint8_t * buffer)
{
  int32_t value = 0;
  value = buffer[3];
  value <<= 8;
  value += buffer[2];
  value <<= 8;
  value += buffer[1];
  value <<= 8;
  value += buffer[0];

  return value;
}

static bool_t
check_procedure_1 (struct ParserState *state, char * proc_name)
{
  struct Statement *stmt =
    find_proc_decl (state, proc_name, strlen (proc_name), FALSE);
  uint8_t *code = wh_ostream_data (stmt_query_instrs (stmt));
  int shift = 0;
  int cond_exp_pos = 5;
  int while_end_pos = 0;

  if (w_opcode_decode (code + cond_exp_pos + 2) != W_JFC)
    {
      return FALSE;
    }
  else
    {
      while_end_pos = get_int32 (code + cond_exp_pos + 3);
      while_end_pos += cond_exp_pos + 2;
      if (w_opcode_decode (code + while_end_pos - 5) != W_JMP)
        {
          return FALSE;
        }
      else
        {
          int jmp_back = get_int32 (code + while_end_pos - 4);
          jmp_back += while_end_pos - 5;
          if (jmp_back != cond_exp_pos)
            {
              return FALSE;
            }
        }
    }

  /* check the break statement */
  shift = cond_exp_pos + 2 + 5 + 2 + 5;
  if (w_opcode_decode (code + shift) != W_JMP)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  if (shift != while_end_pos)
    {
      return FALSE;
    }

  /* check the continue statement */
  shift = cond_exp_pos + 2 + 5 + 2;
  if (w_opcode_decode (code + shift) != W_JFC)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  shift += 2;
  if (w_opcode_decode (code + shift) != W_JFC)
    {
      return FALSE;
    }
  shift += 5;
  if (w_opcode_decode (code + shift) != W_JMP)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  if (shift != cond_exp_pos)
    {
      return FALSE;
    }

  return TRUE;
}

static bool_t
check_procedure_2 (struct ParserState *state, char * proc_name)
{
  struct Statement *stmt =
    find_proc_decl (state, proc_name, strlen (proc_name), FALSE);
  uint8_t *code = wh_ostream_data (stmt_query_instrs (stmt));
  int shift = 0;
  int cond_exp_pos = 0;

  /* check continue statement */
  shift = 5 + 2;
  if (w_opcode_decode (code + shift) != W_JFC)
    {
      return FALSE;
    }
  shift += 5;
  if (w_opcode_decode (code + shift) != W_JMP)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  cond_exp_pos = shift;
  shift += 2;
  if (w_opcode_decode (code + shift) != W_JTC)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  if (shift != 5)
    {
      return FALSE;
    }

  /* check the break statement */
  shift = cond_exp_pos - 5;
  if (w_opcode_decode (code + shift) != W_JMP)
    {
      return FALSE;
    }
  shift += get_int32 (code + shift + 1);
  if (shift != (cond_exp_pos + 2 + 5))
    {
      return FALSE;
    }

  return TRUE;
}

static bool_t
check_all_procs (struct ParserState *state)
{

  return check_procedure_1 (state, "proc1") &&
    check_procedure_2 (state, "proc2");
}

int
main ()
{
  bool_t test_result = TRUE;
  struct ParserState state = { 0, };

  init_state_for_test (&state, proc_decl_buffer);

  printf ("Testing parse..");
  if (yyparse (&state) != 0)
    {
      printf ("FAILED\n");
      test_result = FALSE;
    }
  else
    {
      printf ("PASSED\n");
    }

  if (test_result)
    {
      printf ("Testing garbage vals...");
      if (check_used_vals (&state))
        {
          /* those should no be here */
          printf ("FAILED\n");
          test_result = FALSE;
        }
      else
        {
          printf ("PASSED\n");
        }
    }

  printf ("Testing loop statements ...");
  if (check_all_procs (&state))
    {
      printf ("PASSED\n");
    }
  else
    {
      printf ("FAILED\n");
      test_result = FALSE;
    }

  free_state (&state);
  printf ("Memory peak: %u bytes \n", test_get_mem_peak ());
  printf ("Current memory usage: %u bytes...", test_get_mem_used ());
  if (test_get_mem_used () != 0)
    {
      test_result = FALSE;
      printf ("FAILED\n");
    }
  else
    {
      printf ("PASSED\n");
    }

  if (test_result == FALSE)
    {
      printf ("TEST RESULT: FAIL\n");
      return -1;
    }

  printf ("TEST RESULT: PASS\n");
  return 0;
}
