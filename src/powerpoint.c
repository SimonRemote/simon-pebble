#include "common.h"

static void send_request(char * command);
static void update_playing_status();
static void update_main_text();
static void update_progress_bar();
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void up_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_single_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context);
static void click_config_provider(void *context);
static void window_load(Window *window);
static void window_unload(Window *window);


static Window *window;

static TextLayer *header_text;
static TextLayer *main_text;
static TextLayer *footer_text;

static ProgressBarLayer *progress_bar;

static BitmapLayer* progress_bar_left_icon_layer;

static ActionBarLayer *action_bar;

static GBitmap *action_icon_left_arrow;
static GBitmap *action_icon_right_arrow;

static GBitmap *status_bar_icon;
static GBitmap *action_icon_left_arrow;
static GBitmap *action_icon_right_arrow;
static GBitmap *action_icon_play;
static GBitmap *action_icon_pause;
static GBitmap *action_icon_volume_up;
static GBitmap *action_icon_volume_down;
static GBitmap *action_icon_refresh;
static GBitmap *progress_bar_clock;
static GBitmap *progress_bar_volume_icon;

static uint8_t sys_volume;
static uint16_t position;
static uint16_t duration;
static char * slide_progress_text;
static char * time_text;

static uint16_t seconds;

static bool controlling_volume;
static bool is_playing;
static bool refresh_icon_showing;

static bool has_loaded = false;

void powerpoint_init(void) {
  slide_progress_text = malloc(20);
  time_text = malloc(10);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - PowerPoint init %p", window);

  position = 1;
  duration = 1;

  controlling_volume = false;
  is_playing = false;
  refresh_icon_showing = false;

  status_bar_icon = gbitmap_create_with_resource(RESOURCE_ID_STATUS_BAR_ICON_POWERPOINT);
  action_icon_left_arrow = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_LEFT_ARROW);
  action_icon_right_arrow = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_RIGHT_ARROW);
  action_icon_play = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_PLAY);
  action_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_PAUSE);
  action_icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_VOLUME_UP);
  action_icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_VOLUME_DOWN);
  action_icon_refresh = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_REFRESH);
  progress_bar_clock = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PROGRESS_BAR_CLOCK);
  progress_bar_volume_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PROGRESS_BAR_VOLUME_ICON);

  window = window_create();
  window_set_status_bar_icon(window, status_bar_icon);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  send_request("info");

  window_stack_push(window, true);
}

void powerpoint_deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - PowerPoint deinit %p", window);
  free(slide_progress_text);
  free(time_text);
  // save some stuff
  window_destroy(window);
}


void powerpoint_update_ui(DictionaryIterator *iter) {
  if (!has_loaded) {
    return;
  }

  Tuple *tuple = dict_read_first(iter);

  while(tuple) {
    switch(tuple->key) {
      case KEY_CONNECTED:

        break;
      case KEY_CONNECTEDTO:

        break;
      case KEY_HEADERTEXT:
        // text_layer_set_text(header_text, tuple->value->cstring);
        break;
      case KEY_MAINTEXT:
        text_layer_set_text(main_text, tuple->value->cstring);
        break;
      case KEY_FOOTERTEXT:
        // text_layer_set_text(footer_text, tuple->value->cstring);
        break;
      case KEY_POSITION:
        position = (uint16_t)tuple->value->uint32;
        break;
      case KEY_DURATION:
        duration = (uint16_t)tuple->value->uint32;
        break;
      case KEY_SHUFFLE:
        break;
      case KEY_PLAYING:
        // is_playing = (tuple->value->uint32) ? true : false;
        // update_playing_status();
        break;
      case KEY_APPVOLUME:
        // app_volume = (uint8_t)tuple->value->uint32;
        break;
      case KEY_SYSVOLUME:
        sys_volume = (uint8_t)tuple->value->uint32;
        break;
      case KEY_APP:
        //we know it's PowerPoint
        break;
      default:
        break;
    }
    tuple = dict_read_next(iter);
  }

  update_main_text();
  update_progress_bar();
}

static void update_main_text() {

  snprintf(slide_progress_text, 20, "%d of %d", position, duration);
  text_layer_set_text(main_text, slide_progress_text);
}

static void decrementPosition() {
  if (position > 1) {
    position--;
  }
}

static void incrementPosition() {
  if (position < duration) {
    position++;
  }
}

static void update_playing_status() {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, is_playing ? action_icon_pause : action_icon_play);
}

static void update_progress_bar() {
  progress_bar_layer_set_value(progress_bar, (controlling_volume) ? sys_volume : (position*100)/duration);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "position: %d, duration: %d, progress: %d\n", position, duration, (position*100)/duration);
}

