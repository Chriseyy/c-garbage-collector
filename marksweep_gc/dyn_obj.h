#pragma once

#include "dyn_stack.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct VirtualMachine vm_t; 

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
  bool is_marked;

  dyn_kind_t kind;
  dyn_data_t data;
} dyn_obj_t;

dyn_obj_t *dyn_new_integer(vm_t *vm, int value);
dyn_obj_t *dyn_new_float(vm_t *vm, float value);
dyn_obj_t *dyn_new_string(vm_t *vm, const char *value);
dyn_obj_t *dyn_new_vector3(vm_t *vm, dyn_obj_t *x, dyn_obj_t *y, dyn_obj_t *z);
dyn_obj_t *dyn_new_array(vm_t *vm, size_t size);

void dyn_obj_free(dyn_obj_t *obj);
bool dyn_array_set(dyn_obj_t *array, size_t index, dyn_obj_t *value);
dyn_obj_t *dyn_array_get(dyn_obj_t *array, size_t index);
dyn_obj_t *dyn_add(vm_t *vm, dyn_obj_t *a, dyn_obj_t *b);