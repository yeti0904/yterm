#include "components.h"
#include "constants.h"
#include "terminal.h"

bool SetTerminalSize(struct PTY* pty, struct X11* x11) {
	struct winsize ws = {
		.ws_col = x11->buf_w,
		.ws_row = x11->buf_h,
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
	if (ioctl(pty->master, TIOCSWINSZ, &ws) == -1) {
		perror("ioctl(TIOCSWINSZ)");
		return false;
	}

	return true;
}

bool PtPair(struct PTY *pty) {
	char *slave_name;

	/* Opens the PTY master device. This is the file descriptor that
	 * we're reading from and writing to in our terminal emulator.
	 *
	 * We're going for BSD-style management of the controlling terminal:
	 * Don't try to change anything now (O_NOCTTY), we'll issue an
	 * ioctl() later on. */
	pty->master = posix_openpt(O_RDWR | O_NOCTTY);
	if (pty->master == -1)
	{
		perror("posix_openpt");
		return false;
	}

	/* grantpt() and unlockpt() are housekeeping functions that have to
	 * be called before we can open the slave FD. Refer to the manpages
	 * on what they do. */
	if (grantpt(pty->master) == -1)
	{
		perror("grantpt");
		return false;
	}

	if (unlockpt(pty->master) == -1)
	{
		perror("grantpt");
		return false;
	}

	/* Up until now, we only have the master FD. We also need a file
	 * descriptor for our child process. We get it by asking for the
	 * actual path in /dev/pts which we then open using a regular
	 * open(). So, unlike pipe(), you don't get two corresponding file
	 * descriptors in one go. */

	slave_name = ptsname(pty->master);
	if (slave_name == NULL) {
		perror("ptsname");
		return false;
	}

	pty->slave = open(slave_name, O_RDWR | O_NOCTTY);
	if (pty->slave == -1) {
		perror("open(slave_name)");
		return false;
	}

	return true;
}

bool Spawn(struct PTY *pty) {
	pid_t p;
	char* env[] = {"TERM=dumb", NULL};

	p = fork();
	if (p == 0) {
		close(pty->master);

		/* Create a new session and make our terminal this process'
		 * controlling terminal. The shell that we'll spawn in a second
		 * will inherit the status of session leader. */
		setsid();
		if (ioctl(pty->slave, TIOCSCTTY, NULL) == -1)
		{
			perror("ioctl(TIOCSCTTY)");
			return false;
		}

		dup2(pty->slave, 0);
		dup2(pty->slave, 1);
		dup2(pty->slave, 2);
		close(pty->slave);

		execle(SHELL, "-" SHELL, (char *)NULL, env);
		return false;
	}
	else if (p > 0) {
		close(pty->slave);
		return true;
	}

	perror("fork");
	return false;
}
