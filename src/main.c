#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <gtk/gtk.h>

#include "interface.h"
#include "electronics.h"
#include "drawing.h"

/*

 ********************************************************************
*                                                                    *
*                          DDRD-circuit                              *
*                                                                    *
 ********************************************************************

*/

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

