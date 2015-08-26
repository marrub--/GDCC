//-----------------------------------------------------------------------------
//
// Copyright(C) 2015 David Hill
//
// See COPYLIB for license information.
//
//-----------------------------------------------------------------------------
//
// Non-local jumps.
//
//-----------------------------------------------------------------------------

#include <setjmp.h>


//----------------------------------------------------------------------------|
// Extern Functions                                                           |
//

//=========================================================
// Restore calling environment.
//

//
// longjmp
//
_Noreturn void (longjmp)(jmpbuf env, int val)
{
   if(!val)
      val = 1;

   __longjmp(env, val);
}

// EOF

