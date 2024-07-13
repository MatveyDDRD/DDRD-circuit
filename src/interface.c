#include <gtk/gtk.h>
#include <time.h>
#include "DDRD_types.h"
#include "drawing.h"
#include "electronics.h"

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


int DDRD_print(const char *format, ANSI_COLOR color, int level, bool bold, ...) {
    va_list args;
    va_start(args, bold);

    // Установить цвет и стиль текста
    const char *color_code = get_color_code(color);
    const char *bold_code = bold ? "\033[1m" : "\033[22m"; // \033[22m сбрасывает жирный шрифт

    if (color_code == NULL || bold_code == NULL) {
        // Если color_code или bold_code равен NULL, завершаем функцию
        va_end(args);
        return -1;
    }

    // Вывести табуляции в зависимости от уровня
    for (int i = 0; i < level; i++) {
        printf("\t");
    }

    // Установить цвет и стиль текста
    printf("%s%s", bold_code, color_code);

    // Вывести основной текст
    int result = vprintf(format, args);

    // Сбросить форматирование
    printf("\033[0m\n");

    va_end(args);
    return result;
}

//***** END PRINT *****//

// *** Functions definitions ***

void new_file_action_callback(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
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
static void setup_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(listitem, label);
    // No need to unref label due to sunk reference
}

static void bind_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
    GtkWidget *label = gtk_list_item_get_child(listitem);
    GtkStringObject *str_obj = gtk_list_item_get_item(listitem);
    gtk_label_set_text(GTK_LABEL(label), gtk_string_object_get_string(str_obj));
}

static void unbind_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
    // Nothing to do here 
}

static void teardown_list_item_cb(GtkSignalListItemFactory *self, GtkListItem *listitem, gpointer user_data) {
    // Nothing to do here, GtkListItem will be destroyed
}

// Function to create the file list view
void make_file_tree() {
    list_directory();

    char **names_array = (char **)malloc(sizeof(char *) * (file_count_in_current_directory + 1)); // +1 для NULL 

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

void activate(GtkApplication *app, gpointer user_data) {
    printf("Activating application...\n");

    // *** Main window ***
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);

    GtkWidget *main_vertical_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

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
    gtk_orientable_set_orientation(sidebar_stack_switcher, GTK_ORIENTATION_VERTICAL);

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
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(file_list_scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
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
    gpointer drawing_data = malloc(sizeof(draw_data));

    GtkWidget *workspace_area = gtk_drawing_area_new();
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(workspace_area), 500);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(workspace_area), 500);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(workspace_area), draw_function, drawing_data, NULL);

    gtk_widget_set_vexpand(workspace_area, TRUE);
    gtk_widget_set_hexpand(workspace_area, TRUE);
    gtk_frame_set_child(GTK_FRAME(workspace_frame), workspace_area);

    // Show all
    gtk_window_set_child(GTK_WINDOW(window), main_vertical_box);
    gtk_window_present(GTK_WINDOW(window));

    // Применение стилей
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "styles.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                               GTK_STYLE_PROVIDER(css_provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_USER);

    DDRD_circuit *circuit = DDRD_circuit_new(workspace_area);
    DDRD_pos pos = {200, 200};
    DDRD_resistor_new(&circuit, 220.0, pos);
}




















