#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "DDRD_types.h"

#include "electronics.h"

#include "interface.h"

/**
 * structure that have all info for drawing
 * values sets with DDRD_draw_data_proceed(DDRD_circuit *circuit)
 */
typedef struct {
    DDRD_pos view_position;
    DDRD_color stroke_color;
    DDRD_color fill_color;
    DDRD_pos **points;
    int parts_num;
    int *points_num;
    int stroke_width;
} draw_data;

// Mouse press callback
gboolean workspace_press_event(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data) {
    g_print("Click at x: %f, y: %f\n", x, y);
    return GDK_EVENT_PROPAGATE;
}

// Mouse drag callback
void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
    g_print("Dragging to %.2f, %.2f\n", offset_x, offset_y);
}

void workspace_release(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data)
{
    g_print("Release at x: %f, y: %f\n", x, y);
}

// draw_data DDRD_draw_data_proceed(DDRD_circuit *circuit) {

//     printf("    \033[1;92mProceeding data for drawing\033[0m\n");

//     draw_data data;


//     data.stroke_color = (DDRD_color){255, 0, 0};
//     data.fill_color = (DDRD_color){0, 0, 0};
//     data.stroke_width = 2;

//     printf("    Allocaring memory for elements in data.points; max_id = %d\n", circuit->max_id);

//     if(circuit->max_id == 0){
//         DDRD_print("max_id = 0, so we skip proceeding elements", YELLOW, 1, false);
//         return data;
//     }

//     data.points = malloc(circuit->max_id * sizeof(DDRD_pos**));
//     ALLOC_CHECK(data.points);

//     int current_part_index;
//     int current_point_index;

//     int data_last_part_index = 0;

//     DDRD_element **elements = circuit->elements;

//     for (int i = 0; i < (circuit->max_id); ++i) // 1 cycle = 1 element
//     {
//         DDRD_element *current_element = elements[i];
//         // printf("Proceeding element: %s\n", current_element->name);
//         printf("    Proceeding element: \033[0;32m%s\033[0m\n", current_element->name);

//         DDRD_shape *shape = &current_element->shape;
//         int parts_num = 
//             shape->parts_body_number + 
//             shape->parts_legs_number;

//         DDRD_print("parts_num = %d \n", WHITE, 1, false, parts_num);

//         data.parts_num = parts_num;

//         data.points = (DDRD_pos**)malloc( sizeof(DDRD_pos*) * parts_num);
//         ALLOC_CHECK(data.points);

//         data.points_num = (int*)malloc( sizeof(int) * parts_num);

//         int parts_body_num = current_element->shape.parts_body_number;

//             for (current_part_index = 0; current_part_index < parts_num; ++current_part_index)
//             {
//                 printf("     proceeding part number %d\n", current_part_index);

//                 int points_num = current_element->shape.points_in_body_parts_number[current_part_index];


//                 data.points[current_part_index] = (DDRD_pos*)malloc( sizeof(DDRD_pos) * points_num);

//                 for (current_point_index = 0; current_point_index < points_num; ++current_point_index)
//                 {
//                     printf("          proceeding point number: %d; value: ", current_point_index);
//                     data.points[data_last_part_index][current_point_index] = current_element->shape.shape_points[current_point_index];
//                     printf("%d, %d\n",
//                            data.points[data_last_part_index][current_point_index].x,
//                            data.points[data_last_part_index][current_point_index].y);

//                     data.points_num[data_last_part_index]++;
//                 }
//                 data_last_part_index++;
//             }
//     }


//     return data;
// }

