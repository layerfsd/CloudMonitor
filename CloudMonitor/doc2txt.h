#ifndef _DOC2TXT_H__
#define _DOC2TXT_H__

#include <cstdio>
#include "parsedoc.h"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;


class Storage {
public:
  bool init(FILE *doc_file);
  bool stream(const char *name, const uchar **stream, uint *size) const;
private:
  uint stream_num;
  uchar **stream_name;
  uchar **stream_table;
  uint *stream_sizes;
public:
  Storage();
  ~Storage();
};


//bool doc_format(const Storage &st);
//bool doc_image(const Storage &storage, const char *image_dir_path);
//bool doc_property(const Storage &st);
//bool doc_text_file(const Storage &storage, FILE *text_file);
//char *doc_txt(const Storage &storage);
//bool parse_doc(const char *doc_file_path, const char *text_file_path, const char *image_dir_path);
//int DecodeUcs2Utf8(char *FileName);

#endif
