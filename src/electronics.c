#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "DDRD_types.h"
#include "interface.h"
#include "drawing.h"

#define DDRD_DEFAULT_COLORS \
    DDRD_color stroke_color = {255, 0, 0}; \
    DDRD_color fill_color = {255, 0, 255};

// typedef struct {
//     DDRD_pos *shape_points;
//     DDRD_pos *legs_points;
//     DDRD_direction *legs_directions;
//     DDRD_pos click_area[4];
// } DDRD_shape;

/* Default elements shapes */
// точки указаны относительно центра; значения в пикселях

const DDRD_pos resistor_points[4] = {
    {-30, -10},
    {30, -10},
    {30, 10},
    {-30, 10}
};

const int points_in_body_parts_number[1] = {4};

// const DDRD_pos resistor_legs_points[2] = {
//     {-25, 0},
//     {25, 0}
// };

const DDRD_direction resistor_legs_directions[2] = {
    DDRD_LEFT,
    DDRD_RIGHT
};

const DDRD_shape resistor_shape_default = {
    .parts_body_number = 1,
    .click_area = {
        {-15, -10, 30, 20} 
    },
};





// *** Circuit ***
DDRD_circuit *DDRD_circuit_new(GtkWidget *drawing_area) {
    DDRD_print("Making new circuit", 0, GREEN, true);

    DDRD_circuit *circuit = (DDRD_circuit*)malloc(sizeof(DDRD_circuit));
    ALLOC_CHECK(circuit);

    circuit->max_id = 0;
    circuit->elements = NULL;
    circuit->drawing_area = drawing_area;
    circuit->view_position.x = 0;
    circuit->view_position.y = 0;
    circuit->view_zoom = 30;

    // relative to the zero coordinate of the dragged area (not the view's zero coordinate)
    circuit->excluded_dots = NULL;
    circuit->excluded_dots_num = 0;

    return circuit;
}



// *** Circuit manager ***

DDRD_circuit_manager* DDRD_circuit_manager_new(){
    DDRD_print("Making new circuit manager", 0, GREEN, true);
    DDRD_circuit_manager* self = malloc( sizeof(DDRD_circuit_manager) );
    self->circuits = NULL;
    self->circuits_num = 0;
    DDRD_print("circuits_num = %d", 1, WHITE, false, self->circuits_num);
    return self;
}

void managerAddNewCircuit(DDRD_circuit_manager* self, GtkWidget *drawing_area) {
    if (self->circuits_num == 0) {
        self->circuits = (DDRD_circuit**)malloc(sizeof(DDRD_circuit*));
        self->circuits_num = 1;
    } else {
        self->circuits = (DDRD_circuit**)realloc(self->circuits, self->circuits_num * sizeof(DDRD_circuit*));
        self->circuits_num++;
    }
    ALLOC_CHECK(self->circuits);
    printf("   ");
    self->circuits[self->circuits_num - 1] = DDRD_circuit_new(drawing_area);

    DDRD_print("circuits_num = %d", 1, WHITE, false, self->circuits_num);
}




// *** Resistor *** //
typedef struct {
    float resistance;
} DDRD_resistor;

bool DDRD_resistor_new(DDRD_circuit *circuit, float resistance, DDRD_pos position) {
    // Allocate memory for the new resistor element
    DDRD_element *self = (DDRD_element*)malloc(sizeof(DDRD_element));
    ALLOC_CHECK(self);

    self->position = position;
    self->position =  posToGrid(position, circuit->view_zoom);
    DDRD_print("AAA %d %d", 0, RED, position.x);
    self->name = "resistor";
    self->type = DDRD_TYPE_RESISTOR;
    self->id = circuit->max_id;

    // Initialize the shape of the resistor
    DDRD_shape* shape = &self->shape;
    shape->parts_body_number = 1;
    shape->legs_number = 2;
    shape->click_area[0] = (DDRD_pos){-10, -10};
    shape->click_area[1] = (DDRD_pos){-10, 10};
    shape->click_area[2] = (DDRD_pos){10, 10};
    shape->click_area[3] = (DDRD_pos){10, -10};

    // Allocate memory for body parts and shape points
    shape->points_in_body_parts_number = (int*)malloc(sizeof(int) * 4);

    shape->shape_points = (DDRD_pos**)malloc(sizeof(DDRD_pos*)); // only one part
    ALLOC_CHECK(shape->shape_points);

    shape->shape_points[0] = (DDRD_pos*)malloc(sizeof(DDRD_pos) * 4); // and 4 points in part
    ALLOC_CHECK(shape->shape_points[0]);

    shape->points_in_body_parts_number[0] = 4;

    DDRD_print("Initialize points", 0, GREEN, true);
    for (int i = 0; i < 4; ++i) {
        shape->shape_points[0][i].x = resistor_points[i].x + self->position.x; // Copy the values
        shape->shape_points[0][i].y = resistor_points[i].y + self->position.y;
    }

    // Allocate memory for legs points and directions
    shape->legs_points = (DDRD_pos*)malloc(sizeof(DDRD_pos) * 2);
    shape->legs_directions = (DDRD_direction*)malloc(sizeof(DDRD_direction) * 2);
    ALLOC_CHECK(shape->legs_points);
    ALLOC_CHECK(shape->legs_directions);

    for (int i = 0; i < 2; ++i) // 2 legs
    {
        shape->legs_points[i].x = self->position.x;
        shape->legs_points[i].y = self->position.y;
        shape->legs_directions[i] = resistor_legs_directions[i];
    }
    shape->legs_points[0].x += circuit->view_zoom * 2; // 1st leg to right 2nd to left for 2 dots
    shape->legs_points[1].x -= circuit->view_zoom * 2;

    // exclude points under resistor for drawing
    if(circuit->excluded_dots_num == 0){
        circuit->excluded_dots = malloc( sizeof(DDRD_pos) * 3 );
    }else{
        circuit->excluded_dots = realloc(circuit->excluded_dots,
                                         sizeof(DDRD_pos) * ( 3 + circuit->excluded_dots_num));
    }
    circuit->excluded_dots[0] = shape->legs_points[0];
    circuit->excluded_dots[1] = shape->legs_points[1];
    circuit->excluded_dots[2] = self->position;
    circuit->excluded_dots_num += 3;


    // Initialize the resistor-specific properties
    DDRD_resistor* self_resistor = (DDRD_resistor*)malloc(sizeof(DDRD_resistor));
    if (!self_resistor) return false; // Check allocation success
    self_resistor->resistance = resistance;
    self->child_element = self_resistor;

    if (circuit->max_id == 0) {
        circuit->elements = (DDRD_element**)malloc(sizeof(DDRD_element*));
        ALLOC_CHECK(circuit->elements);
    } else {
        circuit->elements = (DDRD_element**)realloc(circuit->elements, 
                             sizeof(DDRD_element*) * (circuit->max_id + 1));
        ALLOC_CHECK(circuit->elements);
    }
    circuit->elements[circuit->max_id] = self;

    circuit->max_id++;

    return true;
}









