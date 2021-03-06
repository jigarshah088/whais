#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "../parser/parser.h"
#include "../semantics/vardecl.h"
#include "../semantics/opcodes.h"
#include "../semantics/procdecl.h"
#include "../semantics/wlog.h"

#include "custom/include/test/test_fmw.h"

extern int yyparse(struct ParserState *);

uint_t last_msg_code = 0xFF, last_msg_type = 0XFF;

static int
get_buffer_line_from_pos(const char *buffer, uint_t buff_pos)
{
  uint_t count = 0;
  int result = 1;

  if (buff_pos == IGNORE_BUFFER_POS)
    {
      return -1;
    }

  while (count < buff_pos)
    {
      if (buffer[count] == '\n')
        {
          ++result;
        }
      else if (buffer[count] == 0)
        {
          assert(0);
        }
      ++count;
    }
  return result;
}

static char *MSG_PREFIX[] = {
  "", "error ", "warning ", "error "
};

void
my_postman(WLOG_FUNC_CONTEXT bag,
            uint_t buff_pos,
            uint_t msg_id,
            uint_t msgType, const char * msgFormat, va_list args)
{
  const char *buffer = (const char *) bag;
  int buff_line = get_buffer_line_from_pos(buffer, buff_pos);

  printf(MSG_PREFIX[msgType]);
  printf("%d : line %d: ", msg_id, buff_line);
  vprintf(msgFormat, args);
  printf("\n");

  if (msgType != MSG_EXTRA_EVENT)
    {
      last_msg_code = msg_id;
      last_msg_type = msgType;
    }
}

char test_prog_1[] = ""
  "VAR table_2 TABLE( f1 DATE); \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "Proc_1( 0 ); \n"
  "RETURN table_2[10, f1]; \n"
  "ENDPROC \n";

char test_prog_2[] = ""
  "VAR table_1 TABLE( f1 DATE, f2 INT16); \n"
  " \n"
  "PROCEDURE Proc_1( proc_arg INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN proc_arg; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR proc_arg INT16; \n"
  "VAR some_arg TEXT; \n"
  "Proc_1( proc_arg, some_arg ); \n"
  "RETURN table_1[0, f1]; \n"
  "ENDPROC \n";

char test_prog_3[] = ""
  "VAR table_1 TABLE( f1 DATE, f2 INT16); \n"
  "VAR table_2 TABLE( f1 DATE); \n"
  " \n"
  "PROCEDURE Proc_1( proc_arg INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN proc_arg; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "Proc_1(); \n" "RETURN '2010/01/01'; \n" "ENDPROC \n";

char test_prog_4[] = ""
  "PROCEDURE Proc_1( v1 DATE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1( some_var, TRUE ); \n" "RETURN some_var; \n" "ENDPROC \n";

char test_prog_5[] = ""
  "PROCEDURE Proc_1( v1 DATE FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE ARRAY FIELD;"
  "Proc_1( some_var, 10 ); \n" "RETURN some_var; \n" "ENDPROC \n";

char test_prog_6[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_7[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_8[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_9[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_10[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_11[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_12[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_13[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_14[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_15[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_16[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_17[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv TABLE;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_18[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_19[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_20[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_21[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";


char test_prog_22[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY;"
  "Proc_1( 10, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_23[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_24[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv TABLE;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_25[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_26[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_27[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_28[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_29[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv INT8 ARRAY;"
  "Proc_1( 11, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_30[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv TABLE;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";


char test_prog_31[] = ""
  "PROCEDURE Proc_1( v1 INT8, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv TABLE;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_32[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE ARRAY;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_33[] = ""
  "PROCEDURE Proc_1( v1 INT8 ARRAY FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_34[] = ""
  "PROCEDURE Proc_1( v1 INT8 FIELD, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_35[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_36[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE ARRAY FIELD;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_37[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "VAR tv DATE ARRAY;"
  "Proc_1( tv, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_38[] = ""
  "PROCEDURE Proc_1( v1 TABLE, v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1( some_var, 10 ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";

char test_prog_39[] = ""
  "PROCEDURE Proc_1(v2 INT16) RETURN INT16 \n"
  "DO \n"
  "RETURN v2; \n"
  "ENDPROC \n"
  " \n"
  "PROCEDURE Proc_1_2() RETURN DATE \n"
  "DO \n"
  "VAR some_var DATE;"
  "Proc_1(some_var ); \n"
  "RETURN some_var;\n"
  "ENDPROC \n";




bool_t
test_for_error(const char *test_buffer, uint_t err_expected, uint_t err_type)
{
  WH_COMPILED_UNIT handler;
  bool_t test_result = TRUE;

  last_msg_code = 0xFF, last_msg_type = 0XFF;
  handler = wh_compiler_load(test_buffer,
                            strlen(test_buffer),
                            &my_postman, (WH_MESSENGER_CTXT) test_buffer);

  if (err_type == MSG_WARNING_EVENT)
    {
      if (handler == NULL)
        {
          test_result = FALSE;
          printf("The expected error code is actually a warning message. "
                  "The buffer should have been compiled anyway, but it "
                  "failed\n");
          return test_result;
        }
      else
        wh_compiler_discard(handler);

      return test_result ;
    }
  else if (handler != NULL)
    {
      printf("Looks like the buffer was compiled succefully, though an error "
              "was expected.\n");
      test_result = FALSE;
      wh_compiler_discard(handler);

      return test_result ;
    }

  if ((last_msg_code != err_expected) || (last_msg_type != err_type))
    test_result = FALSE;

  if (test_get_mem_used() != 0)
    {
      printf("Current memory usage: %u bytes! It should be 0.",
              (uint_t)test_get_mem_used());
      test_result = FALSE;
    }
  return test_result;
}

int
main()
{
  bool_t test_result = TRUE;

  printf("Testing for received error messages...\n");
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_1, MSG_NO_PROC,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_2,
                                                     MSG_PROC_MORE_ARGS,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_3,
                                                     MSG_PROC_LESS_ARGS,
                                                     MSG_WARNING_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_4,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_5,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_6,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_7,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_8,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_9,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_10,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_11,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_12,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_13,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_14,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_15,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_16,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_17,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_18,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_19,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_20,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_21,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_22,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_23,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_24,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_25,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_26,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_27,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_28,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_29,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_30,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_31,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_32,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_33,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_34,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_35,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);
  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_36,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_37,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_38,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  test_result =
    (test_result == FALSE) ? FALSE : test_for_error(test_prog_39,
                                                     MSG_PROC_ARG_NA,
                                                     MSG_ERROR_EVENT);

  if (test_result == FALSE)
    {
      printf("TEST RESULT: FAIL\n");
      return -1;
    }

  printf("TEST RESULT: PASS\n");
  return 0;
}
