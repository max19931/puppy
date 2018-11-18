// To compile your code for 80186/80188/80286 include this file
// at the top of every C file.
//
// Keep in mind that Smaller C's standard library uses some 80386
// instructions and is not 80186/80188/80286-compatible unless
// adjusted for 80186/80188/80286 as well.
//
// This file defines NASM macros (that is, not for YASM or FASM)
// for the following 80386 instructions generated by Smaller C:
//   movzx r16, r/m8
//   movsx r16, r/m8
//   setcc r/m8
// Note that segment prefixes are not (yet?) supported with the
// movzx/movsx macros, e.g.:
//   movzx di, byte [es:si]
// But the compiler does not generate such movzx/movsx instructions.
//
// These macros are not the most efficient, but they do not clobber
// flags or other registers.
//
// This is only for the tiny and small memory models of Smaller C
// (i.e. for use with options: -tiny/-dost, -small/-doss).
// Other models use 32-bit registers unavailable on any x86 CPU
// earlier than 80386.
asm(
"cpu 186                                                               \n"
"                                                                      \n"
"%define ___ReGs___Lo8___ \"alblcldl\"                                 \n"
"%define ___ReGs___Hi8___ \"ahbhchdh\"                                 \n"
"%define ___ReGs___16___ \"axbxcxdxsidibpsp\"                          \n"
"                                                                      \n"
"%macro movzx 2.nolist                                                 \n"
"  %push __CTX_MOVZX__                                                 \n"
"                                                                      \n"
"  %assign %$r_n 8 ; not 8-bit register                                \n"
"  %ifidni %2,al                                                       \n"
"    %assign %$r_n 0                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,bl                                                       \n"
"    %assign %$r_n 1                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,cl                                                       \n"
"    %assign %$r_n 2                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,dl                                                       \n"
"    %assign %$r_n 3                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,ah                                                       \n"
"    %assign %$r_n 4                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,bh                                                       \n"
"    %assign %$r_n 5                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,ch                                                       \n"
"    %assign %$r_n 6                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,dh                                                       \n"
"    %assign %$r_n 7                                                   \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %assign %$l_n 8 ; not 16-bit register                               \n"
"  %ifidni %1,ax                                                       \n"
"    %assign %$l_n 0                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,bx                                                       \n"
"    %assign %$l_n 1                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,cx                                                       \n"
"    %assign %$l_n 2                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,dx                                                       \n"
"    %assign %$l_n 3                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,si                                                       \n"
"    %assign %$l_n 4                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,di                                                       \n"
"    %assign %$l_n 5                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,bp                                                       \n"
"    %assign %$l_n 6                                                   \n"
"  %endif                                                              \n"
"  ;%ifidni %1,sp                                                      \n"
"  ;  %assign %$l_n 7                                                  \n"
"  ;%endif                                                             \n"
"                                                                      \n"
"  %if %$l_n == 8                                                      \n"
"    %fatal expected word register (other than sp) in left operand     \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %if %$r_n != 8                                                      \n"
"    ; right operand is register                                       \n"
"    %if %$l_n < 4                                                     \n"
"      ; movzx hi_lo, reg                                              \n"
"      %if %$l_n != %$r_n                                              \n"
"        %substr %$reg_lo_str ___ReGs___Lo8___ (%$l_n*2+1), 2          \n"
"                           ; 2-char string with reg name, e.g. \"al\" \n"
"        %deftok %$reg_lo %$reg_lo_str ; tokenized to e.g. al          \n"
"        mov %$reg_lo, %2                                              \n"
"      %endif                                                          \n"
"      %substr %$reg_hi_str ___ReGs___Hi8___ (%$l_n*2+1), 2            \n"
"      %deftok %$reg_hi %$reg_hi_str                                   \n"
"      mov %$reg_hi, 0                                                 \n"
"    %else                                                             \n"
"      ; movzx non-hi_lo, reg                                          \n"
"      mov %1, ax                                                      \n"
"      %if %$r_n != 0                                                  \n"
"        mov al, %2                                                    \n"
"      %endif                                                          \n"
"      mov ah, 0                                                       \n"
"      xchg ax, %1                                                     \n"
"    %endif                                                            \n"
"  %else                                                               \n"
"    ; right operand is memory                                         \n"
"    %if %$l_n < 4                                                     \n"
"      ; movzx hi_lo, mem                                              \n"
"      %substr %$reg_lo_str ___ReGs___Lo8___ (%$l_n*2+1), 2            \n"
"      %deftok %$reg_lo %$reg_lo_str                                   \n"
"      %substr %$reg_hi_str ___ReGs___Hi8___ (%$l_n*2+1), 2            \n"
"      %deftok %$reg_hi %$reg_hi_str                                   \n"
"      mov %$reg_lo, %2                                                \n"
"      mov %$reg_hi, 0                                                 \n"
"    %else                                                             \n"
"      ; movzx non-hi_lo, mem                                          \n"
"      ; Strip leading \"byte\" from %2 for lea.                       \n"
"      %define %$mem %2                                                \n"
"      %defstr %$mem_str %2                                            \n"
"      %substr %$pfx %$mem_str 1,4                                     \n"
"      %ifidni %$pfx, \"byte\"                                         \n"
"        %substr %$pfx %$mem_str 5                                     \n"
"        %ifidn %$pfx, \" \"                                           \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"        %ifidn %$pfx, `\t`                                            \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"        %ifidn %$pfx, \"[\"                                           \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"      %endif                                                          \n"
"      ; TBD: move segment prefix (if any) from \"lea ..., [seg:...]\" \n"
"      ; to \"mov ..., [seg:...]\"                                     \n"
"      lea  %1, %$mem                                                  \n"
"      xchg bx, %1                                                     \n"
"      mov  bl, [bx]                                                   \n"
"      mov  bh, 0                                                      \n"
"      xchg bx, %1                                                     \n"
"    %endif                                                            \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %pop ; __CTX_MOVZX__                                                \n"
"%endm                                                                 \n"
"                                                                      \n"
"%macro movsx 2.nolist                                                 \n"
"  %push __CTX_MOVSX__                                                 \n"
"                                                                      \n"
"  %assign %$r_n 8 ; not 8-bit register                                \n"
"  %ifidni %2,al                                                       \n"
"    %assign %$r_n 0                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,bl                                                       \n"
"    %assign %$r_n 1                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,cl                                                       \n"
"    %assign %$r_n 2                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,dl                                                       \n"
"    %assign %$r_n 3                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,ah                                                       \n"
"    %assign %$r_n 4                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,bh                                                       \n"
"    %assign %$r_n 5                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,ch                                                       \n"
"    %assign %$r_n 6                                                   \n"
"  %endif                                                              \n"
"  %ifidni %2,dh                                                       \n"
"    %assign %$r_n 7                                                   \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %assign %$l_n 8 ; not 16-bit register                               \n"
"  %ifidni %1,ax                                                       \n"
"    %assign %$l_n 0                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,bx                                                       \n"
"    %assign %$l_n 1                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,cx                                                       \n"
"    %assign %$l_n 2                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,dx                                                       \n"
"    %assign %$l_n 3                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,si                                                       \n"
"    %assign %$l_n 4                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,di                                                       \n"
"    %assign %$l_n 5                                                   \n"
"  %endif                                                              \n"
"  %ifidni %1,bp                                                       \n"
"    %assign %$l_n 6                                                   \n"
"  %endif                                                              \n"
"  ;%ifidni %1,sp                                                      \n"
"  ;  %assign %$l_n 7                                                  \n"
"  ;%endif                                                             \n"
"                                                                      \n"
"  %if %$l_n == 8                                                      \n"
"    %fatal expected word register (other than sp) in left operand     \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %if %$r_n != 8                                                      \n"
"    ; right operand is register                                       \n"
"    %if %$l_n == 0                                                    \n"
"      ; movsx ax, r                                                   \n"
"      %if %$r_n != 0                                                  \n"
"        mov al, %2                                                    \n"
"      %endif                                                          \n"
"      cbw                                                             \n"
"    %else                                                             \n"
"      %if %$r_n < 4                                                   \n"
"        %assign %$reg %$r_n                                           \n"
"      %else                                                           \n"
"        %assign %$reg %$r_n - 4                                       \n"
"      %endif                                                          \n"
"      %if %$l_n == %$reg                                              \n"
"        ; movsx non-ax, lo/hi of the same non-ax reg                  \n"
"        xchg ax, %1                                                   \n"
"        %if %$r_n >= 4                                                \n"
"          mov al, ah                                                  \n"
"        %endif                                                        \n"
"      %else                                                           \n"
"        ; movsx non-ax, lo/hi of other reg                            \n"
"        mov %1, ax                                                    \n"
"        %if %$r_n != 0                                                \n"
"          mov al, %2                                                  \n"
"        %endif                                                        \n"
"      %endif                                                          \n"
"      cbw                                                             \n"
"      xchg ax, %1                                                     \n"
"    %endif                                                            \n"
"  %else                                                               \n"
"    ; right operand is memory                                         \n"
"    %if %$l_n == 0                                                    \n"
"      ; movsx ax, mem                                                 \n"
"      mov al, %2                                                      \n"
"      cbw                                                             \n"
"    %else                                                             \n"
"      ; movsx non-ax, mem                                             \n"
"      ; Strip leading \"byte\" from %2 for lea.                       \n"
"      %define %$mem %2                                                \n"
"      %defstr %$mem_str %2                                            \n"
"      %substr %$pfx %$mem_str 1,4                                     \n"
"      %ifidni %$pfx, \"byte\"                                         \n"
"        %substr %$pfx %$mem_str 5                                     \n"
"        %ifidn %$pfx, \" \"                                           \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"        %ifidn %$pfx, `\t`                                            \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"        %ifidn %$pfx, \"[\"                                           \n"
"          %substr %$mem_str %$mem_str 5,-1                            \n"
"          %deftok %$mem %$mem_str                                     \n"
"        %endif                                                        \n"
"      %endif                                                          \n"
"      ; TBD: move segment prefix (if any) from \"lea ..., [seg:...]\" \n"
"      ; to \"mov ..., [seg:...]\"                                     \n"
"      lea  %1, %$mem                                                  \n"
"      %if %$l_n != 1                                                  \n"
"        xchg bx, %1                                                   \n"
"      %endif                                                          \n"
"      mov  bl, [bx]                                                   \n"
"      xchg ax, bx                                                     \n"
"      cbw                                                             \n"
"      xchg ax, bx                                                     \n"
"      %if %$l_n != 1                                                  \n"
"        xchg bx, %1                                                   \n"
"      %endif                                                          \n"
"    %endif                                                            \n"
"  %endif                                                              \n"
"                                                                      \n"
"  %pop ; __CTX_MOVSX__                                                \n"
"%endm                                                                 \n"
"                                                                      \n"
"%macro ___set_cc___ 2.nolist                                          \n"
"  mov %2, byte 0                                                      \n"
"  j%-1 %%skip                                                         \n"
"  mov %2, byte 1                                                      \n"
"%%skip:                                                               \n"
"%endm                                                                 \n"
"                                                                      \n"
"%macro seta 1.nolist                                                  \n"
"  ___set_cc___ a, %1                                                  \n"
"%endm                                                                 \n"
"%macro setae 1.nolist                                                 \n"
"  ___set_cc___ ae, %1                                                 \n"
"%endm                                                                 \n"
"%macro setb 1.nolist                                                  \n"
"  ___set_cc___ b, %1                                                  \n"
"%endm                                                                 \n"
"%macro setbe 1.nolist                                                 \n"
"  ___set_cc___ be, %1                                                 \n"
"%endm                                                                 \n"
"%macro setc 1.nolist                                                  \n"
"  ___set_cc___ c, %1                                                  \n"
"%endm                                                                 \n"
"%macro sete 1.nolist                                                  \n"
"  ___set_cc___ e, %1                                                  \n"
"%endm                                                                 \n"
"%macro setg 1.nolist                                                  \n"
"  ___set_cc___ g, %1                                                  \n"
"%endm                                                                 \n"
"%macro setge 1.nolist                                                 \n"
"  ___set_cc___ ge, %1                                                 \n"
"%endm                                                                 \n"
"%macro setl 1.nolist                                                  \n"
"  ___set_cc___ l, %1                                                  \n"
"%endm                                                                 \n"
"%macro setle 1.nolist                                                 \n"
"  ___set_cc___ le, %1                                                 \n"
"%endm                                                                 \n"
"%macro setna 1.nolist                                                 \n"
"  ___set_cc___ na, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnae 1.nolist                                                \n"
"  ___set_cc___ nae, %1                                                \n"
"%endm                                                                 \n"
"%macro setnb 1.nolist                                                 \n"
"  ___set_cc___ nb, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnbe 1.nolist                                                \n"
"  ___set_cc___ nbe, %1                                                \n"
"%endm                                                                 \n"
"%macro setnc 1.nolist                                                 \n"
"  ___set_cc___ nc, %1                                                 \n"
"%endm                                                                 \n"
"%macro setne 1.nolist                                                 \n"
"  ___set_cc___ ne, %1                                                 \n"
"%endm                                                                 \n"
"%macro setng 1.nolist                                                 \n"
"  ___set_cc___ ng, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnge 1.nolist                                                \n"
"  ___set_cc___ nge, %1                                                \n"
"%endm                                                                 \n"
"%macro setnl 1.nolist                                                 \n"
"  ___set_cc___ nl, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnle 1.nolist                                                \n"
"  ___set_cc___ nle, %1                                                \n"
"%endm                                                                 \n"
"%macro setno 1.nolist                                                 \n"
"  ___set_cc___ no, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnp 1.nolist                                                 \n"
"  ___set_cc___ np, %1                                                 \n"
"%endm                                                                 \n"
"%macro setns 1.nolist                                                 \n"
"  ___set_cc___ ns, %1                                                 \n"
"%endm                                                                 \n"
"%macro setnz 1.nolist                                                 \n"
"  ___set_cc___ nz, %1                                                 \n"
"%endm                                                                 \n"
"%macro seto 1.nolist                                                  \n"
"  ___set_cc___ o, %1                                                  \n"
"%endm                                                                 \n"
"%macro setp 1.nolist                                                  \n"
"  ___set_cc___ p, %1                                                  \n"
"%endm                                                                 \n"
"%macro setpe 1.nolist                                                 \n"
"  ___set_cc___ pe, %1                                                 \n"
"%endm                                                                 \n"
"%macro setpo 1.nolist                                                 \n"
"  ___set_cc___ po, %1                                                 \n"
"%endm                                                                 \n"
"%macro sets 1.nolist                                                  \n"
"  ___set_cc___ s, %1                                                  \n"
"%endm                                                                 \n"
"%macro setz 1.nolist                                                  \n"
"  ___set_cc___ z, %1                                                  \n"
"%endm                                                                 \n"
);
