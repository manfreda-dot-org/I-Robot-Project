; void DrawHorizontalLineList( POINT * LinePtr, int ListLength, int StartScanLine, int Color );

	.486
	.MODEL	flat, stdcall, c
	OPTION	READONLY

SCREEN_WIDTH	EQU	640

EXTERN ScreenBuffer:DWORD
EXTERN ScreenPitch:DWORD

DrawHorizontalLineList PROTO C LinePtr:DWORD, ListLength:DWORD, StartScanLine:DWORD, Color:DWORD

	code	SEGMENT PARA READONLY FLAT 'CODE'

	ALIGN	16
DrawHorizontalLineList PROC C PUBLIC USES EAX EBX ECX EDX ESI EDI, LinePtr:DWORD, ListLength:DWORD, StartScanLine:DWORD, Color:DWORD

; string operations work on data segement
	mov	ax,ds
	mov	es,ax

; make string instructions increment pointers
	cld

; EDX points to the start of the first scan line
	mov	eax,ScreenPitch
	mul	StartScanLine
	mov	edx,eax
	add	edx,ScreenBuffer

; EBX points to the first XStart/Xend descriptor
	mov	ebx,LinePtr

; ESI points to number of scan lines to draw
	mov	esi,ListLength
	test	esi,esi
	jz	FillDone

; AX has color to fill with
	mov	eax,Color
	mov	ah,al
;	push	ax
;	bswap	eax
;	pop	ax

FillLoop:
; get start/end pixel on line to fill
	mov	edi,[ebx]	; XStart
	mov	ecx,[ebx+4]	; XEnd

; check start boundaries
	test	edi,edi
 	jns	CheckEnd
	xor	edi,edi
; check end boundaries
CheckEnd:
	cmp	ecx,SCREEN_WIDTH
	jl	EndCheck
	mov	ecx,SCREEN_WIDTH-1
EndCheck:

; determine number of bytes to fill
	sub	ecx,edi
	js	LineFillDone
	inc	ecx

; determine address of first pixel to fill
	add	edi,edx

; fill any odd leading bytes (address is odd)
	test	edi,1
	jz	MainFill
	stosb
	dec	ecx
	jz	LineFillDone

MainFill:
; fill all WORDs
	shr	ecx,1
	rep	stosw
	adc	ecx,ecx
	rep	stosb

LineFillDone:
	add	ebx,8
	add	edx,ScreenPitch
	dec	esi
	jnz	FillLoop

FillDone:
	ret

DrawHorizontalLineList ENDP

	code	ENDS
END