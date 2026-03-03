#ifndef DYN_OBJ_H
#define DYN_OBJ_H

#include <stdbool.h>
#include <stddef.h>

typedef struct DynObject dyn_obj_t;

typedef struct {
  size_t size;
  dyn_obj_t **elements;
} dyn_array_t;

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
} dyn_kind_t;

typedef union DynObjectData {
  int v_int;
  float v_float;
  char *v_string;
  dyn_vector3_t v_vector3;
  dyn_array_t v_array;
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

void dyn_refcount_inc(dyn_obj_t *obj);
void dyn_refcount_dec(dyn_obj_t *obj);
void dyn_refcount_free(dyn_obj_t *obj);

bool dyn_array_set(dyn_obj_t *array, size_t index, dyn_obj_t *value);
dyn_obj_t *dyn_array_get(dyn_obj_t *array, size_t index);

#endif