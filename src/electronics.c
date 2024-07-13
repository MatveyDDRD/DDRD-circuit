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
DDRD_pos resistor_shape_default_points[4] = {
    { -10, -5 },
    { 10, -5 },
    { 10, 5 },
    { -10, 5 }
};

// координаты точки - начало ножки
DDRD_pos resistor_legs_points_default[2] = {
    { -10, -5 },
    { 10, -5 }
};

DDRD_direction resistor_legs_directions_default[2] = {
    DDRD_RIGHT,
    DDRD_LEFT
};

// area user should click; should be little bigger than resistor 
DDRD_pos resistor_click_area_default[4] = {
    { -15, -10 },
    { 15, -10 },
    { 15, 10 },
    { -15, 10 }
};

int points_in_body_parts_number_default[] = {4};

int points_in_legs_parts_number_default[] = {1, 1};



// *** Circuit ***
DDRD_circuit *DDRD_circuit_new(GtkWidget *drawing_area) {
    printf("\033[1;92mMaking new circuit \033[0m\n");
    DDRD_circuit *circuit = (DDRD_circuit*)malloc( sizeof(DDRD_circuit) );
    circuit->max_id = 0;
    circuit->elements = NULL;
    circuit->drawing_area = drawing_area;
    circuit->view_position.x = 0;
    circuit->view_position.y = 0;

    return circuit;
}

// *** Resistor *** //
typedef struct {
    float resistance;
} DDRD_resistor;

bool DDRD_resistor_new(DDRD_circuit *circuit, float resistance, DDRD_pos position) {
    DDRD_print("Making new resistor, resistance = %d", GREEN, 0, true, (int)resistance);
    circuit->max_id++;
    int max_id = circuit->max_id;

    circuit->elements = (DDRD_element**)malloc( sizeof(DDRD_element*) * 10 );

    // Создаем новый элемент
    DDRD_element* new_element = (DDRD_element*)malloc(sizeof(DDRD_element));
    ALLOC_CHECK(new_element);

    // Создаем дочерний элемент резистор
    DDRD_resistor* new_resistor = (DDRD_resistor*)malloc(sizeof(DDRD_resistor));
    ALLOC_CHECK(new_resistor);



    // Инициализируем shape
    DDRD_shape resistor_default_shape = {
        .shape_points = resistor_shape_default_points,
        .legs_points = resistor_legs_points_default,
        .legs_directions = resistor_legs_directions_default,
        .click_area = {
            { -15, -10 },
            { 15, -10 },
            { 15, 10 },
            { -15, 10 }
        },
        .parts_body_number = 1,
        .parts_legs_number = 2,
        .points_in_body_parts_number = points_in_body_parts_number_default,
        .points_in_legs_parts_number = points_in_legs_parts_number_default
    };


    new_resistor->resistance = resistance;
    new_element->position = position;
    new_element->name = "Resistor";
    new_element->type = DDRD_TYPE_RESISTOR;
    new_element->shape = resistor_default_shape;
    new_element->child_element = (void*)new_resistor;

    // Присваиваем элемент массиву
    circuit->elements[max_id - 1] = new_element;

    gtk_widget_queue_draw(circuit->drawing_area);

    return true;
}

// Main function or entry point
void func(GtkWidget* area) { 
    DDRD_circuit *circuit = DDRD_circuit_new(area);
    printf("Made in func\n");
    DDRD_pos pos = {100, 100};
    DDRD_resistor_new(circuit, 220.0, pos);


//     // for (int i = 0; i < circuit->max_id; ++i) {
//     //     DDRD_element* element = circuit->elements[i];
//     //     printf("Element: %s at (%d, %d)\n", element->name, element->position.x, element->position.y);
}
// }