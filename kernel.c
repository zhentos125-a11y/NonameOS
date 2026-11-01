#include "shell/shell.h"
#include "gfx/gfx.h"
#include "fs/vfs.h"

/* Флаги и адрес multiboot_info, которые GRUB передал через boot.asm */
void kernel_main(unsigned int magic, unsigned int mbi_addr)
{
    (void)magic;

    /* Пытаемся переключиться в графику 800x600x32.
       Если GRUB оставил текстовый режим — shell работает по-старому. */
    if (gfx_init((void*)mbi_addr) == 0)
    {
        /* Есть фреймбуфер: рисуем фон и два тестовых окна */
        gfx_test_desktop();

        /* Виртуальная файловая система тоже видна на экране */
        vfs_init();
        gfx_draw_text(10, 580, "MyOS v0.1.0 - gfx + vfs prototype", GFX_CYAN);
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
