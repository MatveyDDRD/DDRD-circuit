#include <time.h>
#include "drawing.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <gtk/gtk.h>

#include "DDRD_types.h"
#include "electronics.h"

#define DEBUG_MODE 0

// App
GtkWidget *window;
GtkWidget *main_vertical_box;
GtkWidget *workspace_paned;
GtkWidget *workspace_frame;

GtkWidget *sidebar_frame;
GtkWidget *sidebar_stack_container;
GtkWidget *sidebar_stack_switcher;
GtkWidget *sidebar_stack;

// File list
GtkWidget *file_list_container;
GtkWidget **file_button;
GtkWidget *file_list_scrolled;
GtkWidget *file_list_widget;

char *current_directory = ".";
struct fileinfo *file_info_array;
int file_count_in_current_directory;

// Menu
GSimpleAction *open_file_action;
GSimpleAction *new_file_action;

GSimpleAction *act_new_file;
GSimpleAction *act_open_file;

GMenu *menu_bar;
GMenu *menu;
GMenu *new_menu;

GMenuItem *menu_item_new_file;
GMenuItem *menu_item_open_file;

// Cairo
GtkWidget *workspace_area;

DDRD_circuit* circuit_global;

// circuit



//***** PRINT ****//

// Получить строку кода ANSI по перечислению
const char *get_color_code(ANSI_COLOR color) {
    switch (color) {
        case RED:     return "\033[91m";
        case GREEN:   return "\033[92m";
        case YELLOW:  return "\033[93m";
        case BLUE:    return "\033[94m";
        case MAGENTA: return "\033[95m";
        case CYAN:    return "\033[96m";
        case BLACK:   return "\033[90m";
        case WHITE:   return "\033[97m";
        default:      return "\033[97m";
    }
}


#if DEBUG_MODE

extern inline void DDRD_print(const char *format, int level, ANSI_COLOR color, int bold, ...) {

    va_list args;
    va_start(args, bold);

    // skip first arg, because without it, function also prints bold arg value
    // va_arg(args, int);

    // Установить цвет и стиль текста
    const char *color_code = get_color_code(color);
    const char *bold_code = bold ? "\033[1m" : "\033[22m";
    if (color_code == NULL || bold_code == NULL) {
        va_end(args);
        printf("DDRD_print error");
        return;
    }

    // Вывести табуляции в зависимости от уровня
    for (int i = 0; i < level; i++) {
        printf("|    ");
    }

    // Установить цвет и стиль текста
    printf("%s%s", bold_code, color_code);

    // Создаем буфер для форматированной строки
    char buffer[1024];  // Предполагаем, что 1024 байт достаточно. Измените при необходимости.

    // Форматируем строку в буфер
    int result = vsnprintf(buffer, sizeof(buffer), format, args);

    // Выводим отформатированную строку
    if (result >= 0) {
        printf("%s", buffer);
    }

    // Сбросить форматирование
    printf("\033[0m\n");

    va_end(args);
    return;
}

#else

extern inline void DDRD_print(const char *format, int level, ANSI_COLOR color, int bold, ...) {
    // Do nothing
}

#endif


//***** END PRINT *****//

// *** Functions definitions ***

void new_file_action_callback(GSimpleAction *action, GVariant *parameter, gpointer user_draw_parts_data) {
    printf("Hi\n");
}

static void count_files_in_directory() {
    DIR *dir;
    struct dirent *entry;
    file_count_in_current_directory = 0;

    dir = opendir(current_directory);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        file_count_in_current_directory++;
    }
    closedir(dir);
}

static void list_directory() {
    DIR *d;
    struct dirent *dir;
    d = opendir(current_directory);

    if (d == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    count_files_in_directory();
    file_info_array = (struct fileinfo *)malloc(file_count_in_current_directory * sizeof(struct fileinfo));

    int i = 0;
    while ((dir = readdir(d)) != NULL) {
        file_info_array[i].name = strdup(dir->d_name);
        // printf("%s\n", file_info_array[i].name);
        i++;
    }
    closedir(d);
}

// *** File list ***

// Callbacks for setting up, binding, unbinding, and tearing down list items
static void setup_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_draw_parts_data){
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(listitem, label);
    // No need to unref label due to sunk reference
}

static void bind_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_draw_parts_data) {
    GtkWidget *label = gtk_list_item_get_child(listitem);
    GtkStringObject *str_obj = gtk_list_item_get_item(listitem);
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(str_obj));
}

static void unbind_list_item_cb(GtkSignalListItemFactory *self, 
                                GtkListItem *listitem, 
                                gpointer user_draw_parts_data) {
    // Nothing to do here 
}

static void teardown_list_item_cb(GtkSignalListItemFactory *self, 
                                  GtkListItem *listitem, 
                                  gpointer user_draw_parts_data) {
    // Nothing to do here, GtkListItem will be destroyed
}

