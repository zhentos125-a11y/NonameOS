#include "shell/shell.h"

void kernel_main(void)
{
    shell_start();

    while (1)
    {
        __asm__ volatile("hlt");
    }
}
