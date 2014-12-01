//-----------------------------------------------------------------------------
//
// Copyright (C) 2014 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Call statement handling.
//
//-----------------------------------------------------------------------------

#include "Bytecode/ZDACS/Info.hpp"

#include "Bytecode/ZDACS/Code.hpp"

#include "Core/Exception.hpp"
#include "Core/Range.hpp"

#include "IR/Function.hpp"
#include "IR/CallType.hpp"


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
         // Info::genStmnt_Call
         //
         void Info::genStmnt_Call()
         {
            auto ret = stmnt->args[1].aLit.value->getValue().getFastU();

            switch(stmnt->args[0].a)
            {
            case IR::ArgBase::Lit:
               if(ret == 0)
                  numChunkCODE += 8;
               else
                  numChunkCODE += 8 + (ret - 1) * 16;

               break;

            case IR::ArgBase::Stk:
               if(ret == 0)
                  numChunkCODE += 8;
               else
                  numChunkCODE += 4 + (ret - 1) * 16;

               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad gen Call");
            }
         }

         //
         // Info::genStmnt_Cspe
         //
         void Info::genStmnt_Cspe()
         {
            auto ret = stmnt->args[1].aLit.value->getValue().getFastU();

            // No call args.
            if(stmnt->args.size() == 2)
            {
               numChunkCODE += ret ? 48 : 12;
               return;
            }

            switch(stmnt->args[2].a)
            {
            case IR::ArgBase::Lit:
               numChunkCODE += 8 + (stmnt->args.size() - 2) * (ret ? 8 : 4);
               break;

            case IR::ArgBase::Stk:
               numChunkCODE += 8;

               // Dummy args.
               if(ret) numChunkCODE += (7 - stmnt->args.size()) * 8;

               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad gen Cspr");
            }
         }

         //
         // Info::genStmnt_Retn
         //
         void Info::genStmnt_Retn()
         {
            auto argc = stmnt->args.size();

            switch(func->ctype)
            {
            case IR::CallType::StdCall:
            case IR::CallType::StkCall:
               if(argc == 0)
                  numChunkCODE += 4;
               else
                  numChunkCODE += 4 + (argc - 1) * 20;
               break;

            case IR::CallType::ScriptI:
            case IR::CallType::ScriptS:
               if(argc == 0)
                  numChunkCODE += 4;
               else
                  numChunkCODE += 8 + (argc - 1) * 20;
               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad gen Retn");
            }
         }

         //
         // Info::putStmnt_Call
         //
         void Info::putStmnt_Call()
         {
            auto ret = stmnt->args[1].aLit.value->getValue().getFastU();

            switch(stmnt->args[0].a)
            {
            case IR::ArgBase::Lit:
               if(ret == 0)
                  putCode(Code::Call_Nul);
               else
                  putCode(Code::Call_Lit);

               putWord(GetWord(stmnt->args[0].aLit));

               if(ret)
                  putStmntPushRetn(ret - 1);

               break;

            case IR::ArgBase::Stk:
               putCode(Code::Call_Stk);

               if(ret == 0)
                  putCode(Code::Drop_Nul);
               else
                  putStmntPushRetn(ret - 1);

               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad put Call");
            }
         }

         //
         // Info::putStmnt_Cspe
         //
         void Info::putStmnt_Cspe()
         {
            auto ret = stmnt->args[1].aLit.value->getValue().getFastU();

            IR::ArgBase a;
            if(stmnt->args.size() == 2)
               a = ret ? IR::ArgBase::Stk : IR::ArgBase::Lit;
            else
               a = stmnt->args[2].a;

            switch(a)
            {
            case IR::ArgBase::Lit:
               if(ret)
               {
                  for(auto const &arg : Core::MakeRange(stmnt->args.begin() + 2, stmnt->args.end()))
                  {
                     putCode(Code::Push_Lit);
                     putWord(GetWord(arg.aLit));
                  }

                  putCode(Code::Cspe_5R1, GetWord(stmnt->args[0].aLit));

                  break;
               }

               switch(stmnt->args.size() - 2)
               {
               case 0: putCode(Code::Cspe_1L); break;
               case 1: putCode(Code::Cspe_1L); break;
               case 2: putCode(Code::Cspe_2L); break;
               case 3: putCode(Code::Cspe_3L); break;
               case 4: putCode(Code::Cspe_4L); break;
               case 5: putCode(Code::Cspe_5L); break;
               }

               putWord(GetWord(stmnt->args[0].aLit));

               // Dummy arg.
               if(stmnt->args.size() == 2)
                  putWord(0);

               for(auto const &arg : Core::MakeRange(stmnt->args.begin() + 2, stmnt->args.end()))
                  putWord(GetWord(arg.aLit));

               break;

            case IR::ArgBase::Stk:
               if(ret)
               {
                  switch(stmnt->args.size() - 2)
                  {
                  case 0: putCode(Code::Push_Lit, 0);
                  case 1: putCode(Code::Push_Lit, 0);
                  case 2: putCode(Code::Push_Lit, 0);
                  case 3: putCode(Code::Push_Lit, 0);
                  case 4: putCode(Code::Push_Lit, 0);
                  case 5: putCode(Code::Cspe_5R1); break;
                  }
               }
               else
               {
                  switch(stmnt->args.size() - 2)
                  {
                  case 0: putCode(Code::Push_Lit, 0);
                  case 1: putCode(Code::Cspe_1); break;
                  case 2: putCode(Code::Cspe_2); break;
                  case 3: putCode(Code::Cspe_3); break;
                  case 4: putCode(Code::Cspe_4); break;
                  case 5: putCode(Code::Cspe_5); break;
                  }
               }

               putWord(GetWord(stmnt->args[0].aLit));

               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad put Cspe");
            }
         }

         //
         // Info::putStmnt_Retn
         //
         void Info::putStmnt_Retn()
         {
            switch(func->ctype)
            {
            case IR::CallType::StdCall:
            case IR::CallType::StkCall:
               if(stmnt->args.size() == 0)
                  putCode(Code::Retn_Nul);
               else
               {
                  putStmntDropRetn(stmnt->args.size() - 1);
                  putCode(Code::Retn_Stk);
               }
               break;

            case IR::CallType::ScriptI:
            case IR::CallType::ScriptS:
               if(stmnt->args.size() == 0)
               {
                  putCode(Code::Rscr);
               }
               else
               {
                  putStmntDropRetn(stmnt->args.size() - 1);
                  putCode(Code::Drop_ScrRet);
                  putCode(Code::Rscr);
               }
               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad put Retn");
            }
         }

         //
         // Info::trStmnt_Call
         //
         void Info::trStmnt_Call()
         {
            CheckArgC(stmnt, 2);
            CheckArgB(stmnt, 1, IR::ArgBase::Lit);
            for(auto n = stmnt->args.size(); --n != 1;)
               CheckArgB(stmnt, n, IR::ArgBase::Stk);

            switch(stmnt->args[0].a)
            {
            case IR::ArgBase::Lit:
            case IR::ArgBase::Stk:
               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad tr Call");
            }
         }

         //
         // Info::trStmnt_Cspe
         //
         void Info::trStmnt_Cspe()
         {
            CheckArgC(stmnt, 2);
            CheckArgB(stmnt, 0, IR::ArgBase::Lit);
            CheckArgB(stmnt, 1, IR::ArgBase::Lit);

            auto ret = stmnt->args[1].aLit.value->getValue().getFastU();

            if(ret > 1)
               throw Core::ExceptStr(stmnt->pos, "bad tr Cspe ret");

            // Too many call args.
            if(stmnt->args.size() > 7)
               throw Core::ExceptStr(stmnt->pos, "bad tr Cspe argc");

            // No call args.
            if(stmnt->args.size() == 2)
               return;

            switch(stmnt->args[2].a)
            {
            case IR::ArgBase::Lit:
               for(auto n = stmnt->args.size(); n-- != 3;)
                  CheckArgB(stmnt, n, IR::ArgBase::Lit);
               break;

            case IR::ArgBase::Stk:
               for(auto n = stmnt->args.size(); n-- != 3;)
                  CheckArgB(stmnt, n, IR::ArgBase::Stk);
               break;

            default:
               throw Core::ExceptStr(stmnt->pos, "bad tr Cspe");
            }
         }

         //
         // Info::trStmnt_Retn
         //
         void Info::trStmnt_Retn()
         {
            auto argc = stmnt->args.size();

            for(auto n = argc; n--;)
               CheckArgB(stmnt, n, IR::ArgBase::Stk);
         }
      }
   }
}

// EOF

