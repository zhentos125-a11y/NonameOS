#ifndef VFS_H
#define VFS_H

/* ===== Виртуальная ФС в оперативной памяти =====
   Никакого диска нет: всё живёт в фиксированном массиве структур. */

#define FS_FILE   0
#define FS_FOLDER 1

#define MAX_ENTRIES 32   /* максимум файлов+папок */
#define NAME_LEN    16
#define CONTENT_LEN 128

typedef struct
{
    char name[NAME_LEN];
    int type;                 /* FS_FILE или FS_FOLDER */
    int parent;               /* индекс родительской папки (-1 = корень) */
    char content[CONTENT_LEN]; /* текст внутри файла */
    int used;                 /* слот занят? */
} FSEntry;

void vfs_init(void);
int  create_file(const char* name, int parent, const char* text);
int  create_folder(const char* name, int parent);
/* Список детей указанной папки в консоль (parent = -1 для корня) */
void list_directory(int parent);
/* Помощник для команды "cat" */
const char* vfs_read_file(const char* name, int parent);

#endif
