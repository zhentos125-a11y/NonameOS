; =====================================================================
;  boot.asm — MyOS: Multiboot 1 + ручной переход в графику через VBE
;
;  Почему не через заголовок Multiboot (бит 0x10 REQUEST_GFX_MODE):
;  Твоя версия GRUB не умеет читать gfx-поля и падает с ошибкой
;  "unsupported flag: 0x10" -> "you need to load the kernel first".
;  Поэтому заголовок остаётся классическим (флаги = 0), а графику
;  800x600x32 мы включаем САМИ прерыванием BIOS int 0x10 (VBE 0x4F02),
;  пока BIOS ещё жив — сразу после _start, до ядра.
;
;  Схема:
;   1) GRUB прыгает в _start: EAX=magic, EBX=multiboot_info. Покрываем
;      первые ~17 МБ identity-страницами (адрес == физический), чтобы
;      иметь возможность сохранить результат VBE выше 1 МБ без Page Fault.
;   2) VBE function 0x4F01 (check mode 0x112) + 0x4F02 (set mode, LBA).
;   3) PhysBasePtr фреймбуфера сохраняем в .bss (он ниже 1 МБ? нет —
;      но страницы уже включены, запись пройдёт).
;   4) Отключаем страницы, передаём (magic, mbi, fb_addr) в kernel_main.
;   Если что-то из VBE не удалось — fb_addr = 0, ядро останется в VGA.
; =====================================================================

section .multiboot
align 4
    dd 0x1BADB002                 ; magic
    dd 0x00000000                 ; flags = 0 (никаких gfx-полей!)
    dd -(0x1BADB002 + 0x00000000) ; checksum

section .bss
align 16                   ; выравнивание для resd (NASM требует >= размера данных)
    mb_magic:  resd 1
    mb_info:   resd 1
    vbe_fb:    resd 1              ; адрес линейного фреймбуфера (0 = текст)
    vbe_pitch: resd 1              ; байт на строку (BytesPerScanLine)

; ВАЖНО: каждая таблица страниц должна лежать на границе 4096 байт.
; Каждая таблица = ровно одна страница (1024 записи x 4 байта).
; Смещение boot_pd внутри секции должно быть кратно 4096, поэтому перед
; таблицами добавлен "filler" до границы страницы. Размер filler'а
; подбирается так: 16 (resd) + filler + 5*4096 == кратно 4096.
    _align_pad: resb 4080          ; добиваем смещение до границы 4К
    boot_pd:   resb 4096           ; Page Directory (4К)
    boot_pt0:  resb 4096           ; 0..4 МБ
    boot_pt1:  resb 4096           ; 4..8 МБ
    boot_pt2:  resb 4096           ; 8..12 МБ
    boot_pt3:  resb 4096           ; 12..16 МБ

section .text
global _start
extern kernel_main

_start:
    cli                           ; IDT GRUB'а нам не нужна, свои ISR ещё нет
    mov [mb_magic], eax
    mov [mb_info], ebx

    call enable_temp_paging       ; identity map 0..16 МБ (для записи >1МБ)

    ; ---- буфер VBE Control Info кладём в физ. 0x7000 (безопасный низ) ----
    ; Страницы включены => виртуальный адрес == физический.
    mov dword [vbe_fb], 0
    mov dword [vbe_pitch], 0

    ; --- VBE 0x4F01: проверить режим 0x112 (800x600x32 LBA) --------------
    mov ax, 0x4F01
    mov cx, 0x112
    mov di, 0x2000                ; ES:DI -> буфер CI (ES=0 при входе из GRUB)
    xor esi, esi                  ; (не используется в 16-битном сегменте DI)
    int 0x10
    cmp ax, 0x004F                ; успешный ответ VBE = 0x004F
    jne .no_vbe
    test word [di], 0x0001        ; бит 0 атрибута: режим поддерживается
    jz .no_vbe

    ; --- VBE 0x4F02: включить режим 0x112 + флаг LBA (bit 14) ------------
    mov ax, 0x4F02
    mov bx, 0x4112                ; 0x112 | 0x4000 (LBA)
    int 0x10
    cmp ax, 0x004F
    jne .no_vbe

    ; --- забираем PhysBasePtr (смещение 0x18) и BytesPerScanLine (0x13) --
    mov eax, [di + 0x18]
    mov [vbe_fb], eax
    movzx eax, word [di + 0x12]
    mov [vbe_pitch], eax
    jmp .have_pages_done

.no_vbe:
    mov dword [vbe_fb], 0         ; графики нет — останемся в текстовом VGA

.have_pages_done:
    call disable_temp_paging      ; возвращаемся в плоский режим Multiboot

    push dword [vbe_pitch]        ; 4-й аргумент: pitch (0 если текст)
    push dword [vbe_fb]           ; 3-й аргумент: фреймбуфер (0 если текст)
    push dword [mb_info]          ; 2-й аргумент: multiboot_info
    push dword [mb_magic]         ; 1-й аргумент: магическое число
    call kernel_main

.hang:
    hlt
    jmp .hang

; =====================================================================
;  enable_temp_paging —identity map первых 16 МБ, страницы включаются.
; =====================================================================
enable_temp_paging:
    ; Обнуляем PD (первые 4 записи перезапишем, остальное должно быть 0)
    mov edi, boot_pd
    xor eax, eax
    mov ecx, 1024
    rep stosd

    ; PD[0..3] -> PT0..PT3 (Present | R/W)
    mov eax, boot_pt0
    or  eax, 0x03
    mov [boot_pd + 0],  eax
    mov eax, boot_pt1
    or  eax, 0x03
    mov [boot_pd + 4],  eax
    mov eax, boot_pt2
    or  eax, 0x03
    mov [boot_pd + 8],  eax
    mov eax, boot_pt3
    or  eax, 0x03
    mov [boot_pd + 12], eax

    ; Заполняем 4 таблицы страниц по 1024 идентичных отображений.
    ; Эталонная строка: base | Present | R/W (base = n * 4 МБ).
    ; Каждая следующая запись на 4 байта больше предыдущей -> просто прибавляем.
    ; ВАЖНО: в 32-битном адресовании нельзя [boot_pt0 + ebx*4096],
    ; поэтому вычисляем адрес таблицы через lea с множителем 4 и
    ; последовательным сдвигом (шаг между таблицами = 4096 << 2 = 16384).
    mov eax, boot_pt0               ; edi = текущая таблица
    mov ebx, 0                      ; номер блока (0..3)
.fill_tables:
    mov edi, eax                    ; куда пишем
    mov ecx, ebx
    shl ecx, 22                     ; базовый адрес блока: n * 4 МБ
    or  ecx, 0x03                   ; Present | Read/Write
    mov edx, ecx                    ; эталонная запись
    mov ecx, 1024                   ; counter
.fill_page:
    mov [edi], edx
    add edi, 4
    add edx, 4096                   ; следующая страница того же блока
    dec ecx
    jnz .fill_page
    inc ebx
    add eax, 4096                   ; следующая таблица (+4К)
    cmp ebx, 4
    jb  .fill_tables

    ; CR3 <- boot_pd
    mov eax, boot_pd
    mov cr3, eax

    ; CR0 |= PG (бит 31)
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    ret

disable_temp_paging:
    ; CR0 &= ~PG
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    ; сброс TLB
    mov eax, cr3
    mov cr3, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
