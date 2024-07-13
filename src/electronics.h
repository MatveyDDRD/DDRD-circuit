#pragma once

#ifndef ELECTRONICS_H
#define ELECTRONICS_H

#include <gtk/gtk.h>

#include "DDRD_types.h"

typedef struct {
    DDRD_pos *shape_points;
    DDRD_pos *legs_points;
    DDRD_direction *legs_directions;
    DDRD_pos click_area[4]; // note: 1 element of array = 1 point of area

    int parts_body_number; // how many parts are part of directly element
    int parts_legs_number; // how many parts are part of elements legs
    int *points_in_body_parts_number;
    int *points_in_legs_parts_number;
} DDRD_shape;

typedef enum {
    DDRD_TYPE_RESISTOR,
    DDRD_TYPE_TRANSISTOR
} DDRD_element_type;

typedef struct {
    DDRD_pos position;
    const char* name;
    DDRD_element_type type;
    DDRD_shape shape;
    void *child_element;
} DDRD_element;


typedef struct {
    int max_id;
    DDRD_element** elements;
    DDRD_pos view_position;
    GtkWidget *drawing_area;
} DDRD_circuit;

void func(GtkWidget* area);

bool DDRD_resistor_new(DDRD_circuit *circuit, float resistance, DDRD_pos position);

DDRD_circuit *DDRD_circuit_new(GtkWidget *drawing_area);

#endif