static void update_timer() {
  uint16_t min = seconds / 60;
  uint16_t sec = seconds % 60;

  snprintf(time_text, 10, "%02d:%02d", min, sec);

  // Now draw the strings.
  text_layer_set_text(footer_text, time_text);
}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)  {
  if (is_playing) {
    seconds += 1;
    update_timer();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void send_request(char * command) {
  send_command("PowerPoint", command);
}

// Events
static void back_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  powerpoint_deinit();
  window_stack_pop(true);
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  // send_request(is_playing ? "pause" : "play");
  is_playing = !is_playing;
  update_playing_status();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (controlling_volume) {
    send_request("volume_up");
  } else {
    send_request("previous");
    decrementPosition();
    update_main_text();
    update_progress_bar();
  }
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (controlling_volume) {
    send_request("volume_down");
  } else {
    send_request("next");
    incrementPosition();
    update_main_text();
    update_progress_bar();
  }
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  controlling_volume = !controlling_volume;
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, controlling_volume ? action_icon_volume_up : action_icon_left_arrow);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, controlling_volume ? action_icon_volume_down : action_icon_right_arrow);
  bitmap_layer_set_bitmap(progress_bar_left_icon_layer, controlling_volume ? progress_bar_volume_icon : progress_bar_clock);
  update_progress_bar();
}

static void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_refresh);
  refresh_icon_showing = true;
}

static void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
  if (refresh_icon_showing) {
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, controlling_volume ? action_icon_volume_down : action_icon_right_arrow);

    send_request("info");
    refresh_icon_showing = false;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, LONG_CLICK_HOLD_MS, select_long_click_handler, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, LONG_CLICK_HOLD_MS, down_long_click_handler, down_long_click_release_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);

  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_icon_left_arrow);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_icon_right_arrow);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_icon_play);

  main_text = text_layer_create((GRect) { .origin = { 4, 44 }, .size = { bounds.size.w-30, 60 } });
  text_layer_set_text(main_text, "No Slide Show Found");
  text_layer_set_text_alignment(main_text, GTextAlignmentLeft);
  text_layer_set_font(main_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_overflow_mode(main_text, GTextOverflowModeFill);
  layer_add_child(window_layer, text_layer_get_layer(main_text));

  header_text = text_layer_create((GRect) { .origin = { 4, 0 }, .size = { bounds.size.w-30, 54 } });
  text_layer_set_text_alignment(header_text, GTextAlignmentLeft);
  text_layer_set_font(header_text, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_overflow_mode(header_text, GTextOverflowModeFill);
  layer_add_child(window_layer, text_layer_get_layer(header_text));

  footer_text = text_layer_create((GRect) { .origin = { 4, 128 }, .size = { bounds.size.w-30, 30 } });
  text_layer_set_text(footer_text, "00:00");
  text_layer_set_text_alignment(footer_text, GTextAlignmentLeft);
  text_layer_set_font(footer_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(footer_text));

  progress_bar = progress_bar_layer_create((GRect) { .origin = { 19, 115 }, .size = { 85, 7 } });
  progress_bar_layer_set_orientation(progress_bar, ProgressBarOrientationHorizontal);
  progress_bar_layer_set_range(progress_bar, 0, 100);
  progress_bar_layer_set_frame_color(progress_bar, GColorBlack);
  progress_bar_layer_set_bar_color(progress_bar, GColorBlack);
  layer_add_child(window_layer, progress_bar);

  progress_bar_left_icon_layer = bitmap_layer_create(GRect(4,113,13,11));
  bitmap_layer_set_bitmap(progress_bar_left_icon_layer, progress_bar_clock);
  layer_add_child(window_layer, bitmap_layer_get_layer(progress_bar_left_icon_layer));

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

  has_loaded = true;
}

static void window_unload(Window *window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Window - PowerPoint window_unload %p", window);
  has_loaded = false;

  tick_timer_service_unsubscribe();

  gbitmap_destroy(status_bar_icon);
  gbitmap_destroy(action_icon_left_arrow);
  gbitmap_destroy(action_icon_right_arrow);
  gbitmap_destroy(action_icon_play);
  gbitmap_destroy(action_icon_pause);
  gbitmap_destroy(action_icon_volume_up);
  gbitmap_destroy(action_icon_volume_down);
  gbitmap_destroy(action_icon_refresh);
  gbitmap_destroy(progress_bar_clock);
  gbitmap_destroy(progress_bar_volume_icon);

  text_layer_destroy(header_text);
  text_layer_destroy(main_text);
  text_layer_destroy(footer_text);

  bitmap_layer_destroy(progress_bar_left_icon_layer);
  progress_bar_layer_destroy(progress_bar);

  action_bar_layer_destroy(action_bar);
}

void powerpoint_control() {
  powerpoint_init();
}
