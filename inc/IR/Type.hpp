//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2014 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Intermediary Representation expression type handling.
//
//-----------------------------------------------------------------------------

#ifndef GDCC__IR__Type_H__
#define GDCC__IR__Type_H__

#include "../Core/Array.hpp"
#include "../Core/Number.hpp"
#include "../Core/String.hpp"

#include <memory>


//----------------------------------------------------------------------------|
// Types                                                                      |
//

namespace GDCC
{
   namespace IR
   {
      enum class AddrBase;
      enum class CallType;
      class IArchive;
      class OArchive;
      class Type;

      //
      // TypeBase
      //
      enum class TypeBase
      {
         #define GDCC_IR_TypeList(name) name,
         #include "../IR/TypeList.hpp"
      };

      //
      // TypeError
      //
      class TypeError : public std::exception
      {
      public:
         virtual char const *what() const noexcept {return "TypeError";}
      };

      //
      // Type_Empty
      //
      class Type_Empty
      {
      public:
         Type_Empty() = default;

         explicit Type_Empty(IArchive &in);

         bool operator == (Type_Empty const &) const {return true;}
      };

      //
      // Type_Fixed
      //
      class Type_Fixed
      {
      public:
         Type_Fixed() = default;

         Type_Fixed(Core::FastU bitsI_, Core::FastU bitsF_, bool bitsS_, bool satur_) :
            bitsI{bitsI_}, bitsF{bitsF_}, bitsS{bitsS_}, satur{satur_} {}

         explicit Type_Fixed(IArchive &in);

         Core::Integ &clamp(Core::Integ &value);

         Core::FastU getBits() const {return bitsI + bitsF + bitsS;}

         bool operator == (Type_Fixed const &t) const
            {return bitsI == t.bitsI && bitsF == t.bitsF &&
                    bitsS == t.bitsS && satur == t.satur;}

         Core::FastU bitsI;
         Core::FastU bitsF;
         bool        bitsS : 1;
         bool        satur : 1;


         static Type_Fixed Promote(Type_Fixed const &l, Type_Fixed const &r);
      };

      //
      // Type_Float
      //
      class Type_Float
      {
      public:
         Type_Float() = default;

         Type_Float(Core::FastU bitsI_, Core::FastU bitsF_, bool bitsS_, bool satur_) :
            bitsI{bitsI_}, bitsF{bitsF_}, bitsS{bitsS_}, satur{satur_} {}

         explicit Type_Float(IArchive &in);

         bool operator == (Type_Float const &t) const
            {return bitsI == t.bitsI && bitsF == t.bitsF &&
                    bitsS == t.bitsS && satur == t.satur;}

         Core::FastU bitsI;
         Core::FastU bitsF;
         bool        bitsS : 1;
         bool        satur : 1;


         static Type_Float Promote(Type_Float const &l, Type_Float const &r);
      };

      //
      // Type_Funct
      //
      class Type_Funct
      {
      public:
         Type_Funct() = default;

         explicit Type_Funct(CallType callT_) : callT{callT_} {}

         explicit Type_Funct(IArchive &in);

         bool operator == (Type_Funct const &t) const {return callT == t.callT;}

         CallType callT;
      };

      //
      // Type_Multi
      //
      class Type_Multi
      {
      public:
         Type_Multi() = default;

         Type_Multi(Core::Array<Type> const &types_) : types{types_} {}
         Type_Multi(Core::Array<Type> &&types_) : types{std::move(types_)} {}

         explicit Type_Multi(IArchive &in);

         bool operator == (Type_Multi const &t) const {return types == t.types;}

         Core::Array<Type> types;
      };

      //
      // Type_Point
      //
      class Type_Point
      {
      public:
         Type_Point() = default;

         Type_Point(AddrBase reprB_, Core::FastU reprS_, Core::FastU reprW_) :
            reprB{reprB_}, reprS{reprS_}, reprW{reprW_} {}
         explicit Type_Point(IArchive &in);

         bool operator == (Type_Point const &t) const
            {return reprB == t.reprB && reprS == t.reprS && reprW == t.reprW;}

         AddrBase    reprB; // Base
         Core::FastU reprS; // Size in Words
         Core::FastU reprW; // Size of Word
      };

      //
      // Type_StrEn
      //
      class Type_StrEn
      {
      public:
         Type_StrEn() = default;

