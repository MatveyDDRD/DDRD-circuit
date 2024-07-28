#include <gtk/gtk.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include <time.h>
#include <errno.h>

#include "DDRD_types.h"

#include "electronics.h"

#include "interface.h"

#define DOTS_DISTANCE 30


int precise_delay_ms(unsigned long milliseconds) {
    struct timespec ts;
    int ret;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    do {ret = nanosleep(&ts, &ts);
    } while (ret && errno == EINTR);

    return ret;
}

typedef struct{
    int x;
    int y;
    int width;
    int height;
}DDRD_rectangle;


/**
 * structure that have all info for drawing
 * values sets by DDRD_draw_data_process(DDRD_circuit *circuit)
 */
typedef struct {
    DDRD_pos view_position;

    DDRD_color fill_color;
    int stroke_width;


    // *** PARTS ***
    // Each part is a closed shape made up of connected points.
    // One part represents a single component in the drawing.
    int parts_num;
    int *points_num; // how many points in each part
    // first dimention - parts, second dimention - points
    DDRD_pos **points;
    DDRD_color *stroke_points_color; // only for parts

    // circles
    DDRD_pos* circles_positions;
    int *circles_radiuses;
    int num_circles;

    // rectangles
    DDRD_rectangle *rectangles;
    DDRD_color *rectangles_colors;
    int rectangles_num;

} draw_data;


/*
 * Structure to store the state of the mouse, including click status,
 * starting click position, previous position, view coordinates, 
 * inertial movement, and last drag coordinates.
 */
static struct {
    bool isClicked;
    bool wasDragged;

    double x_start_click;
    double y_start_click;

    double x_previous;
    double y_previous;

    int current_view_x;
    int current_view_y;

    int x_inertion;
    int y_inertion;

    int last_drag_x;
    int last_drag_y;
} mouse_data;



// *** Mouse press callback ***
gboolean workspace_press(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data) {
    DDRD_circuit* circuit = circuit_global;
    mouse_data.wasDragged = false;
    mouse_data.isClicked = true;
    mouse_data.x_start_click = x;
    mouse_data.y_start_click = y;
    mouse_data.current_view_x = circuit->view_position.x;  // Сохраняем текущую позицию вида
    mouse_data.current_view_y = circuit->view_position.y;
    DDRD_print("Click at x: %f, y: %f\n", 0, WHITE, x, y);
    return GDK_EVENT_PROPAGATE;
}

// *** Mouse drag callback ***
void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
    DDRD_circuit* circuit = circuit_global;
    
    // Вычисляем новую позицию вида
    circuit->view_position.x = mouse_data.current_view_x + (int)offset_x;
    circuit->view_position.y = mouse_data.current_view_y + (int)offset_y;

    mouse_data.wasDragged = true;

    DDRD_print("New view position: x=%d, y=%d\n", WHITE, 0, circuit->view_position.x, circuit->view_position.y);
    gtk_widget_queue_draw(circuit->drawing_area);
}

void elementUnselect(DDRD_element *element, DDRD_circuit circuit)
{
    element->isSelected = false;
    element->shape.color = (DDRD_color){0, 0, 0};
}

void elementToggleSelect(DDRD_element *element, DDRD_circuit circuit)
{
    if (element->isSelected)
    {
        elementUnselect(element, circuit);
    }else{
        element->isSelected = true;
        element->shape.color = (DDRD_color){6, 112, 252};
    }
}


void workspace_release(GtkGesture *gesture, int n_press, double x, double y, gpointer user_data) {
    DDRD_print("Release at x: %f, y: %f\n", 0, WHITE, x, y);
    mouse_data.isClicked = false;
    DDRD_circuit *circuit = circuit_global;
    if(mouse_data.wasDragged){ return; }

    for (int i = 0; i < circuit->max_id; ++i)
    {
        DDRD_pos click_area[2];
        click_area[0].x = circuit->elements[i]->shape.click_area[0].x + 
        circuit->view_position.x + 
        circuit->elements[i]->position.x;

        click_area[0].y = circuit->elements[i]->shape.click_area[0].y + 
        circuit->view_position.y + 
        circuit->elements[i]->position.y;

        click_area[1].x = circuit->elements[i]->shape.click_area[1].x + 
        circuit->view_position.x + 
        circuit->elements[i]->position.x;

        click_area[1].y = circuit->elements[i]->shape.click_area[1].y + 
        circuit->view_position.y + 
        circuit->elements[i]->position.y;
        if (x >= click_area[0].x && x <= click_area[1].x &&
        y >= click_area[0].y && y <= click_area[1].y)
        {
            printf("AAAA %d\n", circuit->elements[i]->id);
            elementToggleSelect(circuit->elements[i], *circuit);

            for (int ii = 0; ii < circuit->max_id; ++ii)
            {
                if (ii != i)
                {
                    elementUnselect(circuit->elements[ii], *circuit);
                }
            }
            break;
        }else{
            for (int ii = 0; ii < circuit->max_id; ++ii)
            {
                elementUnselect(circuit->elements[ii], *circuit);
            }
        }
    }
    gtk_widget_queue_draw(circuit->drawing_area);
}


