#include "x11.h"
#include "safe.h"
#include "util.h"
#include "terminal.h"
#include "constants.h"
#include "components.h"

void ExitProgram(struct PTY* pty, struct X11* x11) {
	free(x11->escape);
	free(x11->buf);
	close(pty->master);
	close(pty->slave);
}

int Run(struct PTY* pty, struct X11* x11) {
	int    i, maxfd;
	fd_set readable;
	XEvent ev;
	char   buf[1];
	bool   just_wrapped = false;

	maxfd = pty->master > x11->fd ? pty->master : x11->fd;

	while (true) {
		FD_ZERO(&readable);
		FD_SET(pty->master, &readable);
		FD_SET(x11->fd, &readable);

		if (select(maxfd + 1, &readable, NULL, NULL, NULL) == -1) {
			perror("select");
			return 1;
		}

		if (FD_ISSET(pty->master, &readable)) {
			if (read(pty->master, buf, 1) <= 0) {
				/* This is not necessarily an error but also happens
				 * when the child exits normally. */
				fprintf(stderr, "Nothing to read from child: ");
				perror(NULL);
				return 1;
			}

			if (buf[0] == '\r') {
				/* "Carriage returns" are probably the most simple
				 * "terminal command": They just make the cursor jump
				 * back to the very first column. */
				x11->buf_x = 0;
			}
			else {
				if (x11->readingEscape) {
					x11->escape = SafeRealloc(
						x11->escape, strlen(x11->escape) + 2
					);
					strncat(x11->escape, buf, 1);
					continue;
				}
				
				if (buf[0] != '\n') {
					/* If this is a regular byte, store it and advance
					 * the cursor one cell "to the right". This might
					 * actually wrap to the next line, see below. */
					x11->buf[x11->buf_y * x11->buf_w + x11->buf_x] = buf[0];
					x11->buf_x++;

					if (x11->buf_x >= x11->buf_w) {
						x11->buf_x = 0;
						++ x11->buf_y;
						just_wrapped = true;
					}
					else {
						just_wrapped = false;
					}
				}
				else if (!just_wrapped) {
					/* We read a newline and we did *not* implicitly
					 * wrap to the next line with the last byte we read.
					 * This means we must *now* advance to the next
					 * line.
					 *
					 * This is the same behaviour that most other
					 * terminals have: If you print a full line and then
					 * a newline, they "ignore" that newline. (Just
					 * think about it: A full line of text could always
					 * wrap to the next line implicitly, so that
					 * additional newline could cause the cursor to jump
					 * to the next line *again*.) */
					++ x11->buf_y;
					just_wrapped = false;
				}

				/* We now check if "the next line" is actually outside
				 * of the buffer. If it is, we shift the entire content
				 * one line up and then stay in the very last line.
				 *
				 * After the memmove(), the last line still has the old
				 * content. We must clear it. */
				if (x11->buf_y >= x11->buf_h) {
					memmove(
						x11->buf, &x11->buf[x11->buf_w],
						x11->buf_w * (x11->buf_h - 1)
					);
					x11->buf_y = x11->buf_h - 1;

					for (i = 0; i < x11->buf_w; i++) {
						x11->buf[x11->buf_y * x11->buf_w + i] = 0;
					}
				}
			}

			X11_Redraw(x11);
		}

		if (FD_ISSET(x11->fd, &readable)) {
			while (XPending(x11->dpy)) {
				XNextEvent(x11->dpy, &ev);
				switch (ev.type) {
					case Expose: {
						X11_Redraw(x11);
						break;
					}
					case KeyPress: {
						X11_Key(&ev.xkey, pty);
						break;
					}
				}
			}
		}
	}

	return 0;
}

int main() {
	struct PTY pty;
	struct X11 x11;

	if (!X11_Setup(&x11))
		return 1;

	if (!PtPair(&pty))
		return 1;

	if (!SetTerminalSize(&pty, &x11))
		return 1;

	if (!Spawn(&pty))
		return 1;

	return Run(&pty, &x11);
}
