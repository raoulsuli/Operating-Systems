#define _CRT_SECURE_NO_DEPRECATE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "HashMap.h"
#include "helpers.h"

char *slice_str(char *str, int start, int end) //used to get a substring from a string
{
	int i = 0, j = 0;
	char *buffer = calloc(sizeof(char), end - start + 2);

	if (!buffer)
		exit(12);

	for (i = start; i <= end; i++)
		buffer[j++] = str[i];
	buffer[j] = 0;
	return buffer;
}
//process the arguments from cli
int read_args(HashMap *hashMap, char **included_paths,
		char *input_file, char *output_file, int argc, char *argv[])
{

	int no_paths = 0, i = 1, res = 0;
	char *first = NULL, *initial = NULL, *key = NULL, *value = NULL;
	char *val = NULL; //variables to tokenize different strings

	while (i < argc) {
		first = slice_str(argv[i], 0, 0);
		initial = slice_str(argv[i], 0, 1); // "-*" type of args

		if (!first || !initial)
			return 12;

		if (strcmp(argv[i], "-D") == 0) {
			key = strtok(argv[i + 1], "=");
			value = strtok(NULL, "=");

			res = put(hashMap, key, value); //save the mapping
			if (res == 12)
				return 12;
			i += 2;
		} else if (strcmp(initial, "-D") == 0) { // -D No Whitespace
			if (strchr(argv[i], '=') != NULL) {
				key = strtok(argv[i], "=");
				value = strtok(NULL, "=");

				key = slice_str(key, 2, strlen(key));

				if (!key)
					return 12;

				res = put(hashMap, key, value);
				if (res == 12)
					return 12;
				free(key);
			} else {
				key = slice_str(argv[i], 2,
						strlen(argv[i]));

				if (!key)
					return 12;

				res = put(hashMap, key, "");
				if (res == 12)
					return 12;
				free(key);
			}
			i++;
		} else if (strcmp(argv[i], "-I") == 0) {
			strcpy(included_paths[no_paths++], argv[i + 1]);
			i += 2;
		} else if (strcmp(initial, "-I") == 0) {
			//same as before with -I
			val = slice_str(argv[i], 2, strlen(argv[i]));

			if (!val)
				return 12;

			strcpy(included_paths[no_paths++], val);
			i++;
			free(val);
		} else if (strcmp(argv[i], "-o") == 0) {
			strcpy(output_file, argv[i + 1]);
			i += 2;
		} else if (strcmp(initial, "-o") == 0) {
			val = slice_str(argv[i], 2, strlen(argv[i]));

			if (!val)
				return 12;

			strcpy(output_file, val);
			i++;
			free(val);
		} else if (strcmp(first, "-") == 0) {
			//if the -* directive isn't valid
			freeMap(hashMap);
			return -1;
		} else if (strcmp(input_file, "") == 0) {
			//input through stdin
			strcpy(input_file, argv[i]);
			i++;
		} else if (strcmp(output_file, "") == 0) {
			//same as input
			strcpy(output_file, argv[i]);
			i++;
		} else {
			freeMap(hashMap);
			return -1;
		}
		free(first);
		free(initial);
	}

	return no_paths;
}

int do_define(HashMap *hashMap, FILE *fp, char buffer[256])
{//process define directives
	char key[256], value[256], *mapEntry = NULL;
	char *left = NULL;
	int len = 0, n = 0, res = 0;

	fscanf(fp, "%255s", buffer);
	strcpy(key, buffer);

	fscanf(fp, "%255s", buffer);
	mapEntry = get(hashMap, buffer);

	if (mapEntry != NULL)
		strcpy(value, mapEntry);
	else
		strcpy(value, buffer);

	fscanf(fp, "%255s", buffer);
	len = strlen(buffer);

	if (strcmp(buffer, "\\") == 0) { //if it's multiline
		fscanf(fp, "%255s", buffer);
		left = NULL;

		while (strcmp(buffer, "+") == 0) {//if it contains expressions
			strcat(value, " + ");
			fscanf(fp, "%255s", buffer);
			n = strlen(buffer) - 2;

			left = slice_str(buffer, 0, n);

			if (!left)
				return 12;

			if (strcmp(left, "") != 0)
				strcat(value, left);
			else
				strcat(value, buffer);
			fscanf(fp, "%255s", buffer);
			free(left);
		}
		len = strlen(buffer);
		fseek(fp, -len + 1, SEEK_CUR);
	}

	if (strcmp(buffer, "+") == 0) {//expressions

		strcat(value, " + ");
		fscanf(fp, "%255s", buffer);
		strcat(value, buffer);

	} else
		fseek(fp, -len, SEEK_CUR);

	res = put(hashMap, key, value);
	if (res == 12)
		return 12;

	return 0;
}