inline extern DDRD_pos posToGrid(DDRD_pos pos, int view_zoom)
{
  if (view_zoom == 0) {
    return (DDRD_pos){0, 0};
  }
  DDRD_pos ret;

  int original_x = pos.x;
  int original_y = pos.y;

  int dotsX = pos.x / view_zoom;
  ret.x = dotsX * view_zoom;

  int dotsY = pos.y / view_zoom;
  ret.y = dotsY * view_zoom;

  return ret;
}

inline static void gridDotsProcess(draw_data *data, 
                                   int zoom, 
                                   int width, 
                                   int height, 
                                   DDRD_pos view_position, 
                                   DDRD_pos *excluded_dots, 
                                   int excluded_dots_num)
{
    width += 60;
    height += 60;
    int num_dots_x = (width) / DOTS_DISTANCE;
    int num_dots_y = (height) / DOTS_DISTANCE;
    int num_dots = num_dots_x * num_dots_y;

    DDRD_pos local_view_pos;
    local_view_pos.x = view_position.x % DOTS_DISTANCE;
    local_view_pos.y = view_position.y % DOTS_DISTANCE;

    data->circles_positions = malloc(sizeof(DDRD_pos) * num_dots);
    data->circles_radiuses = malloc(sizeof(int) * num_dots);
    data->num_circles = num_dots;
    ALLOC_CHECK(data->circles_positions);
    ALLOC_CHECK(data->circles_radiuses);


    DDRD_pos* circles = data->circles_positions;

    for (int i = 0; i < num_dots_x; ++i)
    {
        for (int ii = 0; ii < num_dots_y; ++ii)
        {
            int index = i * num_dots_y + ii;
            circles[index].x = i * DOTS_DISTANCE;
            circles[index].y = ii * DOTS_DISTANCE;

            data->circles_radiuses[i] = 5;
        }
    }
    for (int i = 0; i < num_dots_x; ++i)
    {
        for (int ii = 0; ii < num_dots_y; ++ii)
        {
            int index = i * num_dots_y + ii;
            circles[index].x = i * DOTS_DISTANCE + local_view_pos.x;
            circles[index].y = ii * DOTS_DISTANCE + local_view_pos.y;

            if (circles[index].x < 0 || circles[index].x > width ||
                circles[index].y < 0 || circles[index].y > height) {
                data->circles_radiuses[index] = 0;
            }
            for (int iii = 0; iii < excluded_dots_num; ++iii)
            {
                if (excluded_dots[iii].x == circles[index].x && excluded_dots[iii].y == circles[index].y)
                {
                    data->circles_radiuses[index] = 0;
                }
            }
        }
    }
}

inline static void pointOfLegProcess(DDRD_pos *point, DDRD_direction direction)
{
    DDRD_print("processing leg", 3, WHITE);

    

    switch (direction) {
        case DDRD_RIGHT:
            point->x += 30; // later
            break;
        case DDRD_LEFT:
            point->x -= 30; // later
            break;
        case DDRD_UP:
            point->y += 30; // later
            break;
        case DDRD_DOWN:
            point->y -= 30; // later
            break;
    }
}

