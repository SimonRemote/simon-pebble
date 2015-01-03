#include "common.h"
#include "airplay.h"

#define NUM_MENU_SECTIONS 2
#define NUM_MENU_ROWS (KEYBOARD_MENU_ITEMS + VOLUME_MENU_ITEMS)

#define KEYBOARD_MENU_ITEMS 5
#define VOLUME_MENU_ITEMS 2

static void send_request(char * command);
static void window_load(Window *window);
static void window_unload(Window *window);


static Window *window;
static MenuLayer *menu_layer;

static bool has_loaded = false;

static Window *window;
static MenuLayer * menu_layer;

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return 35;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      return KEYBOARD_MENU_ITEMS;
      break;

    case 1:
      return VOLUME_MENU_ITEMS;
      break;
  }
  return 0;
}

// A callback is used to specify the height of the section header
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

// Here we draw what each header is
static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Keyboard");
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Volume");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Switch on section
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Space", NULL, NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Up", NULL, NULL);
          break;
        case 2:
          menu_cell_basic_draw(ctx, cell_layer, "Down", NULL, NULL);
          break;
        case 3:
          menu_cell_basic_draw(ctx, cell_layer, "Right", NULL, NULL);
          break;
        case 4:
          menu_cell_basic_draw(ctx, cell_layer, "Left", NULL, NULL);
          break;
      }
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Volume Up", NULL, NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Volume Down", NULL, NULL);
          break;
      }
      break;
  }
}

// Here we capture when a user selects a menu item
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {

    case 0:
      switch (cell_index->row) {
        case 0:
          send_request("space");
          break;
        case 1:
          send_request("up");
          break;
        case 2:
          send_request("down");
          break;
        case 3:
          send_request("right");
          break;
        case 4:
          send_request("left");
          break;
      }
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          send_request("volume_up");
          break;
        // Keynote
        case 1:
          send_request("volume_down");
          break;
      }
      break;
  }
}

void system_init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - System init %p", window);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  send_request("info");

  window_stack_push(window, true);
}

void system_deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - System deinit %p", window);
  // save some stuff
  window_destroy(window);
}

void system_connected(bool connected) {
  wsConnected = connected;
}


void system_update_ui(DictionaryIterator *iter) {
  return;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void send_request(char * command) {
  send_command("System", command);
}

// Events

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the menu layer
  menu_layer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menu_layer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));

  has_loaded = true;
}

static void window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - System window_unload %p", window);
  has_loaded = false;

  menu_layer_destroy(menu_layer);
  window_destroy(window);
}

void system_control() {
  system_init();
}
