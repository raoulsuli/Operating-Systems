#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "so_stdio.h"
#define BUF_SIZE 4096

#define READ 0
#define WRITE 1

struct _so_file {
	HANDLE fd;
	int curr_pos, error, eof, size;
	char buffer[BUF_SIZE];
	int last_operation;
};

FUNC_DECL_PREFIX SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	HANDLE fd;
	SO_FILE *f = NULL;

	if (strcmp(mode, "r") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (strcmp(mode, "r+") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (strcmp(mode, "w") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (strcmp(mode, "w+") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (strcmp(mode, "a") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	else if (strcmp(mode, "a+") == 0)
		fd = CreateFile(pathname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	else
		return NULL;

	if (fd == NULL || fd == INVALID_HANDLE_VALUE)
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
	int retC, retF;

	if (stream == NULL)
		return SO_EOF;

	retF = so_fflush(stream);

	retC = CloseHandle(stream->fd);

	if (retC == 0 || retF == SO_EOF) {
		stream->error = 1;
		free(stream);
		return SO_EOF;
	}

	free(stream);

	return 0;
}

FUNC_DECL_PREFIX HANDLE so_fileno(SO_FILE *stream)
{
	if (stream == NULL)
		return INVALID_HANDLE_VALUE;

	return stream->fd;
}

FUNC_DECL_PREFIX int so_fgetc(SO_FILE *stream)
{
	BOOL ret = FALSE;
	unsigned char c = 0;
	int bytes_read = 0;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}

	if (stream->size == 0) { //if there is no data in buffer, fill it
		ret = ReadFile(stream->fd, stream->buffer, BUF_SIZE, &bytes_read, NULL);
		if (bytes_read < 0) {
			stream->error = 1;
			return SO_EOF;
		} else if (bytes_read == 0) {
			stream->eof = 1;
			return SO_EOF;
		} else if (ret == FALSE) {
			stream->error = 1;
			return SO_EOF;
		}

		stream->size = bytes_read;
		stream->curr_pos = 0;
	}

	c = (unsigned char) stream->buffer[stream->curr_pos++];//retrieve first
	stream->size--;					//unread character

	return (int) c;
}

FUNC_DECL_PREFIX int so_fputc(int c, SO_FILE *stream)
{
	int bytes_written = 0;
	BOOL ret = FALSE;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}

	if (stream->size == BUF_SIZE) {//if there is no space available in
		ret = WriteFile(stream->fd, stream->buffer, BUF_SIZE, //buffer
		&bytes_written, NULL);			//empty it
		if (bytes_written == -1) {
			stream->error = 1;
			return SO_EOF;
		} else if (ret == FALSE) {
			stream->error = 1;
			return SO_EOF;
		}
		stream->curr_pos = 0;
		stream->size = BUF_SIZE -  bytes_written;
	}

	stream->buffer[stream->curr_pos++] = (unsigned char)c; //add character
	stream->size++;
	return c;
}

FUNC_DECL_PREFIX size_t so_fread(void *ptr, size_t size, size_t nmemb,
		SO_FILE *stream)
{
	int bytes_read = 0;
	int c = 0;
	size_t i;

	if (size <= 0 || nmemb <= 0 || stream == NULL)
		return 0;

	for (i = 0; i < nmemb * size; i++) { //read char by char
		c = so_fgetc(stream);

		if (so_ferror(stream))
			return 0;

		if (c == SO_EOF) {
			stream->error = 1;
			return bytes_read;
		}
		if (i % size == 0)
			bytes_read++;

		memcpy((char *)ptr + i, (char *) &c, 1); //copy it in array
	}
	stream->last_operation = READ;
	return bytes_read;
}

FUNC_DECL_PREFIX size_t so_fwrite(const void *ptr, size_t size, size_t nmemb,
		SO_FILE *stream)
{
	size_t bytes_written = 0;
	int ret = 0;
	size_t i = 0;

	if (size <= 0 || nmemb <= 0 || stream == NULL)
		return 0;

	for (i = 0; i < nmemb * size; i++) {
		ret = so_fputc(*((char *)ptr + i), stream);

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
	BOOL ret = FALSE;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}

	ret = WriteFile(stream->fd, stream->buffer, stream->size,
	&bytes_written, NULL);
	if (bytes_written == -1 || ret == FALSE) {
		stream->error = 1;
		return SO_EOF;
	}
	stream->curr_pos = 0;
	stream->size = 0;
	return 0;
}

FUNC_DECL_PREFIX int so_fseek(SO_FILE *stream, long offset, int whence)
{
	DWORD ret = 0;

	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}
	ret = SetFilePointer(stream->fd, offset, NULL, whence);
	if (ret == INVALID_SET_FILE_POINTER)
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

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}
	return SetFilePointer(stream->fd, 0, NULL, FILE_CURRENT) + stream->size;
}

FUNC_DECL_PREFIX int so_feof(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}

	return (stream->eof == 1) ? 1 : 0;
}

FUNC_DECL_PREFIX int so_ferror(SO_FILE *stream)
{
	if (stream == NULL)
		return SO_EOF;

	if (stream->fd == INVALID_HANDLE_VALUE) {
		stream->error = 1;
		return SO_EOF;
	}

	return (stream->error == 1) ? 1 : 0;
}
FUNC_DECL_PREFIX SO_FILE *so_popen(const char *command, const char *type)
{
	return 0;
}

FUNC_DECL_PREFIX int so_pclose(SO_FILE *stream)
{
	return 0;
}