         explicit Type_StrEn(IArchive &in);

         bool operator == (Type_StrEn const &) const {return true;}
      };

      //
      // Type
      //
      class Type
      {
      public:
         Type() : t{TypeBase::Empty}, tEmpty{} {}

         //
         // copy constructor
         //
         Type(Type const &type) : t{type.t}
         {
            switch(t)
            {
               #define GDCC_IR_TypeList(name) case TypeBase::name: \
                     new(&t##name) Type_##name(type.t##name); break;
               #include "../IR/TypeList.hpp"
            }
         }

         //
         // move constructor
         //
         Type(Type &&type) : t{type.t}
         {
            switch(t)
            {
               #define GDCC_IR_TypeList(name) case TypeBase::name: \
                     new(&t##name) Type_##name(std::move(type.t##name)); break;
               #include "../IR/TypeList.hpp"
            }
         }

         //
         // cast constructors
         //
         #define GDCC_IR_TypeList(name) \
            Type(Type_##name const &type) : t{TypeBase::name}, t##name(          type ) {} \
            Type(Type_##name      &&type) : t{TypeBase::name}, t##name(std::move(type)) {}
         #include "../IR/TypeList.hpp"

         explicit Type(IArchive &in);

         //
         // destructor
         //
         ~Type()
         {
            switch(t)
            {
               #define GDCC_IR_TypeList(name) \
                  case TypeBase::name: t##name.~Type_##name(); break;
               #include "../IR/TypeList.hpp"
            }
         }

         explicit operator bool () const {return t != TypeBase::Empty;}

         //
         // operator Type == Type
         //
         bool operator == (Type const &type) const
         {
            if(type.t == t) switch(t)
            {
               #define GDCC_IR_TypeList(name) \
                  case TypeBase::name: return t##name == type.t##name;
               #include "../IR/TypeList.hpp"
            }

            return false;
         }

         // operator Type != Type
         bool operator != (Type const &type) const {return !(*this == type);}

         //
         // copy assignment
         //
         Type &operator = (Type const &type)
         {
            if(type.t == t) switch(t)
            {
               #define GDCC_IR_TypeList(name) \
                  case TypeBase::name: t##name = type.t##name; break;
               #include "../IR/TypeList.hpp"
            }
            else
               new((this->~Type(), this)) Type(type);

            return *this;
         }

         //
         // move assignment
         //
         Type &operator = (Type &&type)
         {
            if(type.t == t) switch(t)
            {
               #define GDCC_IR_TypeList(name) \
                  case TypeBase::name: t##name = std::move(type.t##name); break;
               #include "../IR/TypeList.hpp"
            }
            else
               new((this->~Type(), this)) Type(type);

            return *this;
         }

         TypeBase t;

         union
         {
            #define GDCC_IR_TypeList(name) Type_##name t##name;
            #include "../IR/TypeList.hpp"
         };


         static Type PromoteAdd(Type const &l, Type const &r);
         static Type PromoteBitAnd(Type const &l, Type const &r);
         static Type PromoteBitOrI(Type const &l, Type const &r);
         static Type PromoteBitOrX(Type const &l, Type const &r);
         static Type PromoteDiv(Type const &l, Type const &r);
         static Type PromoteMod(Type const &l, Type const &r);
         static Type PromoteMul(Type const &l, Type const &r);
         static Type PromoteShL(Type const &l, Type const &r);
         static Type PromoteShR(Type const &l, Type const &r);
         static Type PromoteSub(Type const &l, Type const &r);
      };
   }
}


//----------------------------------------------------------------------------|
// Global Functions                                                           |
//

namespace GDCC
{
   namespace IR
   {
      OArchive &operator << (OArchive &out, TypeBase in);

      #define GDCC_IR_TypeList(name) \
         OArchive &operator << (OArchive &out, Type_##name const &in);
      #include "../IR/TypeList.hpp"

      OArchive &operator << (OArchive &out, Type const &in);

      std::ostream &operator << (std::ostream &out, TypeBase in);

      IArchive &operator >> (IArchive &in, TypeBase &out);

      #define GDCC_IR_TypeList(name) \
         IArchive &operator >> (IArchive &in, Type_##name &out);
      #include "../IR/TypeList.hpp"

      IArchive &operator >> (IArchive &in, Type &out);
   }
}

#endif//GDCC__IR__Type_H__

