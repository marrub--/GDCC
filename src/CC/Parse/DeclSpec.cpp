//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2015 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// C declaration specifier parsing.
//
//-----------------------------------------------------------------------------

#include "CC/Parse.hpp"

#include "CC/Scope.hpp"
#include "CC/Type.hpp"

#include "AST/Attribute.hpp"
#include "AST/Type.hpp"

#include "Core/Exception.hpp"
#include "Core/TokenStream.hpp"


//----------------------------------------------------------------------------|
// Global Functions                                                           |
//

namespace GDCC
{
   namespace CC
   {
      //
      // Parser::isDeclSpec
      //
      bool Parser::isDeclSpec(Scope &scope)
      {
         auto tok = in.peek();
         if(tok.tok != Core::TOK_Identi && tok.tok != Core::TOK_KeyWrd)
            return false;

         switch(tok.str)
         {
            // storage-class-specifier
         case Core::STR__Thread_local: return true;
         case Core::STR_auto:          return true;
         case Core::STR_extern:        return true;
         case Core::STR_register:      return true;
         case Core::STR_static:        return true;
         case Core::STR_typedef:       return true;

            // function-specifier
         case Core::STR__Noreturn: return true;
         case Core::STR_inline:    return true;

            // alignment-specifier
         case Core::STR__Alignas: return true;

            // type-qualifier
            // type-specifier
         default:
            return isTypeQual(scope) || isTypeSpec(scope);
         }
      }

      //
      // Parser::parseDeclSpec
      //
      void Parser::parseDeclSpec(Scope &scope, AST::Attribute &attr)
      {
         enum DeclStor
         {
            declStorNone,

            declStorAuto,
            declStorExte,
            declStorRegi,
            declStorStat,
            declStorType,
         };

         auto pos = in.peek().pos;

         AST::TypeQual declQual = AST::QualNone;
         TypeSpec      declSpec;
         DeclStor      declStor = declStorNone;
         int           declThrd = 0;

         //
         // setDeclStor
         //
         auto setDeclStor = [&](DeclStor stor)
         {
            if(declStor)
               throw Core::ExceptStr(pos, "multiple storage class specifier");

            declStor = stor;
         };

         // Read declaration-specifier tokens until there are no more.
         for(;;)
         {
            auto const &tok = in.peek();
            if(tok.tok != Core::TOK_Identi && tok.tok != Core::TOK_KeyWrd)
               break;

            switch(tok.str)
            {
               // storage-class-specifier
            case Core::STR__Thread_local: ++declThrd;                break;
            case Core::STR_auto:          setDeclStor(declStorAuto); break;
            case Core::STR_extern:        setDeclStor(declStorExte); break;
            case Core::STR_register:      setDeclStor(declStorRegi); break;
            case Core::STR_static:        setDeclStor(declStorStat); break;
            case Core::STR_typedef:       setDeclStor(declStorType); break;

               // function-specifier
            case Core::STR__Noreturn: attr.funcNoReturn = true; break;
            case Core::STR_inline:    attr.funcInline   = true; break;

               // alignment-specifier
            case Core::STR__Alignas: throw Core::ExceptStr(pos, "_Alignas stub");

            default:
               // type-specifier
               if(isTypeSpec(scope))
                  {parseTypeSpec(scope, attr, declSpec); continue;}

               // type-qualifier
               if(isTypeQual(scope))
                  {parseTypeQual(scope, declQual); continue;}

               goto parse_done;
            }

            in.get();
         }

         parse_done:;

         // Check for attributes.
         auto attrType = attr;
         if(isAttrSpec(scope))
            parseAttrSpecList(scope, attrType);

         // Validate the storage class.
         if(declThrd > 1 || (declThrd && declStor != declStorNone &&
            declStor != declStorExte && declStor != declStorStat))
            throw Core::ExceptStr(pos, "invalid thread_local");

         // Finalize the type specifiers.
         declSpec.finish(attrType, declQual, pos);
         attr.type = attrType.type;

         // Set attribute storage class.
         if(declThrd) attr.storeThread = true;
         switch(declStor)
         {
         case declStorNone:                        break;
         case declStorAuto: attr.storeAuto = true; break;
         case declStorExte: attr.storeExt  = true; break;
         case declStorRegi: attr.storeReg  = true; break;
         case declStorStat: attr.storeInt  = true; break;
         case declStorType: attr.isTypedef = true; break;
         }
      }
   }
}

// EOF

