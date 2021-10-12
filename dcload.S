#include "dcload.h"

#define ALIGN_2BYTE	.align 1
#define ALIGN_4BYTE	.align 2
#define ALIGN_32BYTE	.align 5
#define ALIGN_CACHELINE	ALIGN_32BYTE

#define CDECL(name) _ ## name

#define DC_VID_BORDER		(0xa05f8040)

ALIGN_4BYTE
.globl CDECL(DCLoadAvailable)
CDECL(DCLoadAvailable):
		mov.l	.Ldcload_magic, r0
		mov.l	.Ldcload_magic_value, r1
		mov.l	@r0, r2
		cmp/eq	r1, r2
		rts
		movt	r0

// r4-r7 syscall params
.globl CDECL(DCLoadSyscall)
CDECL(DCLoadSyscall):
		mov.l	.Ldcload_magic, r0
		mov.l	@(DCLOAD_MAGIC_SYSCALL_OFS, r0), r0
		jmp	@r0
		nop
		
	ALIGN_4BYTE
	.Ldcload_magic:
		.long	DCLOAD_MAGIC_ADDRESS
	.Ldcload_syscall:
		.long	DCLOAD_SYSCALL_ADDRESS
	.Ldcload_magic_value:
		.long	DCLOAD_MAGIC_VALUE
		
.global CDECL(dclWriteStdout)
CDECL(dclWriteStdout):
		mov	r4, r6
		mov	#DCLOAD_WRITE, r4
		mov	r5, r7
		bra	CDECL(DCLoadSyscall)
		mov	#1, r5		//STDOUT
		
.global CDECL(vidInvertBorder)
CDECL(vidInvertBorder):
		mov.l	.Lvid_border, r1
		mov.l	@r1, r2
		not	r2, r2
		rts
		mov.l	r2, @r1
	ALIGN_4BYTE
	.Lvid_border:
		.long	DC_VID_BORDER
	
.global CDECL(dcExit)
CDECL(dcExit):
	bra	CDECL(DCLoadSyscall)
	mov	#DCLOAD_EXIT, r4


