//-----------------------------------------------------------------------------
//
// Copyright (C) 2016-2018 David Hill
//
// See COPYING for license information.
//
//-----------------------------------------------------------------------------
//
// Jump statement handling.
//
//-----------------------------------------------------------------------------

#include "BC/DGE/Info.hpp"

#include "IR/Exception.hpp"


//----------------------------------------------------------------------------|
// Extern Functions                                                           |
//

namespace GDCC::BC::DGE
{
   //
   // Info::chkStmnt_Jcnd_Nil
   //
   void Info::chkStmnt_Jcnd_Nil()
   {
      chkStmntArgB(1, IR::ArgBase::Lit);
   }

   //
   // Info::chkStmnt_Jcnd_Tab
   //
   void Info::chkStmnt_Jcnd_Tab()
   {
      auto n = getStmntSizeW();

      if(n != 1)
         IR::ErrorCode(stmnt, "unsupported size");

      for(Core::FastU i = 1; i != stmnt->args.size(); ++i)
         chkStmntArgB(i, IR::ArgBase::Lit);

      // Argument gets left on stack, so do not just move to stack.
      chkStmntArgB(0, IR::ArgBase::Stk);
   }

   //
   // Info::chkStmnt_Jfar
   //
   void Info::chkStmnt_Jfar()
   {
      chkStmntArgB(0, IR::ArgBase::Lit);
      chkStmntArgB(1, IR::ArgBase::Stk);

      if(stmnt->args.size() > 2)
         chkStmntArgB(2, IR::ArgBase::Sta);
   }

   //
   // Info::chkStmnt_Jset
   //
   void Info::chkStmnt_Jset()
   {
      chkStmntArgB(0, IR::ArgBase::Sta);
      chkStmntArgB(1, IR::ArgBase::Lit);
   }

   //
   // Info::putStmnt_Jcnd_Nil
   //
   void Info::putStmnt_Jcnd_Nil(char const *code)
   {
      auto n = getStmntSize();

      if(n == 0)
         return putCode("Jump_Lit", stmnt->args[1].aLit);

      for(auto i = (n + 3) / 4; --i;)
         putCode("BOrI");

      putCode(code, stmnt->args[1].aLit);
   }

   //
   // Info::putStmnt_Jcnd_Tab
   //
   void Info::putStmnt_Jcnd_Tab()
   {
      if(stmnt->args.size() == 1)
         return;

      if(stmnt->args.size() == 3)
         return putCode("Jcnd_Lit", stmnt->args[1].aLit, stmnt->args[2].aLit);

      putNTS("Jcnd_Tab");
      putNTS('(');

      // Write sorted jump cases.
      putCodeArg(stmnt->args[1].aLit); putNTS(',');
      putCodeArg(stmnt->args[2].aLit);
      for(Core::FastU i = 3, e = stmnt->args.size(); i != e;)
      {
         putNTS(','); putCodeArg(stmnt->args[i++].aLit);
         putNTS(','); putCodeArg(stmnt->args[i++].aLit);
      }

      putNTS(')');
   }

   //
   // Info::putStmnt_Jcnd_Tru
   //
   void Info::putStmnt_Jcnd_Tru()
   {
      auto n = getStmntSize();

      if(n == 0)
         return;

      putStmnt_Jcnd_Nil("Jcnd_Tru");
   }

   //
   // Info::putStmnt_Jfar
   //
   void Info::putStmnt_Jfar()
   {
      auto n = getStmntSizeW();

      if(stmnt->args.size() > 2)
      {
         // Initiate far jump.
         putStmntPushArg(stmnt->args[3], 0);
         putStmntPushIdx(stmnt->args[2], 0);
         putCode("Jfar_Sta", 1, stmnt->args[0].aLit);
      }
      else
      {
         // Check for ongoing far jump.
         putCode("Jfar_Pro", n, stmnt->args[0].aLit);
      }
   }

   //
   // Info::putStmnt_Jset
   //
   void Info::putStmnt_Jset()
   {
      // Push non-jump result.
      putCode("Push_Lit", 0);

      putStmntPushIdx(stmnt->args[0], 0);
      putCode("Jfar_Set", 1, stmnt->args[1].aLit);
   }

   //
   // Info::putStmnt_Jump
   //
   void Info::putStmnt_Jump()
   {
      if(stmnt->args[0].a == IR::ArgBase::Lit)
         putCode("Jump_Lit", stmnt->args[0].aLit);
      else
         putCode("Jump");
   }

   //
   // Info::trStmnt_Jcnd_Nil
   //
   void Info::trStmnt_Jcnd_Nil()
   {
      moveArgStk_src(stmnt->args[0]);
   }

   //
   // Info::trStmnt_Jump
   //
   void Info::trStmnt_Jump()
   {
      if(stmnt->args[0].a != IR::ArgBase::Lit)
         moveArgStk_src(stmnt->args[0]);
   }
}

// EOF

