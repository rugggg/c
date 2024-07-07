/** includes **/
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/** data **/
struct termios orig_termios;

/** terminal **/
void die(const char *s) {
  perror(s);
  exit(1);
}


void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  // turning off a bunch of flags
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // turns off ctrl+s and ctrl+q, ICRNL handls carriage return
  raw.c_iflag &= ~(ICRNL | IXON);
  // handles the echo, and disables ctrl+c ctrl+z, and ctrl+v 
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // handle 
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);

  // timings
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/** main ***/
int main() {
  enableRawMode();
  while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  };

  return 0;
}
