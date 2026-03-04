#ifndef DYN_OBJ_H
#define DYN_OBJ_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct DynObject dyn_obj_t;
typedef struct DynDictEntry dyn_dict_entry_t;

typedef struct {
  size_t size;
  dyn_obj_t **elements;
} dyn_array_t;

typedef struct DynDictEntry {
  char *key;
  dyn_obj_t *value;
  dyn_dict_entry_t *next;
} dyn_dict_entry_t;

typedef struct {
  size_t capacity; 
  size_t size;   
  dyn_dict_entry_t **buckets; 
} dyn_dict_t;


typedef struct {
  dyn_obj_t *x;
  dyn_obj_t *y;
  dyn_obj_t *z;
} dyn_vector3_t;

typedef enum DynObjectKind {
  DYN_INTEGER,
  DYN_FLOAT,
  DYN_STRING,
  DYN_VECTOR3,
  DYN_ARRAY,
  DYN_DICTIONARY,
} dyn_kind_t;

typedef union DynObjectData {
  int v_int;
  float v_float;
  char *v_string;
  dyn_vector3_t v_vector3;
  dyn_array_t v_array;
  dyn_dict_t v_dict;
} dyn_data_t;

typedef struct DynObject {
  int refcount;
  dyn_kind_t kind;
  dyn_data_t data;
} dyn_obj_t;

dyn_obj_t *dyn_new_integer(int value);
dyn_obj_t *dyn_new_float(float value);
dyn_obj_t *dyn_new_string(const char *value);
dyn_obj_t *dyn_new_vector3(dyn_obj_t *x, dyn_obj_t *y, dyn_obj_t *z);
dyn_obj_t *dyn_new_array(size_t size);
dyn_obj_t *dyn_new_dictionary(size_t capacity);


void dyn_refcount_inc(dyn_obj_t *obj);
void dyn_refcount_dec(dyn_obj_t *obj);
void dyn_refcount_free(dyn_obj_t *obj);

bool dyn_dict_set(dyn_obj_t *dict, const char *key, dyn_obj_t *value);
dyn_obj_t *dyn_dict_get(dyn_obj_t *dict, const char *key);
uint32_t hash_string_fnv1a(const char *str);

bool dyn_array_set(dyn_obj_t *array, size_t index, dyn_obj_t *value);
dyn_obj_t *dyn_array_get(dyn_obj_t *array, size_t index);

#endif