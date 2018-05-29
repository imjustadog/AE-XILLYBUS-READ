#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

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

struct xillyfifo {
  unsigned long write_position;
  unsigned long size;
  unsigned char *baseaddr;
};

struct xillyfifo strct_fifo;

void output_write(int fd, unsigned char *buf, int len);
void fifo_write(struct xillyfifo *fifo, unsigned char *buf, int len);
void fifo_output_write(struct xillyfifo *fifo, int fd);

int main(void) {

  int fd, rc, fdw, i;
  int flag = 0;
  int count = 0;
  int capture_count = 0;
  time_t ptime;
  unsigned char folder_name[30];
  unsigned char path[40];

  double time_before = 0.02;
  double voltage_threshold = 0.1;
  double time_after = 0.05;

  unsigned long before_cache = pow(2,(int)(1 + log2(time_before * 10000000 * 4))); //1048576
  int threshold = (voltage_threshold / 2.5 * 8192 + 8192)/256; //33
  unsigned long after_bags = pow(2,(int)(1 + log2(time_after * 10000000 / 32))); //16384

  struct xillyfifo *fifo = &strct_fifo;

  fifo->baseaddr = NULL;
  fifo->size = before_cache;
  fifo->write_position = 0;

  fifo->baseaddr = malloc(fifo->size);

  time(&ptime);
  strcpy(folder_name, ctime(&ptime));
  mkdir(folder_name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |S_IROTH | S_IWOTH |S_IXOTH);
  fd = open("/dev/xillybus_read_32", O_RDONLY);
  //fd = open("xillybus_read_32", O_RDONLY);

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
      perror("read() failed to read");
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
        	if(buf[i] >= threshold) {
        		flag = 1;
        		break;
        	}
        }
    }

    if(flag == 0) {
    	fifo_write(fifo, buf, rc);
    }

    if(flag == 1) {
    	sprintf(path, "%s/%d", folder_name, capture_count);
    	capture_count ++;
    	fdw = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    	fifo_output_write(fifo, fdw);
    	flag = 2;
    }

    if(flag == 2) {
    	fifo_write(fifo, buf, rc);
    	output_write(fdw, buf, rc);
    	count ++;
    	if(count == after_bags) {
    		count = 0;
    		flag = 0;
    		//exit(0);
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

void output_write(int fd, unsigned char *buf, int len) {
  int sent = 0;
  int rc;

  while (sent < len) {
    rc = write(fd, buf + sent, len - sent);

    if ((rc < 0) && (errno == EINTR))
      continue;

    if (rc < 0) {
      perror("output_write() failed to write");
      break;
    }

    if (rc == 0) {
      fprintf(stderr, "Reached write EOF (?!)\n");
      break;
    }

    sent += rc;
  }
}

void fifo_write(struct xillyfifo *fifo, unsigned char *buf, int len) {
	unsigned int num_to_fill;
	unsigned int num_left;
	if(fifo->write_position + len <= fifo->size) {
		memcpy(fifo->baseaddr + fifo->write_position, buf, len);
		fifo->write_position += len;
		if(fifo->write_position == fifo->size)
			fifo->write_position = 0;
	}
	else {
		num_to_fill = fifo->size - fifo->write_position;
		memcpy(fifo->baseaddr + fifo->write_position, buf, num_to_fill);
		num_left = len - num_to_fill;
		memcpy(fifo->baseaddr, (unsigned char *)(buf + num_to_fill), num_left);
		fifo->write_position = num_left;
	}
}

void fifo_output_write(struct xillyfifo *fifo, int fd) {
	int sent;
	int rc;
	unsigned long len_to_fill = fifo->size - fifo->write_position;
	unsigned long len_left = fifo->write_position;

	sent = 0;

	while (sent < len_to_fill) {
	  rc = write(fd, fifo->baseaddr + fifo->write_position + sent, len_to_fill - sent);

	  if ((rc < 0) && (errno == EINTR))
	    continue;

	  if (rc < 0) {
	    perror("fifo_output_write() failed to write (pre)");
	    break;
	  }

	  if (rc == 0) {
	    fprintf(stderr, "Reached write EOF (?!)\n");
	    break;
	  }

	  sent += rc;
	}

	sent = 0;

	while (sent < len_left) {
	  rc = write(fd, fifo->baseaddr + sent, len_left - sent);

	  if ((rc < 0) && (errno == EINTR))
	    continue;

	  if (rc < 0) {
	    perror("fifo_output_write() failed to write (after)");
	    break;
	  }

	  if (rc == 0) {
	    fprintf(stderr, "Reached write EOF (?!)\n");
	    break;
	  }

	  sent += rc;
	}

	fifo->write_position = 0;
}
