#ifndef TERM_HPP_INCLUDED
#define TERM_HPP_INCLUDED

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

class TerminalRawMode {
	public:
		TerminalRawMode() {
			tcgetattr(0, &vt_orig); /* get the current settings */
			struct termios trm = vt_orig;
			trm.c_cc[VMIN] = 1; 	// Minimum number of characters for noncanonical read (MIN).
			trm.c_cc[VTIME] = 0; 	// Timeout in deciseconds for noncanonical read (TIME).
			// echo off, canonical mode off, extended input processing off, signal chars off:
			trm.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
			tcsetattr(STDIN_FILENO, TCSANOW, &trm);
		}

		~TerminalRawMode() {
			tcsetattr(0, TCSANOW, &vt_orig);
			if (this->cursor_app_set) printf("\033[?1l");
			if (this->keypad_app_set) printf("\033[?66l"); // CSI ? 1 h, Application Cursor Keys
                        if (this->bracketed_paste_mode) printf("\033[?2004l");
                        fflush(stdout);
		};

		void setCursorApp() {
			this->cursor_app_set = true;
			printf("\033[?1h"); // CSI ? 1 h, Application Cursor Keys
			fflush(stdout);
		}

		void setKeypadApp() {
			this->keypad_app_set = true;
			//printf("\033[?66h"); // CSI ? 1 h, Application Cursor Keys
			printf("SETTING KEYPAD\n");
			printf("\033="); // CSI ? 1 h, Application Cursor Keys
			fflush(stdout);
		}

		void placeCursor(unsigned int row, unsigned int col) {
			printf("\033[%u;%uH", row, col);
			fflush(stdout);
		}

                void setBracketedPasteMode(bool set) {
                        if (set == this->bracketed_paste_mode) return;
                        printf("\033[?2004%s", set ? "h" : "l");
                        fflush(stdout);
                        this->bracketed_paste_mode = set;
                }

                void useScreenBuffer(bool normal) {
                        decPrivateMode(1049, normal);
                }

        private:
                bool const is_tty{!!isatty(0)};
		struct termios vt_orig;
		bool cursor_app_set{false};
		bool keypad_app_set{false};
                bool bracketed_paste_mode{false};

		void decPrivateMode(unsigned int mode, bool set) {
			printf("\033[?%u%c", mode, (set ? 'h' : 'l'));
			fflush(stdout);
		}
};

#endif