inline static void elementprocess(DDRD_shape shape, draw_data *data, DDRD_circuit* circuit)
{
    // CHECK_NULL(element->name);
    DDRD_print("processing element", 2, GREEN);


    // *** Processing body of elements ***
    // allocating memory for points_num and points
    if (data->parts_num == 0){
        DDRD_print("allocating memory for %d elements", 3, WHITE, shape.parts_body_number);
        DDRD_print("points_num", 4, WHITE, false);
        data->points_num = (int*)malloc( sizeof(int) *  shape.parts_body_number);
        data->points = (DDRD_pos**)malloc( sizeof(DDRD_pos) * shape.parts_body_number);
        ALLOC_CHECK(data->points_num);
        ALLOC_CHECK(data->points);

        data->stroke_points_color = malloc(sizeof(DDRD_color) * (shape.parts_body_number + shape.legs_number));

    }else if(data->parts_num > 0)
    {
        DDRD_print("reallocating memory for +%d elements", 3, WHITE, shape.parts_body_number);
        data->points_num = (int*)realloc(data->points_num,  
                                         sizeof(int) * (data->parts_num + shape.parts_body_number));
        ALLOC_CHECK(data->points_num);
        DDRD_print("points", 4, WHITE, false);
        data->points = (DDRD_pos**)realloc(data->points, 
                                           sizeof(DDRD_pos) * (data->parts_num + shape.parts_body_number));

        data->stroke_points_color = realloc(data->stroke_points_color,
                                            sizeof(DDRD_color) * (shape.parts_body_number + shape.legs_number + data->parts_num));
    }
    for (int i = 0; i < shape.parts_body_number; ++i)
    {
        data->points[i + data->parts_num] = (DDRD_pos*)malloc(sizeof(DDRD_pos) * shape.points_in_body_parts_number[i]);
    }

    // proceeding points_num and colors
    for (int i = 0; i < shape.parts_body_number; ++i)
    {
        data->points_num[i + data->parts_num] = shape.points_in_body_parts_number[i];
        data->stroke_points_color[i + data->parts_num] = shape.color;
    }

    // proceeding points
    DDRD_print("processing points", 3, WHITE, true);
    for (int i = 0; i < shape.parts_body_number; ++i) // parts
    {
        DDRD_print("processing part %d", 4, WHITE, i);
        for (int ii = 0; ii < shape.points_in_body_parts_number[i]; ++ii) // points
        {
            data->points[i + data->parts_num][ii].x = shape.shape_points[i][ii].x;
            data->points[i + data->parts_num][ii].y = shape.shape_points[i][ii].y;

            DDRD_print("processing point %d: values: %d, %d", 
                       5, WHITE, ii, 
                       data->points[i + data->parts_num][ii].x, 
                       data->points[i + data->parts_num][ii].y);
        }
    }

    data->parts_num += shape.parts_body_number;


    // *** Processing legs of elements ***
    
    // for points_num 
    if (data->parts_num == 0) {
        data->points_num = malloc(sizeof(DDRD_pos**) *  shape.legs_number);
    } else {
        data->points_num = realloc(data->points_num, 
                                   (data->parts_num * sizeof(DDRD_pos**)) * (sizeof(DDRD_pos**) * shape.legs_number));
    }
    ALLOC_CHECK(data->points_num);

    for (int i = 0; i < shape.legs_number; ++i)
    {
        data->points_num[data->parts_num + i] = 2; // always 2 points in leg
    }

    // for points
    if (data->parts_num == 0)
    {
        data->points = malloc( sizeof(DDRD_pos*) * shape.legs_number);
    } else {
        data->points = realloc(data->points,
                               sizeof(DDRD_pos*) * (data->parts_num + shape.legs_number));
    }
    ALLOC_CHECK(data->points);

    for (int i = 0; i < shape.legs_number; ++i)
    {
        data->stroke_points_color[data->parts_num + i] = shape.color;
        data->points[data->parts_num + i] = malloc(sizeof(DDRD_pos) * 2);
        ALLOC_CHECK(data->points_num);


        data->points[data->parts_num + i][0] = (DDRD_pos){
            .x = shape.legs_points[i].x,
            .y = shape.legs_points[i].y
        };

        data->points[data->parts_num + i][1] = (DDRD_pos){
            .x = shape.legs_points[i].x,
            .y = shape.legs_points[i].y
        };
        pointOfLegProcess(&data->points[data->parts_num +i][1], shape.legs_directions[i]);

    }

    data->parts_num += shape.legs_number;
}


/*
 * processes draw_data
 * 
 * draw_data contains parts and points within it.
 * Each part is a closed shape made up of connected points.
 * One part represents a single component in the drawing.
 */
