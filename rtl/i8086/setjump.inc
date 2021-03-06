{
    This file is part of the Free Pascal run time library.
    Copyright (c) 2013 by the Free Pascal development team

    SetJmp and LongJmp implementation for exception handling

    See the file COPYING.FPC, included in this distribution,
    for details about the copyright.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 **********************************************************************}

{$ifdef WIN16}

Function fpc_SetJmp (Var S : Jmp_buf) : smallint;[Public, alias : 'FPC_SETJMP']; compilerproc;
begin
{$ifdef FPC_X86_DATA_NEAR}
  fpc_SetJmp:=Catch(Ptr(DSeg,Word(@CATCHBUF(S.catch_buf))));
{$else FPC_X86_DATA_NEAR}
  fpc_SetJmp:=Catch(@CATCHBUF(S.catch_buf));
{$endif FPC_X86_DATA_NEAR}
end;

Procedure fpc_longJmp (Var S : Jmp_buf; value : smallint);[Public, alias : 'FPC_LONGJMP']; compilerproc;
begin
{$ifdef FPC_X86_DATA_NEAR}
  Throw(Ptr(DSeg,Word(@CATCHBUF(S.catch_buf))),value);
{$else FPC_X86_DATA_NEAR}
  Throw(@CATCHBUF(S.catch_buf),value);
{$endif FPC_X86_DATA_NEAR}
end;

{$else WIN16}

Function fpc_SetJmp (Var S : Jmp_buf) : smallint;assembler;nostackframe;[Public, alias : 'FPC_SETJMP']; compilerproc;
asm
  mov si, sp

{$ifdef FPC_X86_DATA_NEAR}
  mov di, ss:[si + 2 + extra_param_offset]  // S
  mov ax, ds
  mov es, ax
{$else FPC_X86_DATA_NEAR}
  les di, ss:[si + 2 + extra_param_offset]  // S
{$endif FPC_X86_DATA_NEAR}
  mov word es:[di + Jmp_buf.bp], bp
  mov ax, word ss:[si]
  mov word es:[di + Jmp_buf.ip], ax
{$ifdef FPC_X86_CODE_FAR}
  mov ax, word ss:[si + 2]
  mov word es:[di + Jmp_buf.cs], ax
{$endif FPC_X86_CODE_FAR}
  lea ax, [si + 4 + extra_param_offset + extra_data_offset]
  mov word es:[di + Jmp_buf.sp], ax

  xor ax, ax
end;


Procedure fpc_longJmp (Var S : Jmp_buf; value : smallint); assembler;nostackframe;[Public, alias : 'FPC_LONGJMP']; compilerproc;
asm
  mov si, sp

{$ifdef FPC_X86_DATA_NEAR}
  mov bx, ss:[si + 4 + extra_param_offset]  // S
{$else FPC_X86_DATA_NEAR}
  les bx, ss:[si + 4 + extra_param_offset]  // S
{$endif FPC_X86_DATA_NEAR}
  mov ax, ss:[si + 2 + extra_param_offset]  // value
  test ax, ax
  jnz @@L1
  inc ax
@@L1:
{$ifdef FPC_X86_DATA_NEAR}
  mov bp, word [bx + Jmp_buf.bp]
  mov sp, word [bx + Jmp_buf.sp]
{$else FPC_X86_DATA_NEAR}
  mov bp, word es:[bx + Jmp_buf.bp]
  mov sp, word es:[bx + Jmp_buf.sp]
{$endif FPC_X86_DATA_NEAR}
  // we should also clear the fpu
  // fninit no must be done elsewhere PM
  // or we should reset the control word also
{$ifdef FPC_X86_DATA_NEAR}
  {$ifdef FPC_X86_CODE_NEAR}
    jmp word [bx + Jmp_buf.ip]
  {$else FPC_X86_CODE_NEAR}
    // the inline asm doesn't support jmp far yet, so we use db for now
    // jmp far [bx + Jmp_buf.ip]
    db 0FFh, 06Fh, Jmp_buf.ip
  {$endif FPC_X86_CODE_NEAR}
{$else FPC_X86_DATA_NEAR}
  {$ifdef FPC_X86_CODE_NEAR}
    jmp word es:[bx + Jmp_buf.ip]
  {$else FPC_X86_CODE_NEAR}
    // the inline asm doesn't support jmp far yet, so we use db for now
    // jmp far es:[bx + Jmp_buf.ip]
    db 026h, 0FFh, 06Fh, Jmp_buf.ip
  {$endif FPC_X86_CODE_NEAR}
{$endif FPC_X86_DATA_NEAR}
end;

{$endif WIN16}
