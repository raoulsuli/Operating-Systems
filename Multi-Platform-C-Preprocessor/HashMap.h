#ifndef _HASHMAP_H_
#define _HASHMAP_H_

typedef struct {
	int capacity, size;
	char **keys;
	char **values;
} HashMap;

HashMap *initialize_map(int capacity);
int check_size(HashMap *hashMap);
int put(HashMap *hashMap, char *key, char *val);
char *get(HashMap *hashMap, char *key);
void removeKey(HashMap *hashMap, char *key);
void freeMap(HashMap *hashMap);

#endif
