#ifndef INTERFACE_H
#define INTERFACE_H

#include <gtk/gtk.h>

#include "DDRD_types.h"
#include "electronics.h"


void activate(GtkApplication *app, gpointer user_draw_parts_data);

gpointer draw_func_draw_parts_data;

// const DDRD_color ANSI_COLORS[];

const char* get_nearest_color_code(DDRD_color color);

int DDRD_print(const char *format, ANSI_COLOR colorw, int level, ...);

extern DDRD_circuit* circuit_global;

#endif
