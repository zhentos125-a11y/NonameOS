#include "shell.h"
#include "commands.h"
#include "../drivers/keyboard.h"

static volatile char* video = (volatile char*)0xB8000;
static int cursor = 0;
char command_buffer[128];

// ФУНКЦИЮ outb ОТСЮДА ПОЛНОСТЬЮ СТЕРЛИ!

void disable_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void putchar(char c)
{
    video[cursor++] = c;
    video[cursor++] = 0x0F;
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
    // Рассчитываем текущую строку на основе глобального курсора
    int current_row = cursor / 160;
    
    // Переходим на следующую строку
    current_row++;

    // Если вылезли за пределы экрана (25 строк)
    if (current_row >= 25)
    {
        clear_screen(); // Очистит экран и САМА сбросит cursor в 0
    }
    else
    {
        // Если экран еще не полон, просто выставляем курсор на начало новой строки
        cursor = current_row * 160;
    }
}

void clear_screen()
{
    for (int i = 0; i < 80 * 25; i++)
    {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x0F;
    }
    cursor = 0;
}

static void prompt()
{
    print("MyOS> ");
}

void shell_start()
{
    disable_cursor();
    clear_screen();

    print("MyOS v0.3");
    newline();
    newline();

    while (1)
    {
        int pos = 0;
        for (int i = 0; i < 128; i++) command_buffer[i] = 0;

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

            if (pos < 127)
            {
                command_buffer[pos++] = c;
                putchar(c);
            }
        }
    }
}

