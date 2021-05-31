#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "so_stdio.h"

#define BUF_SIZE 4096
#define READ 0
#define WRITE 1

struct _so_file {
	int fd, curr_pos, eof, error, size;
	char buffer[BUF_SIZE];
	int last_operation;
};

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	int fd;
	SO_FILE *f = NULL;

	if (strcmp(mode, "r") == 0)
		fd = open(pathname, O_RDONLY, 0644);
	else if (strcmp(mode, "r+") == 0)
		fd = open(pathname, O_RDWR, 0644);
	else if (strcmp(mode, "w") == 0)
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (strcmp(mode, "w+") == 0)
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
	else if (strcmp(mode, "a") == 0)
		fd = open(pathname, O_APPEND | O_WRONLY | O_CREAT, 0644);
	else if (strcmp(mode, "a+") == 0)
		fd = open(pathname, O_APPEND | O_RDWR | O_CREAT, 0644);
	else
		return NULL;

	if (fd < 0)
		return NULL;

	f = calloc(sizeof(SO_FILE), 1);
	f->fd = fd;
	f->curr_pos = 0;
	f->eof = 0;
	f->error = 0;
	f->size = 0;
	f->last_operation = -1;
	memset(f->buffer, 0, BUF_SIZE);
	return f;
}

FUNC_DECL_PREFIX int so_fclose(SO_FILE *stream)
{
	int retC;

	if (stream == NULL)
		return SO_EOF;

	so_fflush(stream);

	retC = close(stream->fd);

	if (retC == SO_EOF) {
		stream->error = 1;
		free(stream);
		return SO_EOF;
	}

	free(stream);

	return retC;
}

FUNC_DECL_PREFIX int so_fileno(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	return stream->fd;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream)
{
	ssize_t bytes_read = 0;
	unsigned char c = 0;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}

	if (stream->size == 0) {
		bytes_read = read(stream->fd, stream->buffer, BUF_SIZE);
		if (bytes_read < 0) {
			stream->error = 1;
			return SO_EOF;
		} else if (bytes_read == 0) {
			stream->eof = 1;
			return SO_EOF;
		}
		stream->size = bytes_read;
		stream->curr_pos = 0;
	}

	c = (unsigned char) stream->buffer[stream->curr_pos++];
	stream->size--;

	return (int) c;
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream)
{
	ssize_t bytes_written = 0;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}

	if (stream->size == BUF_SIZE) {
		bytes_written = write(stream->fd, stream->buffer, BUF_SIZE);
		if (bytes_written == -1) {
			stream->error = 1;
			return SO_EOF;
		}
		stream->curr_pos = 0;
		stream->size = BUF_SIZE -  bytes_written;
	}

	stream->buffer[stream->curr_pos++] = (unsigned char)c;
	stream->size++;
	return c;
}

FUNC_DECL_PREFIX size_t so_fread(void *ptr, size_t size, size_t nmemb,
		SO_FILE *stream)
{
	ssize_t bytes_read = 0;
	int i = 0, c = 0;

	if (size <= 0 || nmemb <= 0 || stream == NULL)
		return 0;

	for (i = 0; i < nmemb * size; i++) {
		c = so_fgetc(stream);

		if (so_ferror(stream))
			return 0;

		if (c == SO_EOF)
			return bytes_read;

		if (i % size == 0)
			bytes_read++;

		memcpy(ptr + i, (char *) &c, 1);
	}
	stream->last_operation = READ;
	return bytes_read;
}

FUNC_DECL_PREFIX size_t so_fwrite(const void *ptr, size_t size, size_t nmemb,
		SO_FILE *stream)
{
	ssize_t bytes_written = 0;
	int i = 0, ret = 0;


	if (size <= 0 || nmemb <= 0 || stream == NULL)
		return 0;

	for (i = 0; i < nmemb * size; i++) {
		ret = so_fputc(*((int *)(ptr + i)), stream);

		if (so_ferror(stream))
			return 0;

		if (ret == SO_EOF)
			return bytes_written;

		if (i % size == 0)
			bytes_written++;
	}

	stream->last_operation = WRITE;
	return bytes_written;
}

FUNC_DECL_PREFIX int so_fflush(SO_FILE *stream)
{
	int bytes_written = 0;

	if (stream == NULL)
		return SO_EOF;
	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}

	bytes_written = write(stream->fd, stream->buffer, stream->size);
	if (bytes_written == -1) {
		stream->error = 1;
		return SO_EOF;
	}
	stream->curr_pos = 0;
	stream->size = 0;
	return 0;
}

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence)
{
	int ret = 0;

	if (stream == NULL)
		return SO_EOF;
	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}
	ret = lseek(stream->fd, offset, whence);
	if (ret == -1)
		return SO_EOF;

	if (stream->last_operation == READ) {
		memset(stream->buffer, 0, BUF_SIZE);
		stream->curr_pos = 0;
		stream->size = 0;
	} else
		so_fflush(stream);
	return 0;
}

FUNC_DECL_PREFIX long so_ftell(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;
	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}
	return lseek(stream->fd, 0, SEEK_CUR) + stream->size;
}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;
	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}

	return (stream->eof == 1) ? 1 : 0;
}

FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;
	if (stream->fd < 0) {
		stream->error = 1;
		return SO_EOF;
	}

	return (stream->error == 1) ? 1 : 0;
}

FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type)
{
	;
}

FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream)
{
	;
}
