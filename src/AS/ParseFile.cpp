//-----------------------------------------------------------------------------
//
// Copyright (C) 2015 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Assembly file parsing.
//
//-----------------------------------------------------------------------------

#include "AS/Parse.hpp"

#include "AS/Macro.hpp"
#include "AS/TStream.hpp"

#include "Core/File.hpp"
#include "Core/Token.hpp"

#include <iostream>


//----------------------------------------------------------------------------|
// Extern Functions                                                           |
//

namespace GDCC
{
   namespace AS
   {
      //
      // ParseFile
      //
      void ParseFile(char const *inName, IR::Program &prog)
      {
         auto buf = Core::FileOpenStream(inName, std::ios_base::in);
         if(!buf)
         {
            std::cerr << "couldn't open '" << inName << "' for reading\n";
            throw EXIT_FAILURE;
         }

         TStream   in    {*buf, inName};
         MacroMap  macros{};
         ParserCtx ctx   {in, macros, prog};

         while(!ctx.in.peek(Core::TOK_EOF))
            AS::ParseDeclaration(ctx);
      }
   }
}

// EOF

