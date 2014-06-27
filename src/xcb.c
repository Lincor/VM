#include <config.h>

#ifdef GUI
#if GUI_LIB == XCB

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
	while (e = xcb_wait_for_event(connection)) {
		switch (e->response_type & ~0x80) {
			case XCB_KEY_RELEASE:
			return ((xcb_key_release_event_t*)e)->detail;
		}
		free(e);
	}
}

void xcb_printf(char *f,...) {
	va_list args;
	va_start(args,f);
	char *s = malloc(strlen(f)+sizeof(args));
	vsprintf(s,f,args);
	int i;
	for (i=0;i<strlen(s);i++) xcb_putchar(s[i]);
	//puts(s);
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
	values[1] = XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE;
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
#endif
