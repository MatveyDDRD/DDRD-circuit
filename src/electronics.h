#pragma once

#ifndef ELECTRONICS_H
#define ELECTRONICS_H

#include <gtk/gtk.h>

#include "DDRD_types.h"

typedef struct {
    int parts_body_number; // how many parts are part of directly element
    int *points_in_body_parts_number;
    DDRD_pos **shape_points;

   
    // note: leg position is one of its points, that touches the body
    // second point of the leg calculated based on legs_directions
    int legs_number; // how many parts are part of elements legs
    DDRD_pos *legs_points;
    DDRD_direction *legs_directions;
    DDRD_pos click_area[4]; // note: 1 element of array = 1 point of area
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
    int id; // self id
} DDRD_element;

// ***

typedef struct {
    int max_id; // how many elements
    DDRD_element** elements;

    GtkWidget *drawing_area;
    DDRD_pos view_position;

    int view_zoom; // dots distance
    DDRD_pos* excluded_dots;
    int excluded_dots_num;
} DDRD_circuit;

DDRD_circuit *DDRD_circuit_new(GtkWidget *drawing_area);

typedef struct {
    DDRD_circuit** circuits;
    int circuits_num;
}DDRD_circuit_manager;

DDRD_circuit_manager* DDRD_circuit_manager_new();

void managerAddNewCircuit(DDRD_circuit_manager* self, GtkWidget *drawing_area);

// ***

void func(GtkWidget* area);

bool DDRD_resistor_new(DDRD_circuit *circuit, float resistance, DDRD_pos position);


#endif