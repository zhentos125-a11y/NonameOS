#include "commands.h"
#include "shell.h"
#include "../fs/vfs.h"

extern void print(const char* str);
extern void newline();
extern void clear_screen();
extern void putchar(char c);
extern char keyboard_getchar();

/* ===== Свои itoa/atoi: стандартной библиотеки в ОС нет ===== */

static int custom_atoi(const char* str) {
    int res = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str) {
        if (*str < '0' || *str > '9') break;
        res = (res << 3) + (res << 1) + (*str - '0');
        str++;
    }
    return res * sign;
}

static void custom_itoa(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    int start, end;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        if (num == (-2147483647 - 1)) { /* INT_MIN, чтобы не было UB */
            const char* min_int = "-2147483648";
            int j;
            for (j = 0; min_int[j]; j++) str[i++] = min_int[j];
            str[i] = '\0';
            return;
        }
        num = -num;
    }

    /* Классический разбор по десятичному остатку */
    while (num > 0) {
        int rem = num % 10;
        str[i++] = (char)('0' + rem);
        num /= 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';

    /* Переворачиваем строку */
    start = 0;
    end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static int my_strcmp(const char* a, const char* b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return 0;
        a++;
        b++;
    }
    return *a == *b;
}

/* Простейший поиск подстроки "ключ " внутри команды */
static int starts_with(const char* cmd, const char* key, const char** rest)
{
    int i = 0;

    while (key[i])
    {
        if (cmd[i] != key[i]) return 0;
        i++;
    }

    if (rest) *rest = cmd + i;
    return 1;
}

void execute_command(char* cmd)
{
    if (my_strcmp(cmd, "help"))
    {
        newline();
        print("help      - this list");
        newline();
        print("about     - about MyOS");
        newline();
        print("version   - kernel version");
        newline();
        print("calc      - calculator");
        newline();
        print("clear     - clear screen");
        newline();
        print("ls        - list root directory");
        newline();
        print("touch NAME / mkdir NAME - create file/folder");
        newline();
        print("cat NAME  - read file");
        newline();
        print("exit      - power off");
        newline();
    }
    else if (my_strcmp(cmd, "about"))
    {
        newline();
        print("MyOS by iQkz");
        newline();
    }
    else if (my_strcmp(cmd, "version"))
    {
        newline();
        print("MyOS v0.1.0");
        newline();
    }
    else if (my_strcmp(cmd, "calc"))
    {
        char buf1[16] = {0};
        char buf2[16] = {0};
        char op = 0;
        int p = 0;

        newline();
        print("Enter first number: ");
        while (1) {
            char c = keyboard_getchar();
            if (c == '\n') {
                buf1[p] = 0;
                break;
            }
            if (c == '\b') {
                if (p > 0) { p--; cursor_back(); }
                continue;
            }
            if (((c >= '0' && c <= '9') || (c == '-' && p == 0)) && p < 15) {
                buf1[p++] = c;
                putchar(c);
            }
        }

        newline();
        print("Enter operator (+, -, *, /): ");
        while (1) {
            char c = keyboard_getchar();
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                op = c;
                putchar(c);
                break;
            }
        }

        p = 0;
        newline();
        print("Enter second number: ");
        while (1) {
            char c = keyboard_getchar();
            if (c == '\n') {
                buf2[p] = 0;
                break;
            }
            if (c == '\b') {
                if (p > 0) { p--; cursor_back(); }
                continue;
            }
            if (((c >= '0' && c <= '9') || (c == '-' && p == 0)) && p < 15) {
                buf2[p++] = c;
                putchar(c);
            }
        }

        int num1 = custom_atoi(buf1);
        int num2 = custom_atoi(buf2);
        int result = 0;

        if (op == '+') {
            result = num1 + num2;
        }
        else if (op == '-') {
            result = num1 - num2;
        }
        else if (op == '*') {
            result = num1 * num2;
        }
        else if (op == '/') {
            if (num2 == 0) {
                newline();
                print("Error: Division by zero!");
                newline();
                return;
            }
            result = num1 / num2;
        }

        char res_str[32] = {0};
        custom_itoa(result, res_str);

        newline();
        print("Result: ");
        print(res_str);
        newline();
    }
    else if (my_strcmp(cmd, "clear"))
    {
        clear_screen();
    }
    else if (my_strcmp(cmd, "ls"))
    {
        list_directory(0);
    }
    else if (starts_with(cmd, "touch ", (const char**)0) ||
             starts_with(cmd, "mkdir ", (const char**)0))
    {
        const char* rest = 0;
        char name[16];
        int i = 0;

        if (starts_with(cmd, "touch ", &rest))
        {
            while (rest[i] && i < 15) { name[i] = rest[i]; i++; }
            name[i] = 0;
            create_file(name, 0, "");
            newline();
            print("File created: ");
            print(name);
            newline();
        }
        else
        {
            starts_with(cmd, "mkdir ", &rest);
            while (rest[i] && i < 15) { name[i] = rest[i]; i++; }
            name[i] = 0;
            create_folder(name, 0);
            newline();
            print("Folder created: ");
            print(name);
            newline();
        }
    }
    else if (starts_with(cmd, "cat ", (const char**)0))
    {
        const char* rest = 0;
        char name[16];
        int i = 0;
        const char* text;

        starts_with(cmd, "cat ", &rest);
        while (rest[i] && i < 15) { name[i] = rest[i]; i++; }
        name[i] = 0;

        text = vfs_read_file(name, 0);
        newline();
        if (text)
        {
            print(text);
        }
        else
        {
            print("File not found");
        }
        newline();
    }
    else if (my_strcmp(cmd, "exit"))
    {
        newline();
        print("Shutting down...");
        newline();

        /* isa-debug-exit: QEMU завершится с кодом (val<<1)|1.
           Пишем 0x10 -> код выхода 33 (обычный "чистый" shutdown). */
        outb(0x501, 0x31);   /* старый порт завершения QEMU */
        outb(0xF4, 0x10);    /* isa-debug-exit (порт из Makefile-флага) */

        while (1)
        {
            __asm__ volatile("hlt");
        }
    }
    else
    {
        newline();
        print("Unknown command");
        newline();
    }
}
