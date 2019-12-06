;; lib.inc

;; 显示 AL 中的数字
DispAL:
	push	ecx
	push	edx
	push	edi

	mov	edi, [dwDispPos]

	mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
	mov	dl, al
	shr	al, 4
	mov	ecx, 2
.begin:
	and	al, 01111b
	cmp	al, 9
	ja	.1
	add	al, '0'
	jmp	.2
.1:
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	.begin
	;add	edi, 2

	mov	[dwDispPos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
;; DispAL 结束


;; 显示一个整形数
DispInt:
	mov	eax, [esp + 4]
	shr	eax, 24
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 16
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4
	mov	[dwDispPos], edi
	pop	edi

	ret
;; DispInt 结束

;; 显示一个字符串
PMDispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [dwDispPos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[dwDispPos], edi

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
;; PMDispStr 结束

;; 换行
DispReturn:
	push	szReturn
	call	PMDispStr			;printf("\n");
	add	esp, 4

	ret
;; DispReturn 结束

;; clear framebuffer
ClearScreen:
    push    edi
    push    ecx
    push    eax
    mov     cx, 80 * 25
    mov     ah, 0FH
    mov     al, 0x20
    xor		edi, edi
    mov     [gs:edi], ax
.1:
    mov     [gs:edi], ax
    add     edi, 2
    loop    .1
.2:
    pop     eax
    pop     ecx
    pop     edi
	ret
;; end of ClearScreen

;; MemCpy
Memcpy:
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