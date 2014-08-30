#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int builtin_getch() {
	struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	if (ch == 27)
		if ((ch = getchar()) == '[')
			if (!(ch = getchar()))
				ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}
