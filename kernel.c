#include "shell/shell.h"
#include "gfx/gfx.h"
#include "fs/vfs.h"

/* Флаги и адрес multiboot_info, которые GRUB передал через boot.asm.
   fb_addr/pitch — результат VBE-переключения в 800x600x32, которое
   boot.asm сделал сам (наш GRUB не поддерживает gfx-тег Multiboot 0x10). */
void kernel_main(unsigned int magic, unsigned int mbi_addr,
                 unsigned int fb_addr, unsigned int fb_pitch)
{
    (void)magic;
    (void)mbi_addr;

    /* Пытаемся использовать графику 800x600x32, включённую boot.asm.
       Если VBE не сработал (fb_addr == 0) — shell работает по-старому
       в текстовом VGA-режиме. */
    if (gfx_init_vbe((void*)fb_addr, fb_pitch) == 0)
    {
        /* Есть фреймбуфер: рисуем фон и два тестовых окна */
        gfx_test_desktop();

        /* Виртуальная файловая система тоже видна на экране */
        vfs_init();
        gfx_draw_text(10, 580, "MyOS v0.1.0 - gfx(VBE) + vfs prototype", GFX_CYAN);
        gfx_refresh();
    }

    /* Shell пишет в VGA-текст (0xB8000). В графическом режиме он
       продолжит работать "в тени" фреймбуфера, но клавиатура и команды
       останутся доступны через тот же цикл. */
    shell_start();

    while (1)
    {
        __asm__ volatile("hlt");
    }
}
