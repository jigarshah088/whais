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

#include "pm_interpreter.h"

using namespace std;
using namespace prima;

void
InitInterpreter ()
{
}

I_InterpreterSession&
GetInterpreterInstance ()
{
  static GlobalSession slGlobalSession;

  return slGlobalSession;
}

I_InterpreterSession&
GetInterpreterInstance (I_DBSHandler& dbsHandler)
{
  //TODO: Not implemented yet
  return GetInterpreterInstance ();
}

void
ReleaseInterpreterInstance (I_InterpreterSession& dbsHandler)
{
  //TODO: Not implemented yet
}


void
CleanInterpreter ()
{
  //TODO: Not implemented yet
}
