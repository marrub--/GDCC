//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2018 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Platform information/selection.
//
//-----------------------------------------------------------------------------

#ifndef GDCC__Platform__Platform_H__
#define GDCC__Platform__Platform_H__

#include "../Platform/Types.hpp"


//----------------------------------------------------------------------------|
// Types                                                                      |
//

namespace GDCC::Platform
{
   //
   // Format
   //
   enum class Format
   {
      None,
      ACSE,
      DGE_NTS,
   };

   //
   // Target
   //
   enum class Target
   {
      None,
      Doominati,
      ZDoom,
      Zandronum,
   };
}


//----------------------------------------------------------------------------|
// Extern Objects                                                             |
//

namespace GDCC::Platform
{
   extern Format FormatCur;
   extern Target TargetCur;
}


//----------------------------------------------------------------------------|
// Extern Functions                                                           |
//

namespace GDCC::Platform
{
   unsigned GetByteBitsI();

   // How many extra auto words does this type of call require?
   unsigned GetCallAutoAdd(IR::CallType call);

   unsigned GetWordAlign();
   unsigned GetWordBits();
   unsigned GetWordBytes();
   unsigned GetWordPoint();
   unsigned GetWordShift();

   // Does this type of call require explicit auto pointer propagation?
   bool IsCallAutoProp(IR::CallType call);

   // Do extra arguments to this type of call require variadic handling?
   bool IsCallVaria(IR::CallType call);

   bool IsFamily_ZDACS();

   // Does null have a zero representation?
   bool IsZeroNull_DJump();
   bool IsZeroNull_Funct(IR::CallType call);
   bool IsZeroNull_Point(IR::AddrBase addr);
   bool IsZeroNull_StrEn();

   // Must unused, undefined objects in this space be emitted?
   bool MustEmitObject(IR::AddrBase addr);
}

#endif//GDCC__Platform__Platform_H__

