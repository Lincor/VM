#define ui_init		xcb_init
#define ui_printf	xcb_printf
#define ui_putchar	xcb_putchar
#define ui_update	xcb_update
#define ui_getch	xcb_getch

void xcb_printf(char *,...);
void xcb_update();
void xcb_putchar(char);
int xcb_getch();
void xcb_init();
