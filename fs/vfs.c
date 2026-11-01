#include "vfs.h"

extern void print(const char* str);
extern void newline();

static FSEntry fs[MAX_ENTRIES];

static int str_eq(const char* a, const char* b)
{
    while (*a && *b)
    {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static void str_copy(char* dst, const char* src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++)
    {
        dst[i] = src[i];
    }
    dst[i] = 0;
}

static int alloc_slot(void)
{
    int i;
    for (i = 0; i < MAX_ENTRIES; i++)
    {
        if (!fs[i].used) return i;
    }
    return -1; /* память ФС закончилась */
}

void vfs_init(void)
{
    int i;

    for (i = 0; i < MAX_ENTRIES; i++)
    {
        fs[i].used = 0;
    }

    /* Слот 0 — корневая папка "/" */
    fs[0].used = 1;
    fs[0].type = FS_FOLDER;
    fs[0].parent = -1;
    str_copy(fs[0].name, "/", NAME_LEN);

    /* Немного тестового содержимого */
    create_folder("docs", 0);
    create_file("readme.txt", 0, "Hello from MyOS RAM filesystem!");
    create_file("todo.txt", 1, "1. Draw pixels 2. Make files");
}

int create_file(const char* name, int parent, const char* text)
{
    int slot = alloc_slot();

    if (slot < 0) return -1;

    str_copy(fs[slot].name, name, NAME_LEN);
    fs[slot].type = FS_FILE;
    fs[slot].parent = parent;
    str_copy(fs[slot].content, text, CONTENT_LEN);
    fs[slot].used = 1;

    return slot;
}

int create_folder(const char* name, int parent)
{
    int slot = alloc_slot();

    if (slot < 0) return -1;

    str_copy(fs[slot].name, name, NAME_LEN);
    fs[slot].type = FS_FOLDER;
    fs[slot].parent = parent;
    fs[slot].content[0] = 0;
    fs[slot].used = 1;

    return slot;
}

void list_directory(int parent)
{
    int i;

    newline();
    print("Directory of ");
    print(fs[parent].name);
    print(":");

    for (i = 0; i < MAX_ENTRIES; i++)
    {
        if (fs[i].used && fs[i].parent == parent)
        {
            newline();
            print("  ");
            print(fs[i].name);
            print(fs[i].type == FS_FOLDER ? "  [DIR]" : "  [FILE]");
        }
    }

    newline();
}

const char* vfs_read_file(const char* name, int parent)
{
    int i;

    for (i = 0; i < MAX_ENTRIES; i++)
    {
        if (fs[i].used && fs[i].type == FS_FILE &&
            fs[i].parent == parent && str_eq(fs[i].name, name))
        {
            return fs[i].content;
        }
    }

    return 0;
}
