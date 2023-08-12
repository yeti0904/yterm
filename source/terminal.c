#include "safe.h"
#include "util.h"
#include "escapes.h"
#include "terminal.h"
#include "constants.h"
#include "components.h"

void Terminal_Init(Terminal* terminal, char** env, Vec2 size) {
	terminal->buffer = TextScreen_New(size.x, size.y);
	terminal->title  = DupString(APP_NAME);

	PtPair(&terminal->pty);
	SetTerminalSize(terminal);
	Spawn(&terminal->pty, env);
}

void Terminal_Free(Terminal* terminal) {
	free(terminal->title);
	TextScreen_Free(&terminal->buffer);
}

void Terminal_Update(Terminal* terminal) {
	// check for stuff in stdout
	fd_set        readable;
	unsigned char in;
	
	FD_ZERO(&readable);
	FD_SET(terminal->pty.master, &readable);

	struct timeval selectTimeout;
	selectTimeout.tv_sec  = 0;
	selectTimeout.tv_usec = 1;

	if (
		select(
			terminal->pty.master + 1, &readable, NULL, NULL, &selectTimeout
		) == -1
	) {
		perror("select");
		exit(1);
	}

	size_t bytesRead = 0;

	while (FD_ISSET(terminal->pty.master, &readable)) {
		if (read(terminal->pty.master, &in, 1) <= 0) {
			/* This is not necessarily an error but also happens
			 * when the child exits normally. */
			fprintf(stderr, "Nothing to read from child: ");
			perror(NULL);
			exit(1);
		}

		++ bytesRead;

		switch (in) {
			case '\x1b': {
				if (terminal->config->interpretEscapes) {
					HandleEscape(terminal);
					break;
				}
			} // fall through
			default: {
				TextScreen_PutCharacter(&terminal->buffer, in);
			}
		}

		if (bytesRead > 1024) {
			break;
		}

		if (
			select(
				terminal->pty.master + 1, &readable, NULL, NULL, &selectTimeout
			) == -1
		) {
			perror("select");
			exit(1);
		}
	}
}

// this code is most likely to survive my purge of eduterm's X11 code
void SetTerminalSize(Terminal* terminal) {
	struct winsize ws = {
		.ws_col = terminal->buffer.size.x,
		.ws_row = terminal->buffer.size.y,
	};

	/* This is the very same ioctl that normal programs use to query the
	 * window size. Normal programs are actually able to do this, too,
	 * but it makes little sense: Setting the size has no effect on the
	 * PTY driver in the kernel (it just keeps a record of it) or the
	 * terminal emulator. IIUC, all that's happening is that subsequent
	 * ioctls will report the new size -- until another ioctl sets a new
	 * size.
	 *
	 * I didn't see any response to ioctls of normal programs in any of
	 * the popular terminals (XTerm, VTE, st). They are not informed by
	 * the kernel when a normal program issues an ioctl like that.
	 *
	 * On the other hand, if we were to issue this ioctl during runtime
	 * and the size actually changed, child programs would get a
	 * SIGWINCH. */
	if (ioctl(terminal->pty.master, TIOCSWINSZ, &ws) == -1) {
		perror("ioctl(TIOCSWINSZ)");
		exit(1);
	}
}

void PtPair(Pty* pty) {
	char *slave_name;

	/* Opens the PTY master device. This is the file descriptor that
	 * we're reading from and writing to in our terminal emulator.
	 *
	 * We're going for BSD-style management of the controlling terminal:
	 * Don't try to change anything now (O_NOCTTY), we'll issue an
	 * ioctl() later on. */
	pty->master = posix_openpt(O_RDWR | O_NOCTTY);
	if (pty->master == -1) {
		perror("posix_openpt");
		exit(1);
	}

	/* grantpt() and unlockpt() are housekeeping functions that have to
	 * be called before we can open the slave FD. Refer to the manpages
	 * on what they do. */
	if (grantpt(pty->master) == -1) {
		perror("grantpt");
		exit(1);
	}

	if (unlockpt(pty->master) == -1) {
		perror("grantpt");
		exit(1);
	}

	/* Up until now, we only have the master FD. We also need a file
	 * descriptor for our child process. We get it by asking for the
	 * actual path in /dev/pts which we then open using a regular
	 * open(). So, unlike pipe(), you don't get two corresponding file
	 * descriptors in one go. */

	slave_name = ptsname(pty->master);
	if (slave_name == NULL) {
		perror("ptsname");
		exit(1);
	}

	pty->slave = open(slave_name, O_RDWR | O_NOCTTY);
	if (pty->slave == -1) {
		perror("open(slave_name)");
		exit(1);
	}
}

void Spawn(Pty* pty, char** env) {
	pid_t p;
	//char* env[] = {"TERM=xterm-16color", NULL};

	size_t envSize = 0;
	for (size_t i = 0; env[i] != NULL; ++ i) {
		++ envSize;
	}
	char** envArray = (char**) SafeMalloc((envSize + 1) * sizeof(char*));

	for (size_t i = 0; i <= envSize; ++ i) {
		envArray[i] = env[i];
	}

	bool termSet = false;
	for (size_t i = 0; i < envSize; ++ i) {
		if (StringStartsWith(envArray[i], "TERM=")) {
			envArray[i] = "TERM=" TERM;
			termSet     = true;
		}
	}

	if (!termSet) {
		envArray = (char**) SafeRealloc(envArray, (envSize + 2) * sizeof(char**));

		envArray[envSize + 1] = NULL;
		envArray[envSize]     = "TERM=" TERM;
	}

	p = fork();
	if (p == 0) {
		close(pty->master);

		/* Create a new session and make our terminal this process'
		 * controlling terminal. The shell that we'll spawn in a second
		 * will inherit the status of session leader. */
		setsid();
		if (ioctl(pty->slave, TIOCSCTTY, NULL) == -1) {
			perror("ioctl(TIOCSCTTY)");
			exit(1);
		}

		dup2(pty->slave, 0);
		dup2(pty->slave, 1);
		dup2(pty->slave, 2);
		close(pty->slave);

		char* command = GetUserShell();

		//execle(SHELL, "-" SHELL, (char*) NULL, envArray);
		execle(command, command, (char*) NULL, envArray);
		free(envArray);
		exit(1); // TODO: make this close the tab or something
	}
	else if (p > 0) {
		free(envArray);
		close(pty->slave);
		return;
	}

	perror("fork");
	exit(1);
}
