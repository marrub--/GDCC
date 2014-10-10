//-----------------------------------------------------------------------------
//
// Copyright (C) 2013-2014 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// ZDoom ACS target information and handling class.
//
//-----------------------------------------------------------------------------

#include "Bytecode/ZDACS/Info.hpp"

#include "Core/Option.hpp"

#include "IR/CallType.hpp"
#include "IR/Program.hpp"

#include "Option/Bool.hpp"
#include "Option/Exception.hpp"
#include "Option/Int.hpp"

#include <iostream>


//----------------------------------------------------------------------------|
// Options                                                                    |
//

//
// --bc-zdacs-LocArs-array
//
static GDCC::Option::Int<GDCC::Core::FastU> LocArsArrayOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-LocArs-array")
      .setGroup("codegen")
      .setDescS("Sets the global array used for LocArs."),

   &GDCC::Bytecode::ZDACS::Info::LocArsArray
};

//
// --bc-zdacs-chunk-STRE
//
static GDCC::Option::Bool ChunkSTREOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-chunk-STRE")
      .setGroup("output")
      .setDescS("Generates an encrypted string table."),

   &GDCC::Bytecode::ZDACS::Info::UseChunkSTRE
};

//
// --bc-zdacs-fake-ACS0
//
static GDCC::Option::Bool FakeACS0Opt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-fake-ACS0")
      .setGroup("output")
      .setDescS("Generates a fake ACS0 header."),

   &GDCC::Bytecode::ZDACS::Info::UseFakeACS0
};

//
// --bc-zdacs-init-gbl-array
//
static GDCC::Core::FastU InitGblArray = 0;
static GDCC::Option::Int<GDCC::Core::FastU> InitGblArrayOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-init-gbl-array")
      .setGroup("codegen")
      .setDescS("Sets the global array used to store init status.")
      .setDescL(
         "Sets the global array used to store initialization status. "
         "Default is to use the LocArs array."),

   &InitGblArray
};

//
// --bc-zdacs-init-gbl-index
//
static GDCC::Core::FastU InitGblIndex = 0;
static GDCC::Option::Int<GDCC::Core::FastU> InitGblIndexOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-init-gbl-index")
      .setGroup("codegen")
      .setDescS("Sets the global array index used to store init status.")
      .setDescL(
         "Sets the global array index used to store initialization status. "
         "Default is 0 if using the LocArs array, -1 otherwise."),

   &InitGblIndex
};

//
// --bc-zdacs-init-script-number
//
static GDCC::Option::Int<GDCC::Core::FastU> InitScriptNumberOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-init-script-number")
      .setGroup("codegen")
      .setDescS("Sets the number for the init script.")
      .setDescL(
         "Sets the script number for the initialization script. "
         "Default is 999."),

   &GDCC::Bytecode::ZDACS::Info::InitScriptNumber
};

//
// --bc-zdacs-init-wld-array
//
static GDCC::Core::FastU InitWldArray = 0;
static GDCC::Option::Int<GDCC::Core::FastU> InitWldArrayOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-init-wld-array")
      .setGroup("codegen")
      .setDescS("Sets the world array used to store init status.")
      .setDescL(
         "Sets the world array used to store initialization status. "
         "Default is to use any world array with an initializer."),

   &InitWldArray
};

//
// --bc-zdacs-init-wld-index
//
static GDCC::Core::FastU InitWldIndex = 0xFFFFFFFF;
static GDCC::Option::Int<GDCC::Core::FastU> InitWldIndexOpt
{
   &GDCC::Core::GetOptionList(), GDCC::Option::Base::Info()
      .setName("bc-zdacs-init-wld-index")
      .setGroup("codegen")
      .setDescS("Sets the world array index used to store init status.")
      .setDescL(
         "Sets the world array index used to store initialization status. "
         "Default is -1."),

   &InitWldIndex
};


//----------------------------------------------------------------------------|
// Global Variables                                                           |
//

namespace GDCC
{
   namespace Bytecode
   {
      namespace ZDACS
      {
         IR::Type_Fixed const Info::TypeWord{32, 0, false, false};

         Core::FastU Info::InitScriptNumber = 999;

         Core::FastU Info::LocArsArray = 0;

         bool Info::UseChunkSTRE = false;
         bool Info::UseFakeACS0  = false;
      }
   }
}


//----------------------------------------------------------------------------|
// Global Functions                                                           |
//

