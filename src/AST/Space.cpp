//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2014 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Abstract Syntax Tree space handling.
//
//-----------------------------------------------------------------------------

#include "AST/Space.hpp"

#include "IR/Addr.hpp"
#include "IR/Exp.hpp"
#include "IR/Linkage.hpp"
#include "IR/Program.hpp"


//----------------------------------------------------------------------------|
// Global Functions                                                           |
//

namespace GDCC
{
   namespace AST
   {
      //
      // Space constructor
      //
      Space::Space(Core::String name_, Core::String glyph_) :
         glyph{glyph_},
         linka{IR::Linkage::None},
         name {name_},
         space{IR::AddrBase::Gen},
         value{nullptr},

         defin{false}
      {
      }

      //
      // Space destructor
      //
      Space::~Space()
      {
      }

      //
      // Space::genSpace
      //
      void Space::genSpace(IR::Program &prog) const
      {
         IR::Space sp{{space, glyph}};

         sp.linka = linka;

         sp.defin = defin;

         if(value)
         {
            auto val = value->getValue();
            if(val.v == IR::ValueBase::Fixed)
               sp.value = number_cast<Core::FastU>(val.vFixed.value);
            else
               sp.alloc = true;
         }
         else
            sp.alloc = true;

         // Merge into existing space (if any).
         prog.mergeSpace(prog.getSpace({space, glyph}), std::move(sp));
      }

      //
      // Space::Create
      //
      Space::Ref Space::Create(Core::String name, Core::String glyph)
      {
         return static_cast<Ref>(new Space(name, glyph));
      }
   }
}

// EOF

