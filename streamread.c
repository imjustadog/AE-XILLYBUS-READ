#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* streamread.c -- Demonstrate read from a Xillybus FIFO.

This simple command-line application is given one argument: The device
file to read from. The read data is sent to standard output.

This program has no advantage over the classic UNIX 'cat' command. It was
written merely to demonstrate the coding technique.

We don't use allread() here (see memread.c), because if less than the
desired number of bytes arrives, they should be handled immediately.

See http://www.xillybus.com/doc/ for usage examples an information.

*/
unsigned char buf[128];

void allwrite(int fd, unsigned char *buf, int len);

int main(void) {

  int fd, rc, fdw, i;
  int flag = 0;
  int count = 0;

  fd = open("/dev/xillybus_read_32", O_RDONLY);
  fdw = open("/home/adoge/xillybus_save_32", O_WRONLY | O_TRUNC);

  if (fd < 0) {
    if (errno == ENODEV)
      fprintf(stderr, "(Maybe it is a write-only file?)\n");

    perror("Failed to open devfile");
    exit(1);
  }

  while (1) {
    rc = read(fd, buf, sizeof(buf));

    if ((rc < 0) && (errno == EINTR))
      continue;

    if (rc < 0) {
      perror("allread() failed to read");
      exit(1);
    }

    if (rc == 0) {
      fprintf(stderr, "Reached read EOF.\n");
      exit(0);
    }

    // Write all data to standard output = file descriptor 1
    // rc contains the number of bytes that were read.
    if(flag == 0) {
        for(i = 1;i < rc;i += 4) {
        	if(buf[i] >= 33) {
        		flag = 1;
        		break;
        	}
        }
    }
    if(flag == 1) {
    	allwrite(fdw, buf, rc);
    	count ++;
    	if(count == 16384) {
    		count = 0;
    		flag = 0;
    		exit(0);
    	}
    }
  }
}

/*
   Plain write() may not write all bytes requested in the buffer, so
   allwrite() loops until all data was indeed written, or exits in
   case of failure, except for EINTR. The way the EINTR condition is
   handled is the standard way of making sure the process can be suspended
   with CTRL-Z and then continue running properly.

   The function has no return value, because it always succeeds (or exits
   instead of returning).

   The function doesn't expect to reach EOF either.
*/

void allwrite(int fd, unsigned char *buf, int len) {
  int sent = 0;
  int rc;

  while (sent < len) {
    rc = write(fd, buf + sent, len - sent);

    if ((rc < 0) && (errno == EINTR))
      continue;

    if (rc < 0) {
      perror("allwrite() failed to write");
      exit(1);
    }

    if (rc == 0) {
      fprintf(stderr, "Reached write EOF (?!)\n");
      exit(1);
    }

    sent += rc;
  }
}