draw_data DDRD_draw_data_process(DDRD_circuit *circuit, int width, int height) {
    DDRD_print("processing data for drawing", 1, GREEN, true);

    DDRD_element **elements = circuit->elements;

    draw_data data = {0};
    data.parts_num = 0;
    data.fill_color = (DDRD_color){0, 0, 0};
    data.stroke_width = 2;


    // count parts in all elements
    int allocate_parts_num = 0;
    for (int i = 0; i < circuit->max_id; ++i)
    {
        allocate_parts_num += elements[i]->shape.parts_body_number;
        allocate_parts_num += elements[i]->shape.legs_number;
    }
    

    // allocate memory for 1st dimention in data->points
    data.points = (DDRD_pos**)malloc( sizeof(DDRD_pos*) * allocate_parts_num);


    data.points_num = (int*)malloc(allocate_parts_num * sizeof(int));
    ALLOC_CHECK(data.points_num);


    // *** Processing elements ***
    DDRD_print("max_id = %d", 2, WHITE, false, circuit->max_id);

    // procees the elements
    for (int i = 0; i < circuit->max_id; ++i)
    {
        elementprocess(elements[i]->shape, &data, circuit);
    }


    gridDotsProcess(&data,circuit->view_zoom, 
                    width, 
                    height, 
                    circuit->view_position, 
                    circuit->excluded_dots, 
                    circuit->excluded_dots_num);

    return data;
}



/* 
 * The function uses the draw_data structure to perform the drawing.
 * The draw_data structure is defined in the drawing.c file.
 * 
 * The function DDRD_draw_data_process processes this structure
 * to prepare it for drawing, and is called within the draw_function.
 */
void draw_function(GtkDrawingArea *area, 
                   cairo_t *cr, 
                   int width, 
                   int height, 
                   gpointer data_for_drawing) {

    DDRD_print("Drawing", 0, GREEN, true);

    // make circuit pointer from global pointer
    CHECK_NULL(circuit_global);
    DDRD_circuit* circuit = circuit_global;

    draw_data data = DDRD_draw_data_process(circuit, width, height);

    DDRD_print("Drawing canvas", 1, GREEN, false);
    DDRD_print("There is a %d elements", 2, WHITE, false, circuit->max_id);

    // background
    cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
    cairo_paint(cr);

    // Draw dots
    for (int i = 0; i < data.num_circles; ++i) {
        // Assuming dot_positions is an array of DDRD_pos
        DDRD_pos dot_pos = data.circles_positions[i];
        // Assuming dot_radius is an integer
        // int dot_radius = data.circles_radiuses[i];
        int dot_radius = 2; // temp

        cairo_set_source_rgb(cr, 0.34, 0.34, 0.34); // Example: red color
        cairo_arc(cr, 
                  dot_pos.x,
                  dot_pos.y, 
                  dot_radius, 0, 2 * M_PI);
        cairo_fill(cr);
    }


    // set windh for lines
    cairo_set_line_width(cr, 5);

    // move points appending to view position
    for (int i = 0; i < data.parts_num; ++i)
    {
        for (int ii = 0; ii < data.points_num[i]; ++ii)
        {
            data.points[i][ii].x += circuit->view_position.x;
            data.points[i][ii].y += circuit->view_position.y;
        }
    }


    // draw parts from data to workspace_area
    for (int i = 0; i < data.parts_num; ++i) // parts
    {
        DDRD_print("drawing part %d", 2, WHITE, i);
        float r = (float)data.stroke_points_color[i].r / 255;
        float g = (float)data.stroke_points_color[i].g / 255;
        float b = (float)data.stroke_points_color[i].b / 255;

        cairo_set_source_rgb(cr, r, g, b);
        for (int ii = 0; ii < data.points_num[i]; ++ii) // points
        {
            DDRD_print("drawing point %d; values: %d %d;  ", 
                       3, WHITE, ii, data.points[i][ii].x, data.points[i][ii].y);

            // if its first point in the part, cairo_move_to(), else cairo_line_to
            if (ii == 0) {
                DDRD_print("Move to", 4, MAGENTA, false);
                cairo_move_to(cr, data.points[i][ii].x, data.points[i][ii].y);
            } else {
                cairo_line_to(cr, data.points[i][ii].x, data.points[i][ii].y);
                DDRD_print("Line to", 4, YELLOW, false);
            }
        }
        cairo_close_path(cr);
        cairo_stroke(cr);
        DDRD_print("stroke", 1, GREEN, false);
    }

    data.rectangles = NULL;
    data.rectangles_colors = NULL;
    data.rectangles_num = 0;


    // Free allocated memory for the points array and points_num array
    for (int i = 0; i < data.parts_num; i++) {
        free(data.points[i]);
    }
    free(data.rectangles);
    free(data.rectangles_colors);
    free(data.points);
    free(data.points_num);
    free(data.circles_positions);
    free(data.circles_radiuses);
    free(data.stroke_points_color);

    DDRD_print("DRAWING COMPLETE", 0, BLUE, true);

}