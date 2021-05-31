#ifndef _HELPERS_H_
#define _HELPERS_H_

char *slice_str(char *str, int start, int end);
int read_args(HashMap *hashMap, char **included_paths,
		char *input_file, char *output_file, int argc, char *argv[]);
int do_define(HashMap *hashMap, FILE *fp, char buffer[256]);
int do_main(HashMap *hashMap, FILE *fp, char buffer[256], FILE *op);
int do_ifdef(HashMap *hashMap, FILE *fp, char buffer[256], FILE *op);
int do_include(HashMap *hashMap, char **included_paths, FILE *fp,
		char buffer[256], FILE *op, int no_paths);

#endif
