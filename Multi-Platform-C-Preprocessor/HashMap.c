#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HashMap.h"

HashMap *initialize_map(int capacity)
{
	int i = 0;
	HashMap *hashMap = calloc(sizeof(HashMap), 1);

	if (!hashMap)
		exit(12);

	hashMap->capacity = capacity;
	hashMap->size = 0;

	hashMap->keys = calloc(sizeof(char *), capacity);
	hashMap->values = calloc(sizeof(char *), capacity);

	if (!hashMap->keys || !hashMap->values)
		exit(12);

	for (i = 0; i < capacity; i++) {
		hashMap->keys[i] = calloc(sizeof(char), 100);
		hashMap->values[i] = calloc(sizeof(char), 100);

		if (!hashMap->keys[i] || !hashMap->values[i])
			exit(12);
	}
	return hashMap;
}

int check_size(HashMap *hashMap) //check if map is full and double its capacity
{
	int i = 0, n = 0;

	if (hashMap->size == hashMap->capacity) {
		hashMap->capacity *= 2;
		hashMap->keys = realloc(hashMap->keys, sizeof(char *) *
				hashMap->capacity);
		hashMap->values = realloc(hashMap->values, sizeof(char *) *
				hashMap->capacity);

		if (!hashMap->keys || !hashMap->values)
			exit(12);

		n = hashMap->capacity;

		for (i = n / 2; i < n; i++) {
			hashMap->keys[i] = calloc(sizeof(char), 100);
			hashMap->values[i] = calloc(sizeof(char), 100);

			if (!hashMap->keys[i] || !hashMap->values[i])
				exit(12);
		}
	}

	return 0;
}

int put(HashMap *hashMap, char *key, char *val) //add a mapping
{
	int ret = check_size(hashMap);

	if (ret == 12)
		exit(12);

	strcpy(hashMap->keys[hashMap->size], key);
	if (val == NULL)
		strcpy(hashMap->values[hashMap->size], "");
	else
		strcpy(hashMap->values[hashMap->size], val);
	hashMap->size++;

	return 0;
}

char *get(HashMap *hashMap, char *key) //return it
{
	int i = 0;

	for (i = 0; i < hashMap->size; i++) {
		if (strcmp(hashMap->keys[i], key) == 0)
			return hashMap->values[i];
	}

	return NULL;
}

void removeKey(HashMap *hashMap, char *key) //delete it
{
	int i = 0, j = 0;

	for (i = 0; i < hashMap->size; i++) {
		if (strcmp(hashMap->keys[i], key) == 0) {
			for (j = i; j < hashMap->size; j++) {
				strcpy(hashMap->keys[j], hashMap->keys[j + 1]);
				strcpy(hashMap->values[j],
						hashMap->values[j + 1]);
			}
		}
	}
	hashMap->size--;
}

void freeMap(HashMap *hashMap) //free everything
{
	int i = 0;

	for (i = 0; i < hashMap->capacity; i++) {
		free(hashMap->keys[i]);
		free(hashMap->values[i]);
	}
	free(hashMap->keys);
	free(hashMap->values);
	free(hashMap);
}
