#ifndef DDRD_TYPES_H
#define DDRD_TYPES_H

#define ALLOC_CHECK(pointer) \
    if (pointer == NULL) { \
        printf("Fatal error: allocation bruh moment in function %s\n", __func__); \
        exit(EXIT_FAILURE); \
    }

#define ERR_CHECK(func) \
    switch (func) \
    { \
        case DDRD_ERROR: \
            printf("Error in %s\n", __func__); \
            break; \
        case DDRD_FATAL_ERROR: \
            printf("Fatal error in %s\n", __func__); \
            exit(EXIT_FAILURE); \
    } 

// ***
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CHECK_NULL(var) do {                                                  \
    if ((var) == NULL) {                                                      \
        printf("\033[1m\033[31mPANIC! %s IS NULL!\033[0m\n", TOSTRING(var));  \
        exit(-1);                                                             \
    }                                                                         \
} while (0)
// ***

typedef enum {
    DDRD_NORMAL,
    DDRD_ERROR,
    DDRD_FATAL_ERROR,
} DDRD_error;

typedef struct {
    int x;
    int y;
} DDRD_pos;

typedef enum {
    DDRD_RIGHT,
    DDRD_LEFT,
    DDRD_UP,
    DDRD_DOWN
} DDRD_direction;

typedef struct {
    // from 0 to 255
    int r;
    int g;
    int b;
} DDRD_color;

// Перечисление предопределённых ANSI цветов
typedef enum {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
} ANSI_COLOR;

enum file_type {
    TYPE_FILE,
    TYPE_DIRECTORY,
    TYPE_LINK
};

struct fileinfo {
    enum file_type type;
    char *name;
};




#endif