namespace GDCC
{
   namespace Bytecode
   {
      namespace ZDACS
      {
         //
         // Info default constructor
         //
         Info::Info() :
            codeInit   {0},
            codeInitEnd{0},

            numChunkAIMP{0},
            numChunkAINI{0},
            numChunkARAY{0},
            numChunkASTR{0},
            numChunkATAG{0},
            numChunkCODE{0},
            numChunkFNAM{0},
            numChunkFUNC{0},
            numChunkLOAD{0},
            numChunkMEXP{0},
            numChunkMIMP{0},
            numChunkMINI{0},
            numChunkMSTR{0},
            numChunkSFLG{0},
            numChunkSNAM{0},
            numChunkSPTR{0},
            numChunkSTRL{0},
            numChunkSVCT{0}
         {
         }

         //
         // Info::backGlyphFunc
         //
         void Info::backGlyphFunc(Core::String glyph, Core::FastU val,
            IR::CallType ctype)
         {
            auto &data = prog->getGlyphData(glyph);

            data.type  = IR::Type_Funct(ctype);
            data.value = IR::ExpCreate_Value(
               IR::Value_Funct(val, IR::Type_Funct(ctype)),
               Core::Origin(Core::STRNULL, 0));
         }

         //
         // Info::backGlyphGlyph
         //
         void Info::backGlyphGlyph(Core::String glyph, Core::String val)
         {
            auto &data = prog->getGlyphData(glyph);

            data.type  = prog->getGlyphData(val).type;
            data.value = IR::ExpCreate_Glyph(
               IR::Glyph(prog, val),
               Core::Origin(Core::STRNULL, 0));
         }

         //
         // Info::backGlyphObj
         //
         void Info::backGlyphObj(Core::String glyph, Core::FastU val)
         {
            auto &data = prog->getGlyphData(glyph);

            data.value = IR::ExpCreate_Value(IR::Value_Point(val,
               data.type.tPoint.reprB, data.type.tPoint.reprN, data.type.tPoint), {nullptr, 0});
         }

         //
         // Info::backGlyphStr
         //
         void Info::backGlyphStr(Core::String glyph, Core::FastU val)
         {
            auto &data = prog->getGlyphData(glyph);

            data.type  = IR::Type_StrEn();
            data.value = IR::ExpCreate_Value(
               IR::Value_StrEn(val, IR::Type_StrEn()),
               Core::Origin(Core::STRNULL, 0));
         }

         //
         // Info::backGlyphWord
         //
         void Info::backGlyphWord(Core::String glyph, Core::FastU val)
         {
            auto &data = prog->getGlyphData(glyph);

            data.type  = TypeWord;
            data.value = IR::ExpCreate_Value(
               IR::Value_Fixed(val, TypeWord), Core::Origin(Core::STRNULL, 0));
         }

         //
         // Info::getInitGblArray
         //
         Core::FastU Info::getInitGblArray()
         {
            if(InitGblArrayOpt.processed)
               return InitGblArray;

            return LocArsArray;
         }

         //
         // Info::getInitGblIndex
         //
         Core::FastU Info::getInitGblIndex()
         {
            if(InitGblIndexOpt.processed)
               return InitGblIndex;

            if(!InitGblArrayOpt.processed || InitGblArray == LocArsArray)
               return 0;

            return -1;
         }

         //
         // Info::getInitWldArray
         //
         Core::FastU Info::getInitWldArray()
         {
            if(InitWldArrayOpt.processed)
               return InitWldArray;

            for(auto const &itr : prog->rangeSpaceWldArs())
            {
               if(!init[&itr].vals.empty())
                  return itr.value;
            }

            return 0;
         }

         //
         // Info::getInitWldIndex
         //
         Core::FastU Info::getInitWldIndex()
         {
            return InitWldIndex;
         }

         //
         // Info::getStkPtrIdx
         //
         Core::FastU Info::getStkPtrIdx()
         {
            switch(func->ctype)
            {
            case IR::CallType::StdCall:
               return 0;

            case IR::CallType::ScriptI:
            case IR::CallType::ScriptS:
               return func->localReg - 1;

            default:
               std::cerr << "ERROR: " << stmnt->pos << ": bad getStkPtrIdx\n";
               throw EXIT_FAILURE;
            }
         }