int do_main(HashMap *hashMap, FILE *fp, char buffer[256], FILE *op)
{//process main function
	int n = strlen(buffer), found_true = 0, len = 0;
	char *value = NULL, b = buffer[n - 1], *key = NULL;

	if (strcmp(buffer, "#undef") == 0) {
		fscanf(fp, "%255s", buffer);
		removeKey(hashMap, buffer);
		return 0;
	}

	if (strcmp(buffer, "#ifdef") == 0 || strcmp(buffer, "#ifndef") == 0) {
		//#ifdef and #ifndef directives
		do_ifdef(hashMap, fp, buffer, op);
		fscanf(fp, "%255s", buffer);
	}

	if (strcmp(buffer, "#if") == 0) { //#if directive
		found_true = 0;

		while (strcmp(buffer, "#endif") != 0) {
			if (strcmp(buffer, "#else") == 0 &&
				found_true == 0) {

				found_true = 1;
				fscanf(fp, "%255s", buffer);

				while (strcmp(buffer, "#endif") != 0) {
					fputs(buffer, op);
					fscanf(fp, "%255s", buffer);
				}
				fputs("\n", op);
			}

			if (strcmp(buffer, "#endif") == 0)
				break;

			fscanf(fp, "%255s", buffer);

			if (found_true == 1)
				continue;

			if (strcmp(buffer, "1") == 0 ||
				strcmp(buffer, "TRUE") == 0) {
				found_true = 1;
				fscanf(fp, "%255s", buffer);

				while (strcmp(buffer, "#elif") != 0 &&
					strcmp(buffer, "#else") != 0 &&
					strcmp(buffer, "#endif") != 0) {
					fputs(buffer, op);
					fscanf(fp, "%255s", buffer);
				}
				fputs("\n", op);
			} else if (strcmp(buffer, "0") == 0 ||
				strcmp(buffer, "FALSE") == 0) {

				while (strcmp(buffer, "#elif") != 0 &&
					strcmp(buffer, "#else") != 0 &&
					strcmp(buffer, "#endif") != 0)
					fscanf(fp, "%255s", buffer);
			} else {
				value = get(hashMap, buffer);
				if (value != NULL && strcmp(value, "0") != 0 &&
						strcmp(value, "FALSE") != 0) {
					found_true = 1;
					fscanf(fp, "%255s", buffer);

					while (strcmp(buffer, "#elif") != 0 &&
					       strcmp(buffer, "#else") != 0 &&
					       strcmp(buffer, "#endif") != 0) {

						fputs(buffer, op);
						fscanf(fp, "%255s", buffer);
					}
					fputs("\n", op);
				} else
					while (strcmp(buffer, "#elif") != 0 &&
						strcmp(buffer, "#else") != 0 &&
						strcmp(buffer, "#endif") != 0)
						fscanf(fp, "%255s", buffer);
			}
		}

		fscanf(fp, "%255s", buffer);

		len = strlen(buffer);

		if (strcmp(buffer, "#if") == 0) {
			fseek(fp, -len, SEEK_CUR);
			return 0;
		}
	}

	if (strcmp(buffer, "}") == 0)
		fputs("\n", op);

	if (b == ';') {
		key = slice_str(buffer, 0, n - 3);

		if (!key)
			return 12;

		value = get(hashMap, key);
		free(key);
	} else
		value = get(hashMap, buffer);

	if (value != NULL) {
		if (b == ';')
			strcat(value, ");\n");
		else
			strcat(value, " ");
		fputs(value, op);

	} else {
		b =  buffer[strlen(buffer) - 1];

		if (b == ';' || b == '{')
			strcat(buffer, "\n");
		else
			strcat(buffer, " ");

		fputs(buffer, op);
	}
	return 0;
}

