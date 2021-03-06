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

#include <assert.h>
#include <iostream>
#include <map>

#include "compiler//whaisc.h"
#include "client/whais_connector.h"
#include "utils/tokenizer.h"

#include "wcmd_cmdsmgr.h"
#include "wcmd_optglbs.h"



using namespace std;
using namespace whais;



static const uint_t MAX_DECODED_STRING = 256;



static string
decode_basic_type(const uint16_t type)
{
  switch(GET_BASE_TYPE(type))
  {
  case T_BOOL:
    return "BOOL";

  case T_CHAR:
    return "CHAR";

  case T_DATE:
    return "DATE";

  case T_DATETIME:
    return "DATETIME";

  case T_HIRESTIME:
    return "HIRESTIME";

  case T_UINT8:
    return "UINT8";

  case T_UINT16:
    return "UINT16";

  case T_UINT32:
    return "UINT32";

  case T_UINT64:
    return "UINT64";

  case T_INT8:
    return "INT8";

  case T_INT16:
    return "INT16";

  case T_INT32:
    return "INT32";

  case T_INT64:
    return "INT64";

  case T_REAL:
    return "REAL";

  case T_RICHREAL:
    return "RICHREAL";

  case T_TEXT:
    return "TEXT";

  case T_UNDETERMINED:
    return "UNDEFINED";

  default:
    assert(false);
  }

  return nullptr;
}


string
wcmd_decode_typeinfo(unsigned int type)
{
  if ( ! IS_ARRAY(type))
    return decode_basic_type(type);

  if (GET_BASE_TYPE(type) == WHC_TYPE_NOTSET)
    return "ARRAY";

  return decode_basic_type(GET_BASE_TYPE(type)) + " ARRAY";
}


map<string, CmdEntry> sCommands;

static const char descHelp[]    = "Display help on available commands.";
static const char descExtHelp[] = "Display the list of available commands "
                                     "or an extended help about a command.\n"
                                     "Usage:\n"
                                     "  help [command]\n"
                                     "Example:\n"
                                     "  help table";

static bool
cmdHelp(const string& cmdLine, ENTRY_CMD_CONTEXT)
{
  size_t currPosition = 0;
  string token        = CmdLineNextToken(cmdLine, currPosition);

  assert(token == "help");

  if (currPosition >= (cmdLine.length() - 1))
    {
      //This is the only token. List commands.
      map<string, CmdEntry>::iterator it = sCommands.begin();

      while (it != sCommands.end())
        {
          const streamsize prevWidth = cout.width(20);
          const char       prevFill  = cout.fill(' ');

          cout << left << it->first;
          cout.width(prevWidth);
          cout.fill(prevFill);
          cout << it->second.mDesc << endl;

          ++it;
        }
      return true;
    }

  token = CmdLineNextToken(cmdLine, currPosition);
  if ( ! CmdLineNextToken(cmdLine, currPosition).empty())
    {
      cout << "Invalid command parameters.\n";
      return false;
    }
  const CmdEntry *const entry = FindCmdEntry(token.c_str());
  if (entry == nullptr)
    {
      cout << "Unknown command '" << token << "'.\n";
      return false;
    }

  cout << "Command '"<< entry->mName << "' help:\n\n";
  cout << entry->mExtendedDesc << endl;

  return true;
}


static const char descEcho[]    = "Prints a text.";
static const char descExtEcho[] = "Used mainly with scripts to print a text "
                                     "provided by the user.\n"
                                     "Usage:\n"
                                     "  echo [user text] ... \n"
                                     "Example:\n"
                                     "  echo First stage completed.";

static bool
cmdEcho(const string& cmdLine, ENTRY_CMD_CONTEXT)
{
  size_t currPosition = 0;
  string token        = CmdLineNextToken(cmdLine, currPosition);

  assert(token == "echo");

  while (currPosition < cmdLine.length()
         && isspace(cmdLine.at(currPosition)))
    {
      ++currPosition;
    }

  cout << cmdLine.substr(currPosition) << endl;

  return true;
}


void
InitCmdManager()
{
  CmdEntry entry;

  entry.mName         = "help";
  entry.mDesc         = descHelp;
  entry.mExtendedDesc = descExtHelp;
  entry.mCmd          = cmdHelp;
  entry.mContext      = nullptr;
  entry.mShowStatus   = false;

  RegisterCommand(entry);


  entry.mName         = "echo";
  entry.mDesc         = descEcho;
  entry.mExtendedDesc = descExtEcho;
  entry.mCmd          = cmdEcho;
  entry.mContext      = nullptr;
  entry.mShowStatus   = false;

  RegisterCommand(entry);
}


void
RegisterCommand(const CmdEntry& entry)
{
  assert(entry.mName != nullptr);
  assert(entry.mDesc != nullptr);
  assert(entry.mExtendedDesc != nullptr);

  pair<string, CmdEntry> cmdEntry(entry.mName, entry);

  sCommands.insert(cmdEntry);
}


const CmdEntry*
FindCmdEntry(const char* const name)
{
  const string command = name;
  CmdEntry*    entry    = nullptr;

  map<string, CmdEntry>::iterator it = sCommands.find(command);
  if (it != sCommands.end())
    entry = &it->second;

  return entry;
}


const string
CmdLineNextToken(const string& cmdLine, size_t& inoutPosition)
{
  static string delimiters = " \t";

  return NextToken(cmdLine, inoutPosition, delimiters);
}


void
printException(ostream& outputStream, const Exception& e)
{
  const VERBOSE_LEVEL level = GetVerbosityLevel();

  if (e.Type() == DBS_EXCEPTION)
    {
      if (level >= VL_DEBUG)
        outputStream << "DBS framework exception.\n";

      if ( e.Message().empty())
        outputStream << e.Description() <<endl;
    }
  else if (e.Type() == INTERPRETER_EXCEPTION)
    {
      if (level >= VL_DEBUG)
        outputStream << "Interpreter exception.\n";

      assert(! e.Message().empty());

      if ( e.Message().empty())
        outputStream << e.Description() <<endl;
    }
  else if (e.Type() == FILE_EXCEPTION)
    {
      if (level >= VL_DEBUG)
        outputStream << "File system IO exception.\n";

      char errorDesc[MAX_DECODED_STRING];

      whf_err_to_str(e.Code(), errorDesc, sizeof errorDesc);
      outputStream << errorDesc << endl;
    }
  else
    {
      assert(false);

      outputStream << "Unknown exception throwed.\n";
    }


  if ( ! e.Message().empty())
    outputStream << e.Message() << endl;

  if (level >= VL_DEBUG)
    {
      outputStream << "Type: " << e.Type() << endl;
      outputStream << "Code: " << e.Code() << endl;
      outputStream << "File: " << e.File() << endl;
      outputStream << "Line: " << e.Line() << endl;
    }
}

