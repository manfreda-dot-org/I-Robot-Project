; void ScanEdge(int X1, int Y1, int X2, int Y2, int SetXStart, int SkipFirst, POINT **EdgePtr)

	.486
	.MODEL	flat, stdcall, c
	OPTION	READONLY

SCREEN_HEIGHT	EQU	480

ScanEdge PROTO C X1:DWORD, Y1:DWORD, X2:DWORD, Y2:DWORD, SetXStart:DWORD, SkipFirst:DWORD, EdgePtr:DWORD

	code	SEGMENT PARA READONLY FLAT 'CODE'

	ALIGN	16
ScanEdge PROC C PUBLIC USES EAX EBX ECX EDX EBP EDI ESI, X1:DWORD, Y1:DWORD, X2:DWORD, Y2:DWORD, SetXStart:DWORD, SkipFirst:DWORD, EdgePtr:DWORD
	LOCAL	AdvanceAmt:DWORD, Height:DWORD, LinesToSkip:DWORD

; check to see if line segment is visible
	cmp	Y1,SCREEN_HEIGHT
	jge	ScanEdgeExit
	cmp	Y2,0
	jl	ScanEdgeExit

; determine number of lines to initially skip
	xor	eax,eax
	cmp	Y1,0
	jge	Skip
	mov	eax,Y1
	neg	eax
	mov	SkipFirst,0	; dont skip the first line, since it is inherently skipped
Skip:	mov	LinesToSkip,eax


; EDI is pointer to current XStart/XEnd
	mov	edi,EdgePtr
	mov	edi,[edi]
	cmp	SetXStart,1
	jz	HLinePtrSet
	add	edi,4


HLinePtrSet:
; Get true height
	mov	ebx,Y2
	sub	ebx,Y1
	mov	Height,ebx
; EBX = count height
	mov	ecx,Y1
	test	ecx,ecx
	jns	CheckY2
	xor	ecx,ecx
CheckY2:
	mov	ebx,Y2
	cmp	ebx,SCREEN_HEIGHT
	jl	CheckHeight
	mov	ebx,SCREEN_HEIGHT-1
CheckHeight:
	sub	ebx,ecx
	jle	ScanEdgeExit	; guard against zero length (horizontal) or negative edges

; EAX = width
	xor	ecx,ecx
	mov	edx,1
	mov	eax,X2
	sub	eax,X1
	jz	IsVertical
	jns	SetAdvanceAmt
	mov	ecx,1
	sub	ecx,Height
	neg	edx
	neg	eax
SetAdvanceAmt:
	mov	AdvanceAmt,edx

; Figure out whether the edge is diagonal, X-major (more horizontal),
; or Y-major (more vertical) and handle appropriately.
	cmp	eax,Height
	jz	IsDiagonal
	jb	YMajor
	xor	edx,edx
	div	Height		; width / height
				; EAX = minimum # pixels to advance X each scanline
				; EDX = error term advance per scan line
	mov	esi,eax
	test	AdvanceAmt,80000000h
	jz	XMajorAdvanceAmtSet
	neg	esi


XMajorAdvanceAmtSet:
	mov	eax,X1

	cmp	LinesToSkip,0
	je	XDoneSkipping
	xchg	LinesToSkip,edi
XSkipLoop:
	add	eax,esi
	add	ecx,edx
	jle	XSkipNoAdvance
	add	eax,AdvanceAmt
	sub	ecx,Height
XSkipNoAdvance:
	dec	edi
	jnz	XSkipLoop
	mov	edi,LinesToSkip
XDoneSkipping:

	cmp	SkipFirst,1	; skip the first point?
	jz	XMajorSkipEntry

XMajorLoop:
	mov	[edi],eax
	add	edi,8		; point to next structure

XMajorSkipEntry:
	add	eax,esi		; set X for the next scan line
	add	ecx,edx		; advance error term
	jle	XMajorNoAdvance
	add	eax,AdvanceAmt
	sub	ecx,Height

XMajorNoAdvance:
        dec     ebx		;count off this scan line
        jnz     XMajorLoop
	jmp	ScanEdgeDone




        ALIGN	16
IsVertical:
	mov	eax,X1
	sub	ebx,SkipFirst
	jle	ScanEdgeExit

VerticalLoop:
	mov	[edi],eax
	add	edi,8
	dec	ebx
	jnz	VerticalLoop
	jmp	ScanEdgeDone




        ALIGN	16
IsDiagonal:
	mov	eax,X1

	cmp	LinesToSkip,0
	je	DiagonalDoneSkipping
	mov	ecx,LinesToSkip

DSkipLoop:
	add	eax,edx
	dec	ecx
	jnz	DSkipLoop
DiagonalDoneSkipping:

	cmp	SkipFirst,1
	jz	DiagonalSkipEntry

DiagonalLoop:
	mov	[edi],eax
	add	edi,8

DiagonalSkipEntry:
	add	eax,edx
	dec	ebx
	jnz	DiagonalLoop
	jmp	ScanEdgeDone




        ALIGN	16
YMajor:
	mov	esi,X1

	cmp	LinesToSkip,0
	je	YDoneSkipping
	xchg	LinesToSkip,edi
YSkipLoop:
	add	ecx,eax
	jle	YSkipNoAdvance
	add	esi,edx
	sub	ecx,Height
YSkipNoAdvance:
	dec	edi
	jnz	YSkipLoop
	mov	edi,LinesToSkip
YDoneSkipping:

        cmp     SkipFirst,1
	push	Height
	xchg	ebp,[esp]		;put Height in EBP for error term calcs
        jz	YMajorSkipEntry

YMajorLoop:
	mov	[edi],esi
	add	edi,8

YMajorSkipEntry:
	add	ecx,eax
	jle	YMajorNoAdvance
	add	esi,edx
        sub     ecx,ebp

YMajorNoAdvance:
        dec     ebx
        jnz     YMajorLoop
	pop	ebp




ScanEdgeDone:
	cmp	SetXStart,1
	jz	UpdateHLinePtr
	sub	edi,4

UpdateHLinePtr:
	mov	ebx,EdgePtr
	mov	[ebx],edi

ScanEdgeExit:
	ret

ScanEdge ENDP

	code	ENDS
END