#include "keyboard.h"

static inline unsigned char inb(unsigned short port)
{
    unsigned char result;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(result)
        : "Nd"(port)
    );

    return result;
}

static const char keymap[128] =
{
    0,  27, '1','2','3','4','5','6','7','8','9','0',
    '-', '=', '\b', '\t',
    'q','w','e','r','t','y','u','i','o','p', '[',']', '\n',
    0, /* 29 - Control */
    'a','s','d','f','g','h','j','k','l', ';','\'','`',
    0, /* 42 - Left Shift */
    '\\', 'z','x','c','v','b','nных','m', ',', '.', '/', 
    0, /* 54 - Right Shift */
    '*', /* 55 - Звездочка (*) на NumPad */
    0,   /* 56 - Alt */
    ' ', /* 57 - Пробел */
    0,0,0,0,0,0,0,0,0,0, /* 58-67 - Клавиши F1-F10 */
    0,   /* 68 - NumLock */
    0,   /* 69 - ScrollLock */
    0,   /* 70 - Home */
    0,   /* 71 - Up Arrow */
    0,   /* 72 - Page Up */
    '-', /* 73 - В некоторых клонах, пустая ячейка */
    '-', /* 74 - СТРОГО СЮДА: Минус (-) на NumPad */
    0,   /* 75 - Left Arrow */
    0,   /* 76 - Center */
    0,   /* 77 - Right Arrow */
    '+', /* 78 - СТРОГО СЮДА: Плюс (+) на NumPad */
};

char keyboard_getchar()
{
    unsigned char scancode;

    while (1)
    {
        /* Ждём, пока контроллер сообщит,
           что данные готовы */
        while (!(inb(0x64) & 1))
        {
        }

        scancode = inb(0x60);

        /* Игнорируем отпускание клавиши */
        if (scancode & 0x80)
        {
            continue;
        }

        if (scancode < 128)
        {
            char c = keymap[scancode];

            if (c)
            {
                return c;
            }
        }
    }
}
