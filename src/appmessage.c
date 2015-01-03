#include "common.h"


static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
static char *translate_error(AppMessageResult result);

void send_command(char * app, char * command) {
  Tuplet app_tup = TupletCString(KEY_APP, app);
  Tuplet command_tup = TupletCString(KEY_COMMAND, command);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_tuplet(iter, &app_tup);
  dict_write_tuplet(iter, &command_tup);
  dict_write_end(iter);

  app_message_outbox_send();
}

void appmessage_init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_sent(out_sent_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_open(app_message_inbox_size_maximum() /* inbound_size */, app_message_outbox_size_maximum() /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage - Message Received");
  Tuple *app = dict_find(iter, KEY_APP);

  if (app) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage - Message for %s", app->value->cstring);

    if (!strcmp("iTunes", app->value->cstring)) {
      itunes_update_ui(iter);
    } else if (!strcmp("Spotify", app->value->cstring)) {
      spotify_update_ui(iter);
    } else if (!strcmp("Keynote", app->value->cstring)) {
      keynote_update_ui(iter);
    } else if (!strcmp("PowerPoint", app->value->cstring)) {
      powerpoint_update_ui(iter);
    } else if (!strcmp("System", app->value->cstring)) {
      system_update_ui(iter);
    }
    return;
  }

  Tuple *connected = dict_find(iter, KEY_CONNECTED);
  if (connected) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "AppMessage - Connection status");
    if (connected->value->uint32) {
      wsConnected = true;
    } else {
      wsConnected = false;
    }
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped, %s", translate_error(reason));
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {

}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {

}


static char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}
