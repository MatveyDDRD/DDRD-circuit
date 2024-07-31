[![wakatime](https://wakatime.com/badge/github/MatveyDDRD/DDRD-circuit.svg)](https://wakatime.com/badge/github/MatveyDDRD/DDRD-circuit?style=plastic)

# This is a program for creating, editing and running electronic circuits

## How to compile

```console
cd ~/
mkdir DDRD_circuit
cd DDRD_circuit
git clone https://github.com/MatveyDDRD/DDRD-circuit.git
./install.sh
``` 

### This is a hierarchical representation of the GTK application's UI components

    GtkApplicationWindow (window)
    └── GtkBox (main_vertical_box, vertical)
        ├── GtkPaned (workspace_paned, horizontal)
        │   ├── GtkFrame (sidebar_frame)
        │   │   └── GtkBox (sidebar_stack_container, horizontal)
        │   │       ├── GtkStackSwitcher (sidebar_stack_switcher, vertical orientation)
        │   │       └── GtkStack (sidebar_stack)
        │   │           ├── GtkBox (file_list_container, vertical) "Files"
        │   │           │   └── GtkScrolledWindow (file_list_scrolled)
        │   │           │       └── GtkListView (file_list_widget)
        │   │           └── GtkBox (elements_container, vertical) "Elements"
        │   └── GtkFrame (workspace_frame)


### This is a hierarchical representation of the... of the... hm... idk

    Main Window
    ├── Menu
    │   ├── File
    │   │   ├── New
    │   │   ├── Open
    │   │   ├── Save
    │   │   └── Exit
    │   ├── Edit
    │   │   ├── Undo
    │   │   ├── Redo
    │   │   ├── Cut
    │   │   ├── Copy
    │   │   └── Paste
    │   └── Help
    │       ├── Help Contents
    │       └── About
    ├── Toolbar
    │   ├── Selection Tool
    │   ├── Drawing Tool
    │   ├── Move Tool
    │   └── Eraser Tool
    ├── Workspace
    │   ├── Schematic Canvas
    │   │   ├── Components
    │   │   ├── Connections (wires between components)
    │   │   └── Selection Layer (for selected components)
    │   └── Grid
    ├── Properties Panel
    │   ├── Component Properties
    │   │   ├── Name
    │   │   ├── Type
    │   │   ├── Value
    │   │   └── Notes
    │   └── Schematic Properties
    │       ├── Schematic Name
    │       ├── Description
    │       ├── Author
    │       └── Creation Date
    └── Status Bar
        ├── Cursor Coordinates
        ├── Current Tool
        └── Messages and Hints
