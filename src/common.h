#pragma once

#include <pebble.h>
#include "progress_bar.h"

#define LONG_CLICK_HOLD_MS 600

enum { 
    KEY_CONNECTED   = 0,
    KEY_CONNECTEDTO = 1,
    KEY_HEADERTEXT  = 2,
    KEY_MAINTEXT    = 3,
    KEY_FOOTERTEXT  = 4,
    KEY_POSITION    = 5,
    KEY_DURATION    = 6,
    KEY_SHUFFLE     = 7,
    KEY_PLAYING     = 8,
    KEY_APPVOLUME   = 9,
    KEY_SYSVOLUME   = 10,
    KEY_APP         = 11,
    KEY_COMMAND     = 12,
    KEY_APACTIVE    = 13,
    KEY_APDEVICES   = 14,
};

extern bool wsConnected;


void send_command(char * app, char * cmd);

/* appmessage */
void appmessage_init(void);

/* General */
typedef void (*Callback)(char *name);

/* PowerPoint */
void powerpoint_control();
void powerpoint_update_ui(DictionaryIterator *iter);

/* iTunes */
void itunes_control();
void itunes_update_ui(DictionaryIterator *iter);
void itunes_connected(bool connected);

/* Spotify */
void spotify_control();
void spotify_update_ui(DictionaryIterator *iter);

/* Keynote */
void keynote_control();
void keynote_update_ui(DictionaryIterator *iter);

/* System */
void system_control();
void system_update_ui(DictionaryIterator *iter);

