#include "shell.h"
#include "commands.h"
#include "../drivers/keyboard.h"
#include "../gfx/gfx.h"

static volatile char* video = (volatile char*)0xB8000;
static int cursor = 0;
static char command_buffer[128];

void disable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void putchar(char c)
{
    /* Защита от выхода за пределы VGA-памяти */
    if (cursor >= 80 * 25 * 2)
    {
        scroll_up();
    }

    video[cursor++] = c;
    video[cursor++] = 0x0F;
}

/* Прокрутка: сдвигаем все строки вверх на одну */
void scroll_up(void)
{
    int i;

    for (i = 0; i < (24 * 80) * 2; i++)
    {
        video[i] = video[i + 160];
    }

    /* Последняя строка очищается */
    for (i = 24 * 80; i < 25 * 80; i++)
    {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }

    cursor = 24 * 160;
}

void print(const char* str)
{
    while (*str)
    {
        putchar(*str++);
    }
}

void newline()
{
    int current_row = cursor / 160;

    current_row++;

    if (current_row >= 25)
    {
        scroll_up();
    }
    else
    {
        cursor = current_row * 160;
    }
}

void clear_screen()
{
    int i;

    for (i = 0; i < 80 * 25; i++)
    {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }
    cursor = 0;
}

/* Откат текстового курсора на один символ назад (для Backspace) */
void cursor_back(void)
{
    if (cursor >= 2)
    {
        cursor -= 2;
        video[cursor] = ' ';
        video[cursor + 1] = 0x0F;
    }
}

static void prompt()
{
    print("MyOS> ");
}

void shell_start()
{
    disable_cursor();
    clear_screen();

    print("MyOS v0.1.0");
    newline();
    newline();

    while (1)
    {
        int pos = 0;
        int i;

        for (i = 0; i < 128; i++) command_buffer[i] = 0;

        prompt();

        while (1)
        {
            char c = keyboard_getchar();

            if (c == '\n')
            {
                command_buffer[pos] = 0;
                execute_command(command_buffer);
                newline();
                break;
            }

            /* Backspace: стираем символ */
            if (c == '\b')
            {
                if (pos > 0)
                {
                    pos--;
                    command_buffer[pos] = 0;
                    /* Откатываем курсор экрана назад и затираем символ */
                    cursor -= 2;
                    video[cursor] = ' ';
                    video[cursor + 1] = 0x0F;
                }
                continue;
            }

            if (pos < 127)
            {
                command_buffer[pos++] = c;
                putchar(c);
            }
        }
    }
}
