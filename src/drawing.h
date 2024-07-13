
#pragma once


#ifndef DRAWING_H
#define DRAWING_H

#include <gtk/gtk.h>

#include "electronics.h"


typedef struct {
    DDRD_pos view_position;
    DDRD_color stroke_color;
    DDRD_color fill_color;
    int stroke_width;

    // 1st dimention - elements
    // 2nd dimention - their parts
    // 3rd dimention - points
    DDRD_pos ***points;
} draw_data;


void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data);

gboolean workspace_press_event(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data);

void workspace_release(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data);



void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);

draw_data DDRD_draw_data_proceed(DDRD_circuit *circuit);

#endif