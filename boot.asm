section .multiboot
align 4
    ; BIT 4 (0x10) = REQUEST_GFX_MODE: просим у GRUB графический режим
    dd 0x1BADB002
    dd 0x00000010
    ; Контрольная сумма: 0x1BADB002 + 0x10 + CHECKSUM = 0
    dd -(0x1BADB002 + 0x10)

    ; Запрашиваем 800x600, 32 бита на пиксель.
    ; Последнее "dd 0" = preferred type: если GRUB не сможет включить
    ; графику, он всё равно загрузит ядро в текстовом режиме (VGA 0xB8000).
    dd 800            ; preferred width
    dd 600            ; preferred height
    dd 32             ; preferred depth (bpp)
    dd 0              ; preferred frame buffer type

section .bss
align 4
mb_flags:   resd 1
mb_info:    resd 1

section .text
global _start
extern kernel_main

_start:
    cli
    ; Сохраняем EAX (магическое число Multiboot) и EBX (адрес multiboot_info),
    ; которые передал GRUB, чтобы передать их в kernel_main
    mov [mb_flags], eax
    mov [mb_info], ebx
    push ebx
    push eax
    call kernel_main

.hang:
    hlt
    jmp .hang

section .note.GNU-stack noalloc noexec nowrite progbits
