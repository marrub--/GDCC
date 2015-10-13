//-----------------------------------------------------------------------------
//
// Copyright (C) 2015 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// ACS object-declaration parsing.
//
//-----------------------------------------------------------------------------

#include "ACC/Parse.hpp"

#include "ACC/Scope.hpp"

#include "AST/Attribute.hpp"
#include "AST/Object.hpp"
#include "AST/Space.hpp"
#include "AST/Statement.hpp"
#include "AST/Storage.hpp"

#include "CC/Exp.hpp"
#include "CC/Exp/Assign.hpp"
#include "CC/Exp/Init.hpp"
#include "CC/Init.hpp"
#include "CC/Scope/Global.hpp"
#include "CC/Scope/Local.hpp"
#include "CC/Statement.hpp"

#include "Core/Exception.hpp"
#include "Core/TokenStream.hpp"

#include "IR/Addr.hpp"
#include "IR/Linkage.hpp"


//----------------------------------------------------------------------------|
// Static Functions                                                           |
//

namespace GDCC
{
   namespace ACC
   {
      //
      // GetDeclBase_Object
      //
      template<typename T>
      static AST::Statement::CRef GetDeclBase_Object(Parser &ctx, T &scope)
      {
         // object-declaration:
         //    declaration-specifiers init-declarator-list ;

         AST::Attribute attrBase;
         attrBase.linka = IR::Linkage::ExtACS;

         // declaration-specifiers
         if(!ctx.isDeclSpec(scope))
            throw Core::ParseExceptExpect(ctx.in.peek(), "object-declaration", false);

         auto pos = ctx.in.peek().pos;

         ctx.parseDeclSpec(scope, attrBase);

         std::vector<AST::Statement::CRef> inits;

         // init-declarator-list:
         //    init-declarator
         //    init-declarator-list , init-declarator
         do
         {
            // init-declarator:
            //    declarator
            //    declarator = initializer

            // declarator
            auto attr = attrBase;
            ctx.parseDeclarator(scope, attr);

            // Objects must have object type.
            if(!attr.type->isCTypeObject())
               throw Core::ExceptStr(attr.namePos, "expected object type");

            ParseDeclObject(ctx, scope, attr, inits);
         }
         while(ctx.in.drop(Core::TOK_Comma));

         if(!ctx.in.drop(Core::TOK_Semico))
            throw Core::ParseExceptExpect(ctx.in.peek(), ";", true);

         switch(inits.size())
         {
         case  0: return AST::StatementCreate_Empty(pos);
         case  1: return inits[0];
         default: return AST::StatementCreate_Multi(pos,
            Core::Array<AST::Statement::CRef>(inits.begin(), inits.end()));
         }
      }

      //
      // GetDeclObjectSpace
      //
      template<typename T>
      static void GetDeclObjectSpace(Parser &ctx, T &scope,
         AST::Attribute &attr, IR::AddrBase base)
      {
         auto spaceAttr = attr;

         spaceAttr.name      += "$arr";
         spaceAttr.space.base = base;

         auto space = scope.getSpace(spaceAttr);

         space->defin = !ctx.importing;
         space->value = spaceAttr.addrI;

         // Don't use the supplied address for the contained object.
         attr.addrI = nullptr;

         attr.type = attr.type->getTypeArrayQualAddr({base, space->glyph});
      }

      //
      // GetDeclObject (global)
      //
      static AST::Object::Ref GetDeclObject(Parser &ctx,
         CC::Scope_Global &scope, AST::Attribute &attr, bool init)
      {
         if(attr.storeInt)
            throw Core::ExceptStr(attr.namePos, "file scope static");

         if(attr.storeGbl)
         {
            if(attr.type->isTypeArray())
               GetDeclObjectSpace(ctx, scope, attr, IR::AddrBase::GblArr);
            else
               attr.type = attr.type->getTypeQualAddr(IR::AddrBase::GblReg);
         }
         else if(attr.storeHub)
         {
            if(attr.type->isTypeArray())
               GetDeclObjectSpace(ctx, scope, attr, IR::AddrBase::HubArr);
            else
               attr.type = attr.type->getTypeQualAddr(IR::AddrBase::HubReg);
         }
         else
         {
            if(attr.type->isTypeArray())
               GetDeclObjectSpace(ctx, scope, attr, IR::AddrBase::ModArr);
            else
               attr.type = attr.type->getTypeQualAddr(IR::AddrBase::ModReg);
         }

         // Fetch/generate object.
         auto obj = scope.getObject(attr);

         // If init, wait until later to mark as definition because the type
         // might need to be completed by the initializer.
         if(!init || ctx.importing)
         {
            // First, make sure it has a complete type.
            if(!obj->type->isTypeComplete())
            {
               // Allow global/world arrays to be incomplete for compatibility.
               if(obj->type->isTypeArray() && (attr.storeGbl || attr.storeHub))
                  obj->type = obj->type->getBaseType()->getTypeArray(1);
               else
                  throw Core::ExceptStr(attr.namePos,
                     "object with incomplete type");
            }

            obj->defin = !ctx.importing;
         }

         // Set address, if one provided.
         if(attr.addrI)
            obj->value = attr.addrI;

         return obj;
      }

