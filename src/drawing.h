
#pragma once


#ifndef DRAWING_H
#define DRAWING_H

#include <gtk/gtk.h>

#include "electronics.h"


// typedef struct {
//     DDRD_pos view_position;
//     DDRD_color stroke_color;
//     DDRD_color fill_color;
//     DDRD_pos **points;
//     int parts_num;
//     int *points_num;
//     int stroke_width;
// } draw_draw_parts_data;


void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_draw_parts_data);

gboolean workspace_press
(GtkGesture *gesture, int n_press, double x, double y, gpointer user_draw_parts_data);

void workspace_release(GtkGesture *gesture, int n_press, double x, double y, gpointer user_draw_parts_data);

inline extern DDRD_pos posToGrid(DDRD_pos pos, int view_zoom);

void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_draw_parts_data);

// draw_draw_parts_data DDRD_draw_draw_parts_data_process(DDRD_circuit *circuit);

#endif