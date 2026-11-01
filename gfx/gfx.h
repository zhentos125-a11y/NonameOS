#ifndef GFX_H
#define GFX_H

/* Простые цвета (XRGB8888) */
#define GFX_BLACK   0x000000
#define GFX_WHITE   0xFFFFFF
#define GFX_BLUE    0x2266CC
#define GFX_GRAY    0x8899AA
#define GFX_DGRAY   0x445566
#define GFX_CYAN    0x33CCCC
#define GFX_RED     0xCC3333

/* Инициализация графики из multiboot_info.
   Возвращает 0, если фреймбуфер доступен, и -1 в текстовом режиме. */
int gfx_init(void* mbi_addr);

/* Базовые примитивы */
void gfx_fill_rect(int x, int y, int w, int h, unsigned int color);
void gfx_draw_char(int x, int y, char c, unsigned int color);
void gfx_draw_text(int x, int y, const char* s, unsigned int color);

/* Окна */
typedef struct
{
    int x, y;               /* левый верхний угол */
    int w, h;               /* размеры */
    unsigned int bg;        /* цвет фона окна */
    const char* title;      /* заголовок */
} Window;

void draw_window(Window* win);

/* Перерисовать "рабочий стол": фон + тестовые окна */
void gfx_test_desktop(void);

/* Принудительная перерисовка всего экрана */
void gfx_refresh(void);

/* Доступно ли сейчас графическое окно-консоль? */
int gfx_is_active(void);

/* Печать строки в графическую консоль-окно (для shell) */
void gfx_console_print(const char* s);

#endif
