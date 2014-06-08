#include "common.h"

bool wsConnected;

#define NUM_MENU_SECTIONS 2
#define NUM_MENU_ROWS 4

#define NUM_MUSIC_MENU_ITEMS 2
#define NUM_PRESENTATION_MENU_ITEMS 2

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
      return NUM_MUSIC_MENU_ITEMS;
      break;

    case 1:
      return NUM_PRESENTATION_MENU_ITEMS;
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
      menu_cell_basic_header_draw(ctx, cell_layer, "Music");
      break;

    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Presentation");
      break;
  }
}

// This is the menu item draw callback where you specify what each item should look like
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Switch on section
  switch (cell_index->section) {

    // Music Player
    case 0:
      switch (cell_index->row) {
        // iTunes
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "iTunes", NULL, NULL);
          break;
        // Spotify
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Spotify", NULL, NULL);
          break;
      }
      break;
    case 1:

      // Presentation
      switch (cell_index->row) {
        // PowerPoint
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "PowerPoint", NULL, NULL);
          break;
        // Keynote
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Keynote", NULL, NULL);
          break;
      }
      break;
  }
}

// Here we capture when a user selects a menu item
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {

    // Music Player
    case 0:
      switch (cell_index->row) {
        // iTunes
        case 0:
          itunes_control(NULL);
          break;
        // Spotify
        case 1:
          spotify_control(NULL);
          break;
      }
      break;
    case 1:

      // Presentation
      switch (cell_index->row) {
        // PowerPoint
        case 0:
          powerpoint_control(NULL);
          break;
        // Keynote
        case 1:
          keynote_control(NULL);
          break;
      }
      break;
  }
}

// Main window
static void window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  // We need the bounds to specify the menu layer's viewport size
  // In this case, it'll be the same as the window's
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

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
}

static void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  wsConnected = false;
  appmessage_init();
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}