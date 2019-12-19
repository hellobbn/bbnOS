
; =============================================================
; TEXT SECTION
; =============================================================
[SECTION .text]

global  memcpy  ; set global
global  memset	; set global

; -------------------------------------------------------------
; PUBLIC void *memcpy(void *pDest, void *pSrc, int iSize);
; -------------------------------------------------------------
memcpy:
		push 		ebp
		mov 		ebp, esp

		push 		esi
		push 		edi
		push 		ecx

		mov 		edi, [ebp + 8]
		mov 		esi, [ebp + 12]
		mov 		ecx, [ebp + 16]
.1:
		cmp 		ecx, 0
		jz 			.2

		mov 		al, [ds:esi]
		inc 		esi

		mov 		byte [es:edi], al
		inc 		edi

		dec 		ecx
		jmp 		.1
.2:
		mov 		eax, [ebx + 8]

		pop 		ecx
		pop 		edi
		pop 		esi
		mov 		esp, ebp
		pop 		ebp

		ret

; -------------------------------------------------------------
; PUBLIC void memset(void* p_dest, char ch, int size);
; -------------------------------------------------------------
memset:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	edx, [ebp + 12]	; Char to be putted
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	byte [edi], dl		; ┓
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回