ENTRY(_start)

SECTIONS
{
    /* Начинаем загрузку с 1 Мегабайта, как требует Multiboot */
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)   /* Заголовок строго в самом начале */
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.COMMON)
        *(.bss)
    }

    /* КРИТИЧЕСКИ ВАЖНО: Выбрасываем мусор компилятора, ломающий GRUB */
    /DISCARD/ :
    {
        *(.eh_frame)
        *(.note.*)
        *(.comment)
    }
}

