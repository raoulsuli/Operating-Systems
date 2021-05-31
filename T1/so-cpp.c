#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HashMap.h"
#include "helpers.h"

#define FILES_SIZE 50
#define MAX_LENGTH 256

int main(int argc, char *argv[])
{
	HashMap *hashMap = initialize_map(argc);
	char **included_paths = calloc(sizeof(char *), argc);//includes array
	char *input_file = calloc(sizeof(char), FILES_SIZE);
	char *output_file = calloc(sizeof(char), FILES_SIZE);
	FILE *fp = stdin;
	FILE *op = stdout;
	char buffer[MAX_LENGTH]; //buffer used to read words from file
	int main_found = 0, i = 0, no_paths = 0, res = 0;

	if (!hashMap || !input_file || !output_file || !included_paths)
		return 12;

	for (i = 0; i < argc; i++) {
		included_paths[i] = calloc(sizeof(char), FILES_SIZE);
		if (!included_paths[i])
			return 12;
	}

	no_paths = read_args(hashMap, included_paths,
			input_file, output_file, argc, argv);
	//number of include directives

	if (no_paths == -1 || no_paths == 12)
		return no_paths;

	//if input_file is given change the file descriptor
	if (strcmp(input_file, "") != 0) {
		fp = fopen(input_file, "r");
		if (strcmp(output_file, "") != 0) //same with output
			op = fopen(output_file, "w");
	}

	if (fp == NULL) { //if file could not be open
		freeMap(hashMap);
		return -1;
	}
	 //read word by word from input
	while (fscanf(fp, "%255s", buffer) == 1) {
		if (strcmp(buffer, "#define") == 0) {//define clause

			res = do_define(hashMap, fp, buffer);
			if (res == 12)
				return 12;
		 //found the main clause
		} else if (strcmp(buffer, "int") == 0 && main_found == 0) {

			main_found = 1;
			fputs(buffer, op);
			fputs(" ", op);
			fgets(buffer, 256, fp);
			fputs(buffer, op);
		} else if (main_found == 1) {//process the main part

			res = do_main(hashMap, fp, buffer, op);
			if (res == 12)
				return 12;
		} else if (strcmp(buffer, "#ifdef") == 0 ||
				strcmp(buffer, "#ifndef") == 0) {

			res = do_ifdef(hashMap, fp, buffer, op);
			if (res == 12)
				return 12;
		} else if (strcmp(buffer, "#include") == 0) {
			res = do_include(hashMap, included_paths, fp, buffer,
			op, no_paths);
			if (res == -1 || res == 12) {
				main_found = 1;
				break;
			}
		}
	}

	 //if file has no main it means it's just some words
	if (main_found == 0) {
		fseek(fp, 0, SEEK_SET);
		while (fscanf(fp, "%255s", buffer) == 1)
			fputs(buffer, op);
	}

	//freeing and closing descriptors part
	fclose(fp);
	fclose(op);
	free(input_file);
	free(output_file);
	for (i = 0; i < argc; i++)
		free(included_paths[i]);
	free(included_paths);
	freeMap(hashMap);
	if (res == -1)
		return -1;
	return 0;

}
