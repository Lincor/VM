#include <config.h>

#ifdef ENABLE_XCB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <stdarg.h>

#define WIDTH 80
#define HEIGHT 25

static int x,y;
static char buf[HEIGHT][WIDTH];
static xcb_connection_t *connection;
static xcb_screen_t *screen;
static xcb_window_t window;
static xcb_intern_atom_cookie_t cookie;
static xcb_intern_atom_reply_t* reply;
static xcb_intern_atom_cookie_t cookie2;
static xcb_intern_atom_reply_t* reply2;

static uint8_t scancodes[256] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
	0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, (0x80 | 0x2a), 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, (0x80 | 0x36), 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0, 0, 0, 0x57,
	0x58, (0x80 | 0x47), (0x80 | 0x48), (0x80 | 0x49), (0x80 | 0x4b), 0, (0x80 | 0x4d), (0x80 | 0x4f),
	(0x80 | 0x1c), (0x80 | 0x1d), (0x80 | 0x35), (0x80 | 0x53), (0x80 | 0x38), (0x80 | 0x1d), (0x80 | 0x47), (0x80 | 0x48),
	(0x80 | 0x49), (0x80 | 0x4b), (0x80 | 0x4d), (0x80 | 0x4f), (0x80 | 0x50), (0x80 | 0x51), (0x80 | 0x52), (0x80 | 0x53),
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, (0x80 | 0x5b), 0, (0x80 | 0x5d),
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static void test_cookie(xcb_void_cookie_t cookie, xcb_connection_t *connection, char *errMessage) {
	xcb_generic_error_t *error = xcb_request_check(connection, cookie);
	if(error) {
		xcb_disconnect(connection);
		exit(-1);
	}
}

static xcb_gc_t getfontgc(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window, const char *font_name) {
	xcb_font_t font = xcb_generate_id(connection);
	xcb_open_font(connection, font, strlen(font_name), font_name);
	xcb_gcontext_t  gc = xcb_generate_id(connection);
	uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
	uint32_t value_list[3] = { screen->white_pixel, screen->black_pixel, font };
	xcb_create_gc(connection, gc, window, mask, value_list);
	xcb_close_font(connection, font);
	return gc;
}

static void xcb_draw_text(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window, int16_t x1, int16_t y1, const char *label) {
	xcb_gcontext_t gc = getfontgc(connection, screen, window, "fixed");
	xcb_image_text_8(connection, strlen(label), window, gc, x1, y1, label);
	xcb_void_cookie_t cookie = xcb_free_gc(connection, gc);
	test_cookie(cookie, connection, "can't free gc");
}

static void xcb_repaint() {
	int i;
	for (i=0;i<HEIGHT;i++) {
		xcb_draw_text(connection,screen,window,0,(i+1)*12,buf[i]);
	}
}

void xcb_update() {
	xcb_generic_event_t *e = xcb_poll_for_event(connection);
	if (e) {
		switch (e->response_type & ~0x80) {
			case XCB_CLIENT_MESSAGE:
			if ((*(xcb_client_message_event_t*)e).data.data32[0] == (*reply2).atom)
				exit(0);
			break;
			case XCB_EXPOSE:
			xcb_repaint();
			break;
		}
		free(e);
	}
}

void xcb_putchar(char c) {
	if (c == '\n') {
		y++;
		x=0;
	} else if (c == '\r') x=0;
	else {
		buf[y][x]=c;
		x++;
		if (x>WIDTH) {
			x=0;
			y++;
		}
		xcb_repaint();
	}
}

int xcb_getch() {
	xcb_generic_event_t *e;
	while ((e = xcb_wait_for_event(connection))) {
		switch (e->response_type & ~0x80) {
			case XCB_KEY_PRESS:
				return scancodes[((xcb_key_press_event_t*)e)->detail];
			case XCB_KEY_RELEASE:
				return scancodes[((xcb_key_release_event_t*)e)->detail];
		}
		free(e);
	}
	return 0;
}

void xcb_printf(char *f,...) {
	va_list args;
	va_start(args,f);
	char *s = malloc(strlen(f)+sizeof(args));
	vsprintf(s,f,args);
	int i;
	for (i=0;i<strlen(s);i++) xcb_putchar(s[i]);
	va_end(args);
}

void xcb_init() {
	connection = xcb_connect(NULL, NULL);
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
	screen = iter.data;
	window = xcb_generate_id(connection);
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[2];
	values[0] = screen->black_pixel;
	values[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE;
	xcb_create_window(connection,
	                  screen->root_depth,
	                  window, screen->root,
	                  20, 200,
	                  640, 480,
	                  0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
	                  screen->root_visual,
	                  mask, values);
	xcb_map_window(connection, window);
	xcb_flush(connection);
	cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
	reply = xcb_intern_atom_reply(connection, cookie, 0);
	cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
	reply2 = xcb_intern_atom_reply(connection, cookie2, 0);
	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1, &(*reply2).atom);
}

#endif