         //
         // Info::isInitiGblArr
         //
         bool Info::isInitiGblArr()
         {
            for(auto const &itr : prog->rangeSpaceGblArs())
               if(!init[&itr].vals.empty()) return true;

            return !init[&prog->getSpaceLocArs()].vals.empty();
         }

         //
         // Info::isInitiWldArr
         //
         bool Info::isInitiWldArr()
         {
            for(auto const &itr : prog->rangeSpaceWldArs())
               if(!init[&itr].vals.empty()) return true;

            return false;
         }

         //
         // Info::lenString
         //
         std::size_t Info::lenString(Core::String s)
         {
            std::size_t len = 0;

            for(auto i = s.begin(), e = s.end(); i != e; ++i) switch(*i)
            {
            case '\0':
               if('0' <= i[1] && i[1] <= '7')
                  len += 4;
               else
                  len += 2;
               break;

            case '\\':
               len += 2;
               break;

            default:
               len += 1;
               break;
            }

            return len + 1;
         }

         //
         // Info::resolveGlyph
         //
         IR::Exp::CRef Info::resolveGlyph(Core::String glyph)
         {
            if(auto exp = prog->getGlyphData(glyph).value)
               return static_cast<IR::Exp::CRef>(exp);

            std::cerr << "ERROR: unbacked glyph: '" << glyph << "'\n";
            throw EXIT_FAILURE;
         }

         //
         // Info::CheckArg
         //
         void Info::CheckArg(IR::Arg const &arg, Core::Origin pos)
         {
            switch(arg.a)
            {
            case IR::ArgBase::GblArr:
               CheckArgB(*arg.aGblArr.arr, IR::ArgBase::Lit, pos);
               CheckArg (*arg.aGblArr.idx, pos);
               break;

            case IR::ArgBase::GblReg:
               CheckArgB(*arg.aGblReg.idx, IR::ArgBase::Lit, pos);
               break;

            case IR::ArgBase::Lit: break;
            case IR::ArgBase::Nul: break;
            case IR::ArgBase::Stk: break;

            case IR::ArgBase::Loc:
               CheckArg(*arg.aLoc.idx, pos);
               break;

            case IR::ArgBase::LocArs:
               CheckArg(*arg.aLocArs.idx, pos);
               break;

            case IR::ArgBase::LocReg:
               CheckArgB(*arg.aLocReg.idx, IR::ArgBase::Lit, pos);
               break;

            case IR::ArgBase::MapArr:
               CheckArgB(*arg.aMapArr.arr, IR::ArgBase::Lit, pos);
               CheckArg (*arg.aMapArr.idx, pos);
               break;

            case IR::ArgBase::MapReg:
               CheckArgB(*arg.aMapReg.idx, IR::ArgBase::Lit, pos);
               break;

            case IR::ArgBase::WldArr:
               CheckArgB(*arg.aWldArr.arr, IR::ArgBase::Lit, pos);
               CheckArg (*arg.aWldArr.idx, pos);
               break;

            case IR::ArgBase::WldReg:
               CheckArgB(*arg.aWldReg.idx, IR::ArgBase::Lit, pos);
               break;

            default:
               std::cerr << "ERROR: " << pos << ": ZDACS cannot use Arg: "
                  << arg.a << '\n';
               throw EXIT_FAILURE;
            }
         }

         //
         // Info::CodeBase
         //
         Core::FastU Info::CodeBase()
         {
            return 24;
         }

         //
         // Info::GetScriptValue
         //
         Core::FastU Info::GetScriptValue(IR::Function const &script)
         {
            if(script.ctype == IR::CallType::ScriptS)
               return -static_cast<Core::FastI>(script.valueInt) - 1;
            else
               return script.valueInt;
         }

         //
         // Info::ResolveValue
         //
         Core::FastU Info::ResolveValue(IR::Value const &val)
         {
            switch(val.v)
            {
            case IR::ValueBase::Fixed:
               if(val.vFixed.vtype.bitsS)
                  return Core::NumberCast<Core::FastI>(val.vFixed.value);
               else
                  return Core::NumberCast<Core::FastU>(val.vFixed.value);

            case IR::ValueBase::Funct:
               return val.vFunct.value;

            case IR::ValueBase::Point:
               return val.vPoint.value;

            case IR::ValueBase::StrEn:
               return val.vFunct.value;

            default:
               std::cerr << "ERROR: bad Value\n";
               throw EXIT_FAILURE;
            }
         }
      }
   }
}

// EOF

