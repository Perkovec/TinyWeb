#ifndef MIME_TYPES_H_
#define MIME_TYPES_H_

struct mimeItem{
  char *ext;
	char *filetype;
};

extern struct mimeItem mimeTypes[];

#endif // MIME_TYPES_H_