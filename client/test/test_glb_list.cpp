/*
 * test_glb_list.cpp
 *
 *  Created on: Jan 9, 2013
 *      Author: ipopa
 */

#include <iostream>
#include <cstring>

#include "test_client_common.h"

using namespace std;

struct GlobalValueEntry
{
  const D_CHAR* const name;
  bool                visited;
};

GlobalValueEntry _procedures[] =
    {
        {"bool_", false},
        {"char_", false},
        {"date_", false},
        {"datetime_", false},
        {"hirestime_", false},
        {"int8_", false},
        {"int16_", false},
        {"int32_", false},
        {"int64_", false},
        {"uint8_", false},
        {"uint16_", false},
        {"uint32_", false},
        {"uint64_", false},
        {"real_", false},
        {"richreal_", false},
        {"text_", false},

        {"array_bool_", false},
        {"array_char_", false},
        {"array_date_", false},
        {"array_datetime_", false},
        {"array_hirestime_", false},
        {"array_int8_", false},
        {"array_int16_", false},
        {"array_int32_", false},
        {"array_int64_", false},
        {"array_uint8_", false},
        {"array_uint16_", false},
        {"array_uint32_", false},
        {"array_uint64_", false},
        {"array_real_", false},
        {"array_richreal_", false},
//        {"array_text_", false},

        {"field_bool_", false},
        {"field_char_", false},
        {"field_date_", false},
        {"field_datetime_", false},
        {"field_hirestime_", false},
        {"field_int8_", false},
        {"field_int16_", false},
        {"field_int32_", false},
        {"field_int64_", false},
        {"field_uint8_", false},
        {"field_uint16_", false},
        {"field_uint32_", false},
        {"field_uint64_", false},
        {"field_real_", false},
        {"field_richreal_", false},
        {"field_text_", false},

        {"field_array_bool_", false},
        {"field_array_char_", false},
        {"field_array_date_", false},
        {"field_array_datetime_", false},
        {"field_array_hirestime_", false},
        {"field_array_int8_", false},
        {"field_array_int16_", false},
        {"field_array_int32_", false},
        {"field_array_int64_", false},
        {"field_array_uint8_", false},
        {"field_array_uint16_", false},
        {"field_array_uint32_", false},
        {"field_array_uint64_", false},
        {"field_array_real_", false},
        {"field_array_richreal_", false},
//        {"field_array_text_", false},

        {"one_field_table_", false},
        {"complete_field_table_", false}
    };

static bool
test_global_name_match (const D_CHAR* glb_name)
{
  const D_CHAR suffix[] = "global_var_this_is_a_long_variable_name_suffix_coz_I_need_to_trigger_an_odd_behavior_001_good";
  D_CHAR buffer[1024];
  const D_UINT glbsCount = sizeof (_procedures)/sizeof (_procedures[0]);

  for (D_UINT i = 0; i < glbsCount; ++i)
    {
      strcpy (buffer, _procedures[i].name);
      strcat (buffer, suffix);
      if (strcmp (buffer, glb_name) == 0)
        {
          if (_procedures[i].visited)
            return false;
          else
            {
              _procedures[i].visited = true;
              return true;
            }
        }
    }

  return false;
}

static bool
test_global_values_list (W_CONNECTOR_HND hnd)
{
  const D_CHAR* recvGlbName = NULL;
  const D_UINT  glbsCount = sizeof (_procedures)/sizeof (_procedures[0]);

  D_UINT globalsCount;
  D_UINT index = 0;

  cout << "Testing the global values listing ... ";

  if ((WListGlobals (hnd, &globalsCount) != WCS_OK)
      || (globalsCount != glbsCount))
    {
      goto test_global_values_list_error;
    }

  do
    {
      if (WListGlobalsFetch (hnd, &recvGlbName) != WCS_OK)
        goto test_global_values_list_error;
      else if ((recvGlbName != NULL)
               && ! test_global_name_match (recvGlbName))
        {
          goto test_global_values_list_error;
        }

      ++index; //Only good for conditional breakpoints.
    }
  while (recvGlbName != NULL);

  for (index = 0; index < glbsCount; ++index)
    {
      if (! _procedures[index].visited)
        goto test_global_values_list_error;
    }

  cout << "OK\n";
  return true;

test_global_values_list_error:

  cout << "FAIL\n";
  return false;
}

static bool
test_for_errors (W_CONNECTOR_HND hnd)
{
  D_UINT          glbsCount;
  const D_CHAR*   nameFetched;

  cout << "Testing against error conditions ... ";
  if ((WListGlobals(NULL, &glbsCount) != WCS_INVALID_ARGS)
      || (WListGlobals (hnd, NULL) != WCS_INVALID_ARGS)
      || (WListGlobals (NULL, NULL) != WCS_INVALID_ARGS))
    {
      goto test_for_errors_fail;
    }
  else if (WListGlobalsFetch (hnd, &nameFetched) != WCS_INVALID_ARGS)
    goto test_for_errors_fail;
  else if (WListGlobals (hnd, &glbsCount) != WCS_OK)
    goto test_for_errors_fail;
  else if ((WListGlobalsFetch (NULL, NULL) != WCS_INVALID_ARGS)
            || (WListGlobalsFetch (NULL, &nameFetched) != WCS_INVALID_ARGS)
            || (WListGlobalsFetch (hnd, NULL) != WCS_INVALID_ARGS))
    {
      goto test_for_errors_fail;
    }
  else if (WListGlobalsFetch (hnd, &nameFetched) != WCS_OK)
    goto test_for_errors_fail;

  cout << "OK\n";
  return true;

test_for_errors_fail :
  cout << "FAIL\n";
  return false;
}



const D_CHAR*
DefaultDatabaseName ()
{
  return "test_list_db";
}

const D_UINT
DefaultUserId ()
{
  return 0;
}

const D_CHAR*
DefaultUserPassword ()
{
  return "root_test_password";
}

int
main (int argc, const char** argv)
{
  W_CONNECTOR_HND       hnd = NULL;

  bool success = tc_settup_connection (argc, argv, &hnd);

  success = success && test_for_errors (hnd);
  success = success && test_global_values_list (hnd);

  WClose (hnd);

  if (!success)
    {
      cout << "TEST RESULT: FAIL" << std::endl;
      return 1;
    }


  cout << "TEST RESULT: PASS" << std::endl;

  return 0;
}

