#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#define MAGIC ("vermagic=")
#define MAGIC_LEN (sizeof(MAGIC) - 1)

static int write_all(const char *ptr, size_t bytes, FILE *stream)
{
	size_t written;

	written = 0;

	while (bytes > 0) {
		if ((written = fwrite(ptr, 1, bytes, stream)) <= 0) return 1;
		ptr = &ptr[written];
		bytes -= written;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	size_t bytes_read;
	int replaced;
	char *loc;
	char buf[1 * 1024 * 1024];

	replaced = 0;

	while ((bytes_read = fread(buf, 1, sizeof(buf), stdin)) > 0) {
		if (!replaced && (loc = memmem(
				buf, bytes_read, MAGIC, MAGIC_LEN))) {
			while (loc < &buf[bytes_read] && loc[0] != '\0') {
				loc[0] = '\0';
				loc = &loc[1];
			}
			if (loc == &buf[bytes_read]) return 2;
			replaced = 1;
		}

		if (write_all(buf, bytes_read, stdout)) return 3;
	}

	return !replaced;
}
