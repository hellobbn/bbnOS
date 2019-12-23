%include "mem.inc"

global enable_irq   ; enable IRQ
global disable_irq  ; disable IRQ

; -------------------------------------------------------------
; void disable_irq(int irq);
; -------------------------------------------------------------
disable_irq:
    mov     ecx, [esp + 4]  ; get the irq
    pushf
    cli
    mov     ah, 1
    rol     ah, cl  ; ah = 1 << (irq % 8)
    cmp     cl, 8
    jae     disable_8   ; master or slave
disable_0:  ; master
    in      al, 0x21
    test    al, ah
    jnz     dis_ready
    or      al, ah
    out     0x21, al
    popf
    mov     eax, 1
    ret
disable_8:
    in      al, 0xA1
    test    al, ah
    jnz     dis_ready
    or      al, ah
    out     0xA1, al
    popf
    mov     eax, 1
    ret
dis_ready:
    popf
    xor     eax, eax
    ret

; -------------------------------------------------------------
; void enable_irq(int irq);
; -------------------------------------------------------------
enable_irq:
    mov     ecx, [esp + 4]  ; get the irq
    pushf
    cli
    mov     ah, ~1
    rol     ah, cl  ; ah = ~(1 << (irq % 8))
    cmp     cl, 8
    jae     enable_8   ; master or slave
enable_0:  ; master
    in      al, 0x21
    and      al, ah
    out     0x21, al
    popf
    ret
enable_8:
    in      al, 0xA1
    and     al, ah
    out     0xA1, al
    popf
    ret