      //
      // GetDeclObject (local)
      //
      static AST::Object::Ref GetDeclObject(Parser &ctx,
         CC::Scope_Local &scope, AST::Attribute &attr, bool init)
      {
         // All block-scope declarations are definitions and all block-scope
         // definitions must have no linkage.
         attr.linka = IR::Linkage::None;

         if(attr.storeGbl)
            throw Core::ExceptStr(attr.namePos, "block scope global");

         if(attr.storeHub)
            throw Core::ExceptStr(attr.namePos, "block scope world");

         if(attr.storeInt)
         {
            if(attr.type->isTypeArray())
               GetDeclObjectSpace(ctx, scope, attr, IR::AddrBase::ModArr);
            else
               attr.type = attr.type->getTypeQualAddr(IR::AddrBase::ModReg);
         }
         else
         {
            if(attr.type->isTypeArray())
               GetDeclObjectSpace(ctx, scope, attr, IR::AddrBase::LocArr);
            else
               attr.type = attr.type->getTypeQualAddr(IR::AddrBase::LocReg);
         }

         // Fetch/generate object.
         auto obj = scope.getObject(attr);

         // No-linkage declarations must be definitions.
         if(attr.linka == IR::Linkage::None)
         {
            // But if it has an initializer, do not set defined yet.
            if(!init)
            {
               // First, make sure it has a complete type.
               if(!obj->type->isTypeComplete())
                  throw Core::ExceptStr(attr.namePos,
                     "object with incomplete type");

               obj->defin = !ctx.importing;

               // Give default initializer.
               obj->init = CC::Exp_Init::Create(
                  CC::Init::Create(obj->type, 0, attr.namePos), true);
            }
         }

         // Set address, if one provided.
         if(attr.addrI)
            obj->value = attr.addrI;

         return obj;
      }

      //
      // ParseDeclObject
      //
      template<typename T>
      static void ParseDeclObject(Parser &ctx, T &scope,
         AST::Attribute &attr, std::vector<AST::Statement::CRef> &inits)
      {
         AST::Type::CPtr lookupType;

         // Check compatibility with existing symbol, if any.
         if(auto lookup = scope.find(attr.name))
         {
            if(lookup.res != CC::Lookup::Obj)
               throw Core::ExceptStr(attr.namePos,
                  "name redefined as different kind of symbol");

            // Defer type compatibility check for later.
            lookupType = lookup.resObj->type;
         }

         // Insert special declaration statement.
         if(inits.empty())
            inits.emplace_back(CC::StatementCreate_Decl(attr.namePos, scope));

         // = initializer
         if(ctx.in.drop(Core::TOK_Equal))
         {
            auto obj = GetDeclObject(ctx, scope, attr, true);

            scope.add(attr.name, obj);

            obj->init = ctx.getExp_Init(scope, obj->type);
            obj->type = obj->init->getType();

            if(obj->store == AST::Storage::Static && !obj->init->isIRExp())
               throw Core::ExceptStr(obj->init->pos,
                  "non-constant initializer for static storage object");

            SetDeclObjectInit(ctx, scope, attr, inits, obj);
         }
         else
            scope.add(attr.name, GetDeclObject(ctx, scope, attr, false));

         // Do type compatibility test here because type may have been altered.
         if(lookupType && lookupType != attr.type &&
            (!lookupType->isTypeArray() || !attr.type->isTypeArray() ||
             lookupType->getBaseType() != attr.type->getBaseType()))
         {
            throw Core::ExceptStr(attr.namePos,
               "object redeclared with different type");
         }
      }

      //
      // SetDeclObjectInit (global)
      //
      static void SetDeclObjectInit(Parser &ctx, CC::Scope_Global &,
         AST::Attribute &, std::vector<AST::Statement::CRef> &,
         AST::Object *obj)
      {
         if(ctx.importing)
            return;

         obj->defin = true;

         // File-scope objects get initialization code generated later.
      }

      //
      // SetDeclObjectInit (local)
      //
      static void SetDeclObjectInit(Parser &ctx, CC::Scope_Local &,
         AST::Attribute &attr, std::vector<AST::Statement::CRef> &inits,
         AST::Object *obj)
      {
         obj->defin = true;

         // Block-scope statics must have constant initializers, so they can be
         // handled like file-scope statics.
         if(obj->store == AST::Storage::Static)
            return;

         auto initExp = CC::ExpCreate_Obj(ctx.prog, obj, attr.namePos);
         initExp = CC::Exp_Assign::Create(initExp, obj->init, obj->init->pos);

         inits.emplace_back(AST::StatementCreate_Exp(initExp));
      }
   }
}


//----------------------------------------------------------------------------|
// Extern Functions                                                           |
//

namespace GDCC
{
   namespace ACC
   {
      //
      // Parser::getDecl_Object
      //
      AST::Statement::CRef Parser::getDecl_Object(Scope_Global &scope)
      {
         return GetDeclBase_Object(*this, scope);
      }

      //
      // Parser::getDecl_Object
      //
      AST::Statement::CRef Parser::getDecl_Object(CC::Scope_Local &scope)
      {
         return GetDeclBase_Object(*this, scope);
      }
   }
}

// EOF

