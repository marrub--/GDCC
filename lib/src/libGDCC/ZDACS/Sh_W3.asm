;;-----------------------------------------------------------------------------
;;
;; Copyright(C) 2014 David Hill
;;
;; See COPYLIB for license information.
;;
;;-----------------------------------------------------------------------------
;;
;; 3-word shift routines for ZDACS.
;;
;;-----------------------------------------------------------------------------


;;----------------------------------------------------------------------------|
;; Functions                                                                  |
;;

;;
;; ___GDCC__ShLU_W3
;;
Function "___GDCC__ShLU_W3" \
   alloc    = 1 \
   ctype    = StkCall \
   defin    = 1 \
   label    = "___GDCC__ShLU_W3$label" \
   linka    = ExtC \
   localReg = 4 \
   param    = 4 \
   retrn    = 3 \
   block
{
   Move_W,    Stk(), LocReg(Lit(3), 0)
   Casm,      Lit(84), Lit(0),  Lit("___GDCC__ShLU_W3$0")  ; Cjmp_Lit
   Casm,      Lit(84), Lit(32), Lit("___GDCC__ShLU_W3$32") ; Cjmp_Lit
   Casm,      Lit(84), Lit(64), Lit("___GDCC__ShLU_W3$64") ; Cjmp_Lit
   CmpI_LT_W, Stk(), Stk(), Lit(32)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShLU_W3$gt32")

   ; ret0 = l0 << r
   ShLU_W, Stk(), LocReg(Lit(0), 0), LocReg(Lit(3), 0)

   ; ret1 = (l1 << r) | (l0 >> (32 - r))
   ShLU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(0), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShRU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret2 = (l2 << r) | (l1 >> (32 - r))
   ShLU_W, Stk(), LocReg(Lit(2), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShRU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShLU_W3$gt32"
   CmpI_LT_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShLU_W3$gt64")

   ; r -= 32
   SubU_W, LocReg(Lit(3), 0), LocReg(Lit(3), 0), Lit(32)

   ; ret0 = 0
   Move_W, Stk(), Lit(0)

   ; ret1 = l0 << r
   ShLU_W, Stk(), LocReg(Lit(0), 0), LocReg(Lit(3), 0)

   ; ret2 = (l1 << r) | (l0 >> (32 - r))
   ShLU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(0), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShRU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShLU_W3$gt64"
   ; ret0 = 0
   Move_W, Stk(), Lit(0)

   ; ret1 = 0
   Move_W, Stk(), Lit(0)

   ; ret2 = l0 << (r - 64)
   Move_W, Stk(), LocReg(Lit(0), 0)
   SubU_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   ShLU_W, Stk(), Stk(), Stk()

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShLU_W3$0"
   Move_W, Stk(), LocReg(Lit(0), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   Retn,   Stk(), Stk()

"___GDCC__ShLU_W3$32"
   Move_W, Stk(), Lit(0)
   Move_W, Stk(), LocReg(Lit(0), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShLU_W3$64"
   Move_W, Stk(), Lit(0)
   Move_W, Stk(), Lit(0)
   Move_W, Stk(), LocReg(Lit(0), 0)
   Retn,   Stk(), Stk(), Stk()
}

;;
;; ___GDCC__ShRI_W3
;;
Function "___GDCC__ShRI_W3" \
   alloc    = 1 \
   ctype    = StkCall \
   defin    = 1 \
   label    = "___GDCC__ShRI_W3$label" \
   linka    = ExtC \
   localReg = 4 \
   param    = 4 \
   retrn    = 3 \
   block
{
   Move_W,    Stk(), LocReg(Lit(3), 0)
   Casm,      Lit(84), Lit(0),  Lit("___GDCC__ShRI_W3$0")  ; Cjmp_Lit
   Casm,      Lit(84), Lit(32), Lit("___GDCC__ShRI_W3$32") ; Cjmp_Lit
   Casm,      Lit(84), Lit(64), Lit("___GDCC__ShRI_W3$64") ; Cjmp_Lit
   CmpI_LT_W, Stk(), Stk(), Lit(32)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShRI_W3$gt32")

   ; ret0 = (l0 >> r) | (l1 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(0), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret1 = (l1 >> r) | (l2 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret2 = l2 >> r
   ShRI_W, Stk(), LocReg(Lit(2), 0), LocReg(Lit(3), 0)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRI_W3$gt32"
   CmpI_LT_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShRI_W3$gt64")

   ; r -= 32
   SubU_W, LocReg(Lit(3), 0), LocReg(Lit(3), 0), Lit(32)

   ; ret0 = (l1 >> r) | (l2 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret1 = l2 >> r
   ShRI_W, Stk(), LocReg(Lit(2), 0), LocReg(Lit(3), 0)

   ; ret2 = l2 < 0 ? -1 : 0
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRI_W3$gt64"
   ; ret0 = l2 >> (r - 64)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   ShRI_W, Stk(), Stk(), Stk()

   ; ret1 = l2 < 0 ? -1 : 0
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)

   ; ret2 = l2 < 0 ? -1 : 0
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRI_W3$0"
   Move_W, Stk(), LocReg(Lit(0), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRI_W3$32"
   Move_W, Stk(), LocReg(Lit(1), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)
   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRI_W3$64"
   Move_W, Stk(), LocReg(Lit(2), 0)
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)
   ShRI_W, Stk(), LocReg(Lit(2), 0), Lit(31)
   Retn,   Stk(), Stk(), Stk()
}

;;
;; ___GDCC__ShRU_W3
;;
Function "___GDCC__ShRU_W3" \
   alloc    = 1 \
   ctype    = StkCall \
   defin    = 1 \
   label    = "___GDCC__ShRU_W3$label" \
   linka    = ExtC \
   localReg = 4 \
   param    = 4 \
   retrn    = 3 \
   block
{
   Move_W,    Stk(), LocReg(Lit(3), 0)
   Casm,      Lit(84), Lit(0),  Lit("___GDCC__ShRU_W3$0")  ; Cjmp_Lit
   Casm,      Lit(84), Lit(32), Lit("___GDCC__ShRU_W3$32") ; Cjmp_Lit
   Casm,      Lit(84), Lit(64), Lit("___GDCC__ShRU_W3$64") ; Cjmp_Lit
   CmpI_LT_W, Stk(), Stk(), Lit(32)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShRU_W3$gt32")

   ; ret0 = (l0 >> r) | (l1 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(0), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret1 = (l1 >> r) | (l2 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret2 = l2 >> r
   ShRU_W, Stk(), LocReg(Lit(2), 0), LocReg(Lit(3), 0)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRU_W3$gt32"
   CmpI_LT_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   Cjmp_Nil,  Stk(), Lit("___GDCC__ShRU_W3$gt64")

   ; r -= 32
   SubU_W, LocReg(Lit(3), 0), LocReg(Lit(3), 0), Lit(32)

   ; ret0 = (l1 >> r) | (l2 << (32 - r))
   ShRU_W, Stk(), LocReg(Lit(1), 0), LocReg(Lit(3), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), Lit(32), LocReg(Lit(3), 0)
   ShLU_W, Stk(), Stk(), Stk()
   OrIU_W, Stk(), Stk(), Stk()

   ; ret1 = l2 >> r
   ShRU_W, Stk(), LocReg(Lit(2), 0), LocReg(Lit(3), 0)

   ; ret2 = 0
   Move_W, Stk(), Lit(0)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRU_W3$gt64"
   ; ret0 = l2 >> (r - 64)
   Move_W, Stk(), LocReg(Lit(2), 0)
   SubU_W, Stk(), LocReg(Lit(3), 0), Lit(64)
   ShRU_W, Stk(), Stk(), Stk()

   ; ret1 = 0
   Move_W, Stk(), Lit(0)

   ; ret2 = 0
   Move_W, Stk(), Lit(0)

   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRU_W3$0"
   Move_W, Stk(), LocReg(Lit(0), 0)
   Move_W, Stk(), LocReg(Lit(1), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   Retn,   Stk(), Stk()

"___GDCC__ShRU_W3$32"
   Move_W, Stk(), LocReg(Lit(1), 0)
   Move_W, Stk(), LocReg(Lit(2), 0)
   Move_W, Stk(), Lit(0)
   Retn,   Stk(), Stk(), Stk()

"___GDCC__ShRU_W3$64"
   Move_W, Stk(), LocReg(Lit(2), 0)
   Move_W, Stk(), Lit(0)
   Move_W, Stk(), Lit(0)
   Retn,   Stk(), Stk(), Stk()
}

;; EOF