draw_data DDRD_draw_data_proceed(DDRD_circuit *circuit) {
    printf("    \033[1;92mProceeding data for drawing\033[0m\n");

    draw_data data = {0}; // Инициализация структуры нулями

    data.stroke_color = (DDRD_color){255, 0, 0};
    data.fill_color = (DDRD_color){0, 0, 0};
    data.stroke_width = 2;

    printf("    Allocaring memory for elements in data.points; max_id = %d\n", circuit->max_id);

    if (circuit->max_id == 0) {
        DDRD_print("max_id = 0, so we skip proceeding elements", YELLOW, 1, false);
        return data;
    }

    data.points = malloc(circuit->max_id * sizeof(DDRD_pos*));
    ALLOC_CHECK(data.points);

    data.parts_num = 0;

    DDRD_element **elements = circuit->elements;

    for (int i = 0; i < circuit->max_id; ++i) // 1 cycle = 1 element
    {
        DDRD_element *current_element = elements[i];
        printf("    Proceeding element: \033[0;32m%s\033[0m\n", current_element->name);

        DDRD_shape *shape = &current_element->shape;
        int parts_num = shape->parts_body_number + shape->parts_legs_number;
        DDRD_print("parts_num = %d \n", WHITE, 1, false, parts_num);

        data.parts_num += parts_num;

        data.points = realloc(data.points, data.parts_num * sizeof(DDRD_pos*));
        ALLOC_CHECK(data.points);

        data.points_num = realloc(data.points_num, data.parts_num * sizeof(int));
        ALLOC_CHECK(data.points_num);

        int data_last_part_index = data.parts_num - parts_num;

        for (int current_part_index = 0; current_part_index < parts_num; ++current_part_index) {
            printf("     proceeding part number %d\n", current_part_index);

            int points_num = current_part_index < shape->parts_body_number ? 
                             shape->points_in_body_parts_number[current_part_index] : 
                             shape->points_in_legs_parts_number[current_part_index - shape->parts_body_number];

            data.points[data_last_part_index + current_part_index] = malloc(points_num * sizeof(DDRD_pos));
            ALLOC_CHECK(data.points[data_last_part_index + current_part_index]);

            data.points_num[data_last_part_index + current_part_index] = points_num;

            for (int current_point_index = 0; current_point_index < points_num; ++current_point_index) {
                printf("          proceeding point number: %d; value: ", current_point_index);
                data.points[data_last_part_index + current_part_index][current_point_index] = current_element->shape.shape_points[current_point_index];
                printf("%d, %d\n",
                       data.points[data_last_part_index + current_part_index][current_point_index].x,
                       data.points[data_last_part_index + current_part_index][current_point_index].y);
            }
        }
    }

    return data;
}


// Draw or redraw cairo area
void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data) {
    printf("\033[1;92mDrawing\033[0m\n");

    CHECK_NULL(user_data);
    DDRD_circuit* circuit = (DDRD_circuit*)user_data;

    DDRD_print("There is a %d elements\n", WHITE, 0, false, circuit->max_id);


    draw_data data = DDRD_draw_data_proceed(circuit);

    // Тестовый код для проверки работы drawing area
    
    // Заполняем фон светло-серым цветом
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_paint(cr);
    
    // Рисуем зеленую линию
    cairo_set_source_rgb(cr, 0, 0, 0.0);
    cairo_set_line_width(cr, 30);

    // printf("parts_num = %d \n", data.parts_num);
    DDRD_print("parts_num = %d \n", WHITE, 1, false, data.parts_num);

    for (int i = 0; i < data.parts_num; ++i)
    {
        printf("        drawing part %d\n", i);
        for (int ii = 0; ii < data.points_num[i]; ++ii)
        {
            printf("           drawing point %d; values: %d %d\n", 
                   ii,
                   data.points[i][ii].x,
                   data.points[i][ii].y);
            if(i == 0){
                cairo_move_to(cr, data.points[i][ii].x, data.points[i][ii].y);
            }else{
                cairo_line_to(cr, data.points[i][ii].x, data.points[i][ii].y);
            }
        }
        cairo_stroke(cr);
    }
    printf("DRAWING COMPLETE\n");
    cairo_set_source_rgb(cr, 1, 0, 0.0);
    cairo_move_to(cr, -2, 0);
    cairo_line_to(cr, 20, 20);
    cairo_line_to(cr, 34, 429);
    cairo_line_to(cr, 43, 248);
    cairo_stroke(cr);
}