// Function to create the file list view
void make_file_tree() {
    list_directory();

    char **names_array = (char **)malloc(sizeof(char *) * (file_count_in_current_directory + 1)); 

    for (int i = 0; i < file_count_in_current_directory; ++i) {
        names_array[i] = file_info_array[i].name;
    }

    GtkStringList *file_string_list = gtk_string_list_new((const char * const *)names_array);
    GtkNoSelection *file_list_no_selection = gtk_no_selection_new(G_LIST_MODEL(file_string_list));

    GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(setup_list_item_cb), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(bind_list_item_cb), NULL);
    // Optional: unbind and teardown callbacks can be omitted
    // g_signal_connect(factory, "unbind", G_CALLBACK(unbind_list_item_cb), NULL);
    // g_signal_connect(factory, "teardown", G_CALLBACK(teardown_list_item_cb), NULL);

    // Create ListView
    file_list_widget = gtk_list_view_new(GTK_SELECTION_MODEL(file_list_no_selection), factory);
}




    // *** Menu Bar ***

    // menu_bar
    // │
    // ├── File
    // │   ├── New File
    // │   └── Open File
    // │
    // └── Network
    //     └── Server
    //         ├── Connect
    //         └── Disconnect

// *** Application activation function **
void activate(GtkApplication *app, gpointer user_draw_parts_data) {
    printf("Activating application...\n");

    // *** Main window ***
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);

    GtkWidget *main_vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);





    // Создаем главное меню
    GMenu *menu_bar = g_menu_new();

    // Создаем подменю "File"
    GMenu *file_menu = g_menu_new();
    g_menu_append_submenu(menu_bar, "File", G_MENU_MODEL(file_menu));

    // Создаем подменю "Network"
    GMenu *network_menu = g_menu_new();
    g_menu_append_submenu(menu_bar, "Network", G_MENU_MODEL(network_menu));

    // Создаем подменю "Server" внутри подменю "Network"
    GMenu *server_menu = g_menu_new();
    g_menu_append_submenu(network_menu, "Server", G_MENU_MODEL(server_menu));


    // Создаем действия "Connect" и "Disconnect"
    GSimpleAction *act_connect = g_simple_action_new("connect", NULL);
    GSimpleAction *act_disconnect = g_simple_action_new("disconnect", NULL);

    // Добавляем действия в карту действий приложения
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_connect));
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_disconnect));

    // Подключаем сигналы активации к действиям
    g_signal_connect(act_connect, "activate", G_CALLBACK(new_file_action_callback), NULL);
    g_signal_connect(act_disconnect, "activate", G_CALLBACK(new_file_action_callback), NULL);

    // Создаем пункт меню "Connect" и добавляем его в подменю "Server"
    GMenuItem *menu_item_connect = g_menu_item_new("Connect", "app.connect");
    g_menu_append_item(server_menu, menu_item_connect);

    // Создаем пункт меню "Disconnect" и добавляем его в подменю "Server"
    GMenuItem *menu_item_disconnect = g_menu_item_new("Disconnect", "app.disconnect");
    g_menu_append_item(server_menu, menu_item_disconnect);

    // Создаем действия "New File" и "Open File"
    GSimpleAction *act_new_file = g_simple_action_new("newfile", NULL);
    GSimpleAction *act_open_file = g_simple_action_new("openfile", NULL);

    // Добавляем действия в карту действий приложения
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_new_file));
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act_open_file));

    // Подключаем сигналы активации к действиям
    g_signal_connect(act_new_file, "activate", G_CALLBACK(new_file_action_callback), NULL);
    g_signal_connect(act_open_file, "activate", G_CALLBACK(new_file_action_callback), NULL);

    // Создаем пункт меню "New File" и добавляем его в подменю "File"
    GMenuItem *menu_item_new_file = g_menu_item_new("New File", "app.newfile");
    g_menu_append_item(file_menu, menu_item_new_file);

    // Создаем пункт меню "Open File" и добавляем его в подменю "File"
    GMenuItem *menu_item_open_file = g_menu_item_new("Open File", "app.openfile");
    g_menu_append_item(file_menu, menu_item_open_file);

    // Устанавливаем главное меню для приложения и показываем его
    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu_bar));
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window), TRUE);



    // *** Paned ***
    GtkWidget *workspace_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *sidebar_frame = gtk_frame_new(NULL);
    GtkWidget *workspace_frame = gtk_frame_new(NULL);

    gtk_widget_set_size_request(workspace_paned, 200, -1);
    gtk_box_append(GTK_BOX(main_vertical_box), workspace_paned);

    gtk_paned_set_start_child(GTK_PANED(workspace_paned), sidebar_frame);
    gtk_paned_set_resize_start_child(GTK_PANED(workspace_paned), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(workspace_paned), FALSE);
    gtk_widget_set_size_request(sidebar_frame, 50, -1);

    gtk_paned_set_end_child(GTK_PANED(workspace_paned), workspace_frame);
    gtk_paned_set_resize_end_child(GTK_PANED(workspace_paned), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(workspace_paned), FALSE);
    gtk_widget_set_size_request(workspace_frame, 50, -1);



    // *** Sidebar stack container ***
    GtkWidget *sidebar_stack_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *sidebar_stack_switcher = gtk_stack_switcher_new();
    gtk_widget_set_halign(sidebar_stack_switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(sidebar_stack_switcher, GTK_ALIGN_START);
    GtkWidget *sidebar_stack = gtk_stack_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(sidebar_stack_switcher), GTK_ORIENTATION_VERTICAL);

    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(sidebar_stack_switcher), GTK_STACK(sidebar_stack));
    gtk_widget_set_vexpand(sidebar_stack, TRUE);

    gtk_box_append(GTK_BOX(sidebar_stack_container), sidebar_stack_switcher);
    gtk_box_append(GTK_BOX(sidebar_stack_container), sidebar_stack);

    gtk_frame_set_child(GTK_FRAME(sidebar_frame), sidebar_stack_container);



    // *** File list container ***
    GtkWidget *file_list_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_vexpand(file_list_container, TRUE);
    gtk_widget_set_hexpand(file_list_container, TRUE);

    make_file_tree();

    // Создание scrolled window и добавление в него file_list_widget
    GtkWidget *file_list_scrolled = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_list_scrolled), 
                                                       GTK_POLICY_AUTOMATIC, 
                                                       GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(file_list_scrolled), file_list_widget);
    gtk_widget_set_vexpand(file_list_scrolled, TRUE);
    gtk_widget_set_hexpand(file_list_scrolled, TRUE);
    gtk_box_append(GTK_BOX(file_list_container), file_list_scrolled);

    // Добавление контейнера с file_list в stack
    gtk_stack_add_titled(GTK_STACK(sidebar_stack), file_list_container, "file_list", "Files");



    // *** Elements container ***
    GtkWidget *elements_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_vexpand(elements_container, TRUE);
    gtk_widget_set_hexpand(elements_container, TRUE);

    // Добавление контейнера с elements в stack
    gtk_stack_add_titled(GTK_STACK(sidebar_stack), elements_container, "elements", "Elements");



    // *** Canvas drawing ***

    gpointer draw_parts_data_for_drawing;

    GtkWidget *workspace_area = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(workspace_area), 500);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(workspace_area), 500);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(workspace_area), 
                                   draw_function, 
                                   draw_parts_data_for_drawing, 
                                   NULL);

    gtk_widget_set_vexpand(workspace_area, TRUE);
    gtk_widget_set_hexpand(workspace_area, TRUE);
    gtk_frame_set_child(GTK_FRAME(workspace_frame), workspace_area);


    // // *** Circuit init ***
    // DDRD_circuit* circuit = DDRD_circuit_new(workspace_area);

    // circuit_global = circuit;

    // DDRD_pos pos1 = { 38, 42 };
    // DDRD_pos pos2 = { 568, 200 };
    // DDRD_resistor_new(circuit, 220, pos1);
    // DDRD_resistor_new(circuit, 230, pos2);
    // 
    // *** Circuit init ***
    DDRD_circuit* circuit = DDRD_circuit_new(workspace_area);

    circuit_global = circuit;

    // Добавляем больше резисторов с различными номиналами и позициями
    DDRD_pos pos1 = { 932, 0 };
    DDRD_pos pos2 = { 35, 35 };
    DDRD_pos pos3 = { 843, 60 };
    DDRD_pos pos4 = { 90, 90 };
    DDRD_pos pos5 = { 120, 354 };
    DDRD_pos pos6 = { 45, 150 };
    DDRD_pos pos7 = { 180, 180 };
    DDRD_pos pos8 = { 210, 235 };
    DDRD_pos pos9 = { 455, 240 };

    DDRD_resistor_new(circuit, 220, pos1); 
    DDRD_resistor_new(circuit, 230, pos2);
    DDRD_resistor_new(circuit, 100, pos3);
    DDRD_resistor_new(circuit, 470, pos4);
    DDRD_resistor_new(circuit, 134, pos5); 
    DDRD_resistor_new(circuit, 330, pos6);
    DDRD_resistor_new(circuit, 270, pos7);
    DDRD_resistor_new(circuit, 150, pos8);
    DDRD_resistor_new(circuit, 680, pos9);


    // *** Gestures ***
    GtkGesture *click_gesture;
    GtkGesture *drag_gesture;

    // press
    click_gesture = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (click_gesture), GDK_BUTTON_PRIMARY);
    g_signal_connect (click_gesture, "pressed", G_CALLBACK (workspace_press), NULL);
    g_signal_connect (click_gesture, "released", G_CALLBACK (workspace_release), NULL);
    gtk_widget_add_controller (workspace_area, GTK_EVENT_CONTROLLER (click_gesture));

    // drag
    drag_gesture = gtk_gesture_drag_new ();
    g_signal_connect (drag_gesture, "drag-update", G_CALLBACK (on_drag_update), NULL);
    gtk_widget_add_controller (workspace_area, GTK_EVENT_CONTROLLER (drag_gesture));

    // scroll
    GtkEventController *scroll_controller = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
    g_signal_connect(scroll_controller, "scroll", G_CALLBACK(on_scroll), NULL);
    gtk_widget_add_controller(workspace_area, scroll_controller);



    // *** Show all ***
    gtk_window_set_child(GTK_WINDOW(window), main_vertical_box);
    gtk_window_present(GTK_WINDOW(window));
}




