int do_ifdef(HashMap *hashMap, FILE *fp, char buffer[256], FILE *op)
{//#ifdef directive
	char *key = NULL, *value = NULL;
	int res = 0;

	if (strcmp(buffer, "#ifdef") == 0) {
		fscanf(fp, "%255s", buffer);
		value = get(hashMap, buffer);
		if (value != NULL) {
			while (strcmp(buffer, "#endif") != 0) {
				fscanf(fp, "%255s", buffer);
				if (strcmp(buffer, "#undef") == 0) {
					//undefine it
					fscanf(fp, "%255s", buffer);
					removeKey(hashMap, buffer);
				} else {
					if (strcmp(buffer, "#endif") == 0)
						return 0;
					fputs(buffer, op);
					fputs("\n", op);
				}
			}
		} else
			while (strcmp(buffer, "#endif") != 0)
				fscanf(fp, "%255s", buffer);
		return 0;
	} else if (strcmp(buffer, "#ifndef")) {
		//do sth if map doesn't contain the mapping
		fscanf(fp, "%255s", buffer);
		value = get(hashMap, buffer);

		if (value == NULL) {
			while (strcmp(buffer, "#endif") != 0) {
				fscanf(fp, "%255s", buffer);
				if (strcmp(buffer, "#define") == 0) {
					fscanf(fp, "%255s", buffer);
					key = buffer;

					fscanf(fp, "%255s", buffer);
					res = put(hashMap, key, buffer);
					if (res == 12)
						return 12;
				} else {
					fputs(buffer, op);
				}
			}
		} else
			while (strcmp(buffer, "#endif") != 0)
				fscanf(fp, "%255s", buffer);
		return 0;
	}
	return 0;
}

int do_include(HashMap *hashMap, char **included_paths, FILE *fp,
		char buffer[256], FILE *op, int no_paths)
{//process includes
	char *result = NULL, *newPath = calloc(sizeof(char), 256);
	FILE *p = NULL;
	int i = 0;

	if (!newPath)
		return 12;

	fscanf(fp, "%255s", buffer);
	result = slice_str(buffer, 1, strlen(buffer) - 2);

	if (!result)
		return 12;

	strcpy(newPath, "_test/inputs/");
	strcat(newPath, result);
	p = fopen(newPath, "r");

	if (p != NULL) { //if file in input is correct write it
		while (fscanf(p, "%255s", buffer) == 1) {

			if (strcmp(buffer, "#ifndef") == 0) {
				fscanf(p, "%255s", buffer);
				fscanf(p, "%255s", buffer);
				fscanf(p, "%255s", buffer);
				continue;
			}

			if (strcmp(buffer, "#endif") == 0)
				break;

			if (strcmp(buffer, "#define") == 0)

				do_define(hashMap, p, buffer);

			else {
				fputs(buffer, op);
				if (buffer[strlen(buffer) - 1] == ';')
					fputs("\n", op);
				else
					fputs(" ", op);
			}
		}
	} else { //check if files with params given exist
		if (no_paths == 0) {
			free(newPath);
			free(result);
			return -1;
		}

		for (i = 0; i < no_paths; i++) {
			strcpy(newPath, included_paths[i]);
			strcat(newPath, "/");
			strcat(newPath, result);
			p = fopen(newPath, "r");

			while (fscanf(p, "%255s", buffer) == 1) {
				if (strcmp(buffer, "#define") == 0)

					do_define(hashMap, p, buffer);

				else {
					fputs(buffer, op);
					if (buffer[strlen(buffer) - 1] == ';')
						fputs("\n", op);
					else
						fputs(" ", op);
				}
			}
			if (p != NULL)
				break;
		}
	}

	if (p != NULL)
		fclose(p);
	free(newPath);
	free(result);
	return 0;
}
