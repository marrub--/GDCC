//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2014 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Intermediary Representation expression value handling.
//
//-----------------------------------------------------------------------------

#ifndef GDCC__IR__Value_H__
#define GDCC__IR__Value_H__

#include "../IR/Type.hpp"


//----------------------------------------------------------------------------|
// Types                                                                      |
//

namespace GDCC
{
   namespace IR
   {
      #define GDCC_IR_TypeList(name) class Value_##name;
      #include "../IR/TypeList.hpp"
      class Value;

      typedef TypeBase ValueBase;

      //
      // Value_Empty
      //
      class Value_Empty
      {
      public:
         Value_Empty() = default;

         Value_Empty(Type_Empty vtype_) : vtype{vtype_} {}

         explicit Value_Empty(IArchive &in);

         explicit operator bool () const {return false;}

         Type_Empty vtype;
      };

      //
      // Value_Fixed
      //
      class Value_Fixed
      {
      public:
         Value_Fixed() = default;

         Value_Fixed(Value_Fixed const &value, Type_Fixed const &vtype);
         Value_Fixed(Value_Fixed &&value, Type_Fixed const &vtype);

         Value_Fixed(Value_Float const &value, Type_Fixed const &vtype);
         Value_Fixed(Value_Float &&value, Type_Fixed const &vtype);

         Value_Fixed(Core::Integ const &value_, Type_Fixed const &vtype_) :
            vtype{vtype_}, value{value_} {clamp();}
         Value_Fixed(Core::Integ &&value_, Type_Fixed const &vtype_) :
            vtype{vtype_}, value{std::move(value_)} {clamp();}

         explicit Value_Fixed(IArchive &in);

         explicit operator bool () const {return static_cast<bool>(value);}

         Value_Fixed &clamp() {vtype.clamp(value); return *this;}

         Core::FastU getFastU() const;

         Type_Fixed  vtype;
         Core::Integ value;
      };

      //
      // Value_Float
      //
      class Value_Float
      {
      public:
         Value_Float() = default;

         Value_Float(Value_Fixed const &value, Type_Float const &vtype);
         Value_Float(Value_Fixed &&value, Type_Float const &vtype);

         Value_Float(Value_Float const &value, Type_Float const &vtype);
         Value_Float(Value_Float &&value, Type_Float const &vtype);

         Value_Float(Core::Float const &value_, Type_Float const &vtype_) :
            vtype{vtype_}, value{value_} {clamp();}
         Value_Float(Core::Float &&value_, Type_Float const &vtype_) :
            vtype{vtype_}, value{std::move(value_)} {clamp();}

         explicit Value_Float(IArchive &in);

         explicit operator bool () const {return static_cast<bool>(value);}

         Value_Float &clamp() {return *this;}

         Core::FastU getFastU() const;

         Type_Float  vtype;
         Core::Float value;
      };

      //
      // Value_Funct
      //
      class Value_Funct
      {
      public:
         Value_Funct() = default;

         Value_Funct(Core::FastU value_, Type_Funct const &vtype_) :
            vtype{vtype_}, value{value_} {}

         explicit Value_Funct(IArchive &in);

         explicit operator bool () const;

         Type_Funct  vtype;
         Core::FastU value;
      };

      //
      // Value_Multi
      //
      class Value_Multi
      {
      public:
         Value_Multi() = default;

         Value_Multi(Core::Array<Value> const &value_, Type_Multi const &vtype_) :
            vtype{vtype_}, value{value_} {}
         Value_Multi(Core::Array<Value> const &value_, Type_Multi &&vtype_) :
            vtype{std::move(vtype_)}, value{value_} {}
         Value_Multi(Core::Array<Value> &&value_, Type_Multi const &vtype_) :
            vtype{vtype_}, value{std::move(value_)} {}
         Value_Multi(Core::Array<Value> &&value_, Type_Multi &&vtype_) :
            vtype{std::move(vtype_)}, value{std::move(value_)} {}

         explicit Value_Multi(IArchive &in);

         explicit operator bool () const {return !value.empty();}

         Type_Multi         vtype;
         Core::Array<Value> value;
      };

      //
      // Value_Point
      //
      class Value_Point
      {
      public:
         Value_Point() = default;

         Value_Point(Core::FastU value_, AddrBase addrB_, Core::String addrN_,
            Type_Point const &vtype_) :
            vtype{vtype_}, value{value_}, addrB{addrB_}, addrN{addrN_} {}

         // Converts pointer type.
         Value_Point(Value_Point const &value, Type_Point const &vtype);

         // Converts from fixed-point to pointer.
         Value_Point(Value_Fixed const &value, Type_Point const &vtype);

         explicit Value_Point(IArchive &in);

         explicit operator bool () const;

         Type_Point   vtype;
         Core::FastU  value;
         AddrBase     addrB;
         Core::String addrN;
      };

      //
      // Value_StrEn
      //
      class Value_StrEn
      {
      public:
         Value_StrEn() = default;

         Value_StrEn(Core::FastU value_, Type_StrEn vtype_) :
            vtype{vtype_}, value{value_} {}

