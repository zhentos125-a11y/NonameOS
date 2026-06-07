#include "commands.h"
#include "shell.h"
extern void print(const char* str);
extern void newline();
extern void clear_screen();
extern void putchar(char c);
extern char keyboard_getchar();

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

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_negative = 1;
        if (num == -2147483648) {
            char* min_int = "-2147483648";
            while (*min_int) str[i++] = *min_int++;
            str[i] = '\0';
            return;
        }
        num = -num;
    }

    while (num > 0) {
        int temp = num;
        while (temp >= 10) {
            temp -= 10;
        }
        int rem = temp;
        
        str[i++] = rem + '0';
        
        int new_num = 0;
        while (num >= 10) {
            num -= 10;
            new_num++;
        }
        num = new_num;
    }

    if (is_negative) {
        str[i++] = '-';
    }
    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static int strcmp(const char* a, const char* b)
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

void execute_command(char* cmd)
{
    if (strcmp(cmd, "help"))
    {
        newline();
        print("help");
        newline();
        print("about");
        newline();
        print("version");
        newline();
        print("calc");
        newline();
        print("clear");
        newline();
        print("exit");
        newline();
    }
    else if (strcmp(cmd, "about"))
    {
        newline();
        print("MyOS by iQkz");
        newline();
    }
    else if (strcmp(cmd, "version"))
    {
        newline();
        print("MyOS v0.0.3");
        newline();
    }
    else if (strcmp(cmd, "calc"))
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
            int sign = 1;
            if (num1 < 0) { sign = -sign; num1 = -num1; }
            if (num2 < 0) { sign = -sign; num2 = -num2; }
            for (int m = 0; m < num2; m++) {
                result += num1;
            }
            result *= sign;
        }
        else if (op == '/') {
            if (num2 == 0) {
                newline(); 
                print("Error: Division by zero!");
                newline(); 
                return;
            }
            int sign = 1;
            if (num1 < 0) { sign = -sign; num1 = -num1; }
            if (num2 < 0) { sign = -sign; num2 = -num2; }
            result = 0;
            while (num1 >= num2) {
                num1 -= num2;
                result++;
            }
            result *= sign;
        }

        char res_str[32] = {0};
        custom_itoa(result, res_str);
        
        newline(); 
        print("Result: ");
        print(res_str);
        newline();
    }
    else if (strcmp(cmd, "clear"))
    {
        clear_screen();
    }
    else if (strcmp(cmd, "exit"))
    {
        newline();
        print("Shutting down...");
        newline();

        // Побайтовый вывод значения 0x2000 для QEMU (0x00 -> 0x604, 0x20 -> 0x605)
        outb(0x604, 0x00);
        outb(0x605, 0x20);

        // Побайтовый вывод для VirtualBox (0x00 -> 0xB004, 0x20 -> 0xB005)
        outb(0xB004, 0x00);
        outb(0xB005, 0x20);

        // Старый порт завершения QEMU
        outb(0x501, 0x31);

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

