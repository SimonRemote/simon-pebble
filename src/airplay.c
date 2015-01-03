/*
 * A lot of this code was written by https://github.com/inonprince
 * Permission was granted to use it in SimonRemote.
 */

#include "airplay.h"

#include "common.h"
#include "util.h"

#define MAX_AP_DEVICES 30
#define TOKEN '|'

static void send_request(char * command);
static void window_load(Window *window);
static void window_unload(Window *window);
static bool is_device_active(char* device_name);

static Window *window;
static MenuLayer *menu_layer;
static GBitmap *action_icon_checkmark;

static char *airplay_devices[MAX_AP_DEVICES];
static char *active_airplay_devices[MAX_AP_DEVICES];
static uint8_t number_of_devices;
static uint8_t number_of_active_devices;

static bool has_loaded = false;

static bool is_device_active(char *device_name) {
  for(uint8_t i = 0; i < number_of_active_devices; i++) {
    if (!strcmp(device_name,active_airplay_devices[i])) {
      return true;
    }
  }
  return false;
}

static uint8_t populate_airplay_list(char *list[], char *string) {
  int i;
  char *tok;

  for ((i = 0, tok = strtok1(string, TOKEN));
       (tok != NULL) && (i < MAX_AP_DEVICES);
       (i++, tok = strtok1(NULL, TOKEN))) {

    list[i] = malloc(strlen(tok) + 1);
    strcpy(list[i], tok);
  }

  return i;
}

static void destroy_airplay_list(char *list[]) {
  for (uint8_t i = 0; i < MAX_AP_DEVICES; i++) {
    if (list[i] != NULL) {
      free(list[i]);
    }
  }

  memset(list, 0, (MAX_AP_DEVICES * sizeof(char *))); // reset array
}

void destroy_airplay_data(void) {
  destroy_airplay_list(airplay_devices);
  destroy_airplay_list(active_airplay_devices);
  number_of_devices = 0;
  number_of_active_devices = 0;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return 35;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return 1;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      return number_of_devices;
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
      menu_cell_basic_header_draw(ctx, cell_layer, "AirPlay Devices");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GBitmap *icon = is_device_active(airplay_devices[cell_index->row]) ? action_icon_checkmark : NULL;
  menu_cell_basic_draw(ctx, cell_layer, airplay_devices[cell_index->row], NULL, icon);
}

// Here we capture when a user selects a menu item
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "selected row %d", cell_index->row);
      char request[80];
      strcpy (request, "airplay_set-");
      strcat (request, airplay_devices[cell_index->row]);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "sending: %s", request);
      send_request(request);
      break;
  }
}

void airplay_init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - AirPlay init %p", window);

  action_icon_checkmark = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_CHECKMARK);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(window, true);

  send_request("info");
}

void airplay_connected(bool connected) {
  wsConnected = connected;
}

void airplay_new_tuple(Tuple *tuple) {
  switch(tuple->key) {
    case KEY_APDEVICES:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "AP Devices: %s", tuple->value->cstring);
      destroy_airplay_list(airplay_devices);
      number_of_devices = populate_airplay_list(airplay_devices, tuple->value->cstring);
      break;
    case KEY_APACTIVE:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Active AP Devices: %s",tuple->value->cstring);
      destroy_airplay_list(active_airplay_devices);
      number_of_active_devices = populate_airplay_list(active_airplay_devices, tuple->value->cstring);
      break;
  }

  if (has_loaded) {
    menu_layer_reload_data(menu_layer);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void send_request(char * command) {
  send_command("iTunes", command);
}

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
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - AirPlay window_unload %p", window);
  has_loaded = false;

  gbitmap_destroy(action_icon_checkmark);
  menu_layer_destroy(menu_layer);
  window_destroy(window);
}

void airplay_control() {
  airplay_init();
}
