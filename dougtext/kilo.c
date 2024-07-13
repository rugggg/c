/** includes **/
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/** defines **/
#define CTRL_KEY(k) ((k) & 0x1f) // macro
                                 // this works bc ctrl key actually 
                                 // strips bits 5 and 6 from whatever key
                                 // you press in combination with Ctrl,
                                 // and sends that
                                 // and, bit 5 fwiw, is typically lower/upper case bit
                                 //
/** prototype def, prob move to headers later **/
void clearScreen(void);
/** data **/
struct termios orig_termios;

/** terminal **/
void die(const char *s) {
  // first clear the screen
  clearScreen();
  // error and exit
  perror(s);
  exit(1);
}

void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode(void) {
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

void clearScreen(void) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
}

char editorReadKey(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1){
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

/** output **/

void editorDrawRows(void) {
  int y;
  for (y=0; y < 24; y++){
    write(STDOUT_FILENO, "|\r\n", 3);
  }
}

void editorRefreshScreen(void) {
  //The 4 in our write() call means we are writing 4 bytes out to the terminal.
  //The first byte is \x1b, which is the escape character, or 27 in decimal.
  //(Try and remember \x1b, we will be using it a lot.)
  //The other three bytes are [2J.
  //so this writes the escape sequence to teminal
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3); // esc seq \x1b[ and H (cursor position) 
                                     // command, 3 bytes.
                                     // H takes two args actually, row, col. defaults
                                     // to 1,1 so we pass no args
  editorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);

}

/** input **/
void editorProcessKeypress(void){
  char c = editorReadKey();
  switch (c){
    case CTRL_KEY('q'):
      clearScreen();
      exit(0);
      break;
  }
}

/** main ***/
int main(void) {
  enableRawMode();
  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  };

  return 0;
}
