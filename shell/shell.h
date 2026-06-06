#ifndef SHELL_H
#define SHELL_H

// Настоящий bare-metal макрос. Физически вшивает asm-код в каждый файл
#define outb(port, val) __asm__ volatile ("outb %b0, %w1" : : "a"(val), "Nd"(port))

void shell_start();

void print(const char* str);
void newline();
void clear_screen();
void putchar(char c);

#endif