         explicit Value_StrEn(IArchive &in);

         explicit operator bool () const;

         Type_StrEn  vtype;
         Core::FastU value;
      };

      //
      // Value
      //
      class Value
      {
      public:
         Value() : v{ValueBase::Empty}, vEmpty{} {}

         explicit Value(IArchive &in);

         //
         // copy constructor
         //
         Value(Value const &val) : v{val.v}
         {
            switch(v)
            {
               #define GDCC_IR_TypeList(name) case ValueBase::name: \
                     new(&v##name) Value_##name(val.v##name); break;
               #include "TypeList.hpp"
            }
         }

         //
         // move constructor
         //
         Value(Value &&val) : v{val.v}
         {
            switch(v)
            {
               #define GDCC_IR_TypeList(name) case ValueBase::name: \
                     new(&v##name) Value_##name(std::move(val.v##name)); break;
               #include "TypeList.hpp"
            }
         }

         //
         // cast constructors
         //
         #define GDCC_IR_TypeList(name) \
            Value(Value_##name const &val) : v{ValueBase::name}, v##name(          val ) {} \
            Value(Value_##name      &&val) : v{ValueBase::name}, v##name(std::move(val)) {}
         #include "TypeList.hpp"

         //
         // destructor
         //
         ~Value()
         {
            switch(v)
            {
               #define GDCC_IR_TypeList(name) \
                  case ValueBase::name: v##name.~Value_##name(); break;
               #include "TypeList.hpp"
            }
         }

         Value const &operator + () const {return *this;}

         //
         // operator (bool)
         //
         explicit operator bool () const
         {
            switch(v)
            {
               #define GDCC_IR_TypeList(name) \
                  case ValueBase::name: return static_cast<bool>(v##name);
               #include "TypeList.hpp"
            }

            return false;
         }

         //
         // copy assignment
         //
         Value &operator = (Value const &val)
         {
            if(val.v == v) switch(v)
            {
               #define GDCC_IR_TypeList(name) \
                  case ValueBase::name: v##name = val.v##name; break;
               #include "TypeList.hpp"
            }
            else
               new((this->~Value(), this)) Value(val);

            return *this;
         }

         //
         // move assignment
         //
         Value &operator = (Value &&val)
         {
            if(val.v == v) switch(v)
            {
               #define GDCC_IR_TypeList(name) \
                  case ValueBase::name: v##name = std::move(val.v##name); break;
               #include "TypeList.hpp"
            }
            else
               new((this->~Value(), this)) Value(std::move(val));

            return *this;
         }

         Core::FastU getFastU() const;

         //
         // getType
         //
         Type getType() const
         {
            switch(v)
            {
               #define GDCC_IR_TypeList(name) \
                  case ValueBase::name: return v##name.vtype;
               #include "TypeList.hpp"
            }

            return Type();
         }

         ValueBase v;

         union
         {
            #define GDCC_IR_TypeList(name) Value_##name v##name;
            #include "TypeList.hpp"
         };
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
      Value         operator - (Value       const &e);
      Value_Fixed   operator - (Value_Fixed const &e);
      Value_Fixed &&operator - (Value_Fixed      &&e);
      Value_Float   operator - (Value_Float const &e);
      Value_Float &&operator - (Value_Float      &&e);

      Value         operator ~ (Value       const &e);
      Value_Fixed   operator ~ (Value_Fixed const &e);
      Value_Fixed &&operator ~ (Value_Fixed      &&e);

      Value         operator * (Value       const &l, Value       const &r);
      Value_Fixed   operator * (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator * (Value_Fixed      &&l, Value_Fixed const &r);
      Value_Float   operator * (Value_Float const &l, Value_Float const &r);
      Value_Float &&operator * (Value_Float      &&l, Value_Float const &r);
      Value         operator / (Value       const &l, Value       const &r);
      Value_Fixed   operator / (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator / (Value_Fixed      &&l, Value_Fixed const &r);
      Value_Float   operator / (Value_Float const &l, Value_Float const &r);
      Value_Float &&operator / (Value_Float      &&l, Value_Float const &r);
      Value         operator % (Value       const &l, Value       const &r);
      Value_Fixed   operator % (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator % (Value_Fixed      &&l, Value_Fixed const &r);
      Value_Float   operator % (Value_Float const &l, Value_Float const &r);
      Value_Float &&operator % (Value_Float      &&l, Value_Float const &r);

      Value         operator + (Value       const &l, Value       const &r);
      Value_Fixed   operator + (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator + (Value_Fixed      &&l, Value_Fixed const &r);
      Value_Point   operator + (Value_Fixed const &l, Value_Point const &r);
      Value_Point &&operator + (Value_Fixed const &l, Value_Point      &&r);
      Value_Float   operator + (Value_Float const &l, Value_Float const &r);
      Value_Float &&operator + (Value_Float      &&l, Value_Float const &r);
      Value_Point   operator + (Value_Point const &l, Value_Fixed const &r);
      Value_Point &&operator + (Value_Point      &&l, Value_Fixed const &r);
      Value         operator - (Value       const &l, Value       const &r);
      Value_Fixed   operator - (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator - (Value_Fixed      &&l, Value_Fixed const &r);
      Value_Float   operator - (Value_Float const &l, Value_Float const &r);
      Value_Float &&operator - (Value_Float      &&l, Value_Float const &r);
      Value_Point   operator - (Value_Point const &l, Value_Fixed const &r);
      Value_Point &&operator - (Value_Point      &&l, Value_Fixed const &r);
      Value_Fixed   operator - (Value_Point const &l, Value_Point const &r);

      Value         operator << (Value       const &l, Value       const &r);
      Value_Fixed   operator << (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator << (Value_Fixed      &&l, Value_Fixed const &r);
      Value         operator >> (Value       const &l, Value       const &r);
      Value_Fixed   operator >> (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator >> (Value_Fixed      &&l, Value_Fixed const &r);

      bool operator <  (Value       const &l, Value       const &r);
      bool operator <  (Value_Fixed const &l, Value_Fixed const &r);
      bool operator <  (Value_Float const &l, Value_Float const &r);
      bool operator >  (Value       const &l, Value       const &r);
      bool operator >  (Value_Fixed const &l, Value_Fixed const &r);
      bool operator >  (Value_Float const &l, Value_Float const &r);
      bool operator <= (Value       const &l, Value       const &r);
      bool operator <= (Value_Fixed const &l, Value_Fixed const &r);
      bool operator <= (Value_Float const &l, Value_Float const &r);
      bool operator >= (Value       const &l, Value       const &r);
      bool operator >= (Value_Fixed const &l, Value_Fixed const &r);
      bool operator >= (Value_Float const &l, Value_Float const &r);

      bool operator == (Value       const &l, Value       const &r);
      bool operator == (Value_Fixed const &l, Value_Fixed const &r);
      bool operator == (Value_Float const &l, Value_Float const &r);
      bool operator != (Value       const &l, Value       const &r);
      bool operator != (Value_Fixed const &l, Value_Fixed const &r);
      bool operator != (Value_Float const &l, Value_Float const &r);

      Value         operator & (Value       const &l, Value       const &r);
      Value_Fixed   operator & (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator & (Value_Fixed      &&l, Value_Fixed const &r);

      Value         operator ^ (Value       const &l, Value       const &r);
      Value_Fixed   operator ^ (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator ^ (Value_Fixed      &&l, Value_Fixed const &r);

      Value         operator | (Value       const &l, Value       const &r);
      Value_Fixed   operator | (Value_Fixed const &l, Value_Fixed const &r);
      Value_Fixed &&operator | (Value_Fixed      &&l, Value_Fixed const &r);

      Value       &operator *=  (Value       &l, Value       const &r);
      Value_Fixed &operator *=  (Value_Fixed &l, Value_Fixed const &r);
      Value_Float &operator *=  (Value_Float &l, Value_Float const &r);
      Value       &operator /=  (Value       &l, Value       const &r);
      Value_Fixed &operator /=  (Value_Fixed &l, Value_Fixed const &r);
      Value_Float &operator /=  (Value_Float &l, Value_Float const &r);
      Value       &operator %=  (Value       &l, Value       const &r);
      Value_Fixed &operator %=  (Value_Fixed &l, Value_Fixed const &r);
      Value_Float &operator %=  (Value_Float &l, Value_Float const &r);
      Value       &operator +=  (Value       &l, Value       const &r);
      Value_Fixed &operator +=  (Value_Fixed &l, Value_Fixed const &r);
      Value_Float &operator +=  (Value_Float &l, Value_Float const &r);
      Value_Point &operator +=  (Value_Point &l, Value_Fixed const &r);
      Value       &operator -=  (Value       &l, Value       const &r);
      Value_Fixed &operator -=  (Value_Fixed &l, Value_Fixed const &r);
      Value_Float &operator -=  (Value_Float &l, Value_Float const &r);
      Value_Point &operator -=  (Value_Point &l, Value_Fixed const &r);
      Value       &operator <<= (Value       &l, Value       const &r);
      Value_Fixed &operator <<= (Value_Fixed &l, Value_Fixed const &r);
      Value       &operator >>= (Value       &l, Value       const &r);
      Value_Fixed &operator >>= (Value_Fixed &l, Value_Fixed const &r);
      Value       &operator &=  (Value       &l, Value       const &r);
      Value_Fixed &operator &=  (Value_Fixed &l, Value_Fixed const &r);
      Value       &operator ^=  (Value       &l, Value       const &r);
      Value_Fixed &operator ^=  (Value_Fixed &l, Value_Fixed const &r);
      Value       &operator |=  (Value       &l, Value       const &r);
      Value_Fixed &operator |=  (Value_Fixed &l, Value_Fixed const &r);

      OArchive &operator << (OArchive &out, ValueBase in);

      #define GDCC_IR_TypeList(name) \
         OArchive &operator << (OArchive &out, Value_##name const &in);
      #include "TypeList.hpp"

      OArchive &operator << (OArchive &out, Value const &in);
   }
}

#endif//GDCC__IR__Value_H__

