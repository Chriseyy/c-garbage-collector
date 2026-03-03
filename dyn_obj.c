#include "dyn_obj.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dyn_obj_t *_new_dyn_object() {
  dyn_obj_t *obj = calloc(1, sizeof(dyn_obj_t));
  if (obj == NULL) {
    return NULL;
  }
  obj->refcount = 1;
  return obj;
}

void dyn_refcount_inc(dyn_obj_t *obj) {
  if (obj == NULL) {
    return;
  }
  obj->refcount++;
}

void dyn_refcount_free(dyn_obj_t *obj) {
  if (obj == NULL) return;

  switch (obj->kind) {
  case DYN_INTEGER:
  case DYN_FLOAT:
    break;
  case DYN_STRING:
    free(obj->data.v_string);
    break;
  case DYN_VECTOR3: {
    dyn_vector3_t vec = obj->data.v_vector3;
    dyn_refcount_dec(vec.x);
    dyn_refcount_dec(vec.y);
    dyn_refcount_dec(vec.z);
    break;
  }
  case DYN_ARRAY: {
    for (size_t i = 0; i < obj->data.v_array.size; i++) {
      dyn_refcount_dec(obj->data.v_array.elements[i]);
    }
    free(obj->data.v_array.elements);
    break;
  }
  default:
    assert(false && "Unknown object kind");
  }
  free(obj);
}

void dyn_refcount_dec(dyn_obj_t *obj) {
  if (obj == NULL) {
    return;
  }
  obj->refcount--;
  if (obj->refcount == 0) {
    dyn_refcount_free(obj);
  }
}

bool dyn_array_set(dyn_obj_t *dyn_obj, size_t index, dyn_obj_t *value) {
  if (dyn_obj == NULL || value == NULL) return false;
  if (dyn_obj->kind != DYN_ARRAY) return false;
  if (index >= dyn_obj->data.v_array.size) return false;

  dyn_refcount_inc(value);
  dyn_refcount_dec(dyn_obj->data.v_array.elements[index]);
  
  dyn_obj->data.v_array.elements[index] = value;
  return true;
}

dyn_obj_t *dyn_array_get(dyn_obj_t *dyn_obj, size_t index) {
  if (dyn_obj == NULL || dyn_obj->kind != DYN_ARRAY) return NULL;
  if (index >= dyn_obj->data.v_array.size) return NULL;

  return dyn_obj->data.v_array.elements[index];
}

dyn_obj_t *dyn_new_integer(int value) {
  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;
  obj->kind = DYN_INTEGER;
  obj->data.v_int = value;
  return obj;
}

dyn_obj_t *dyn_new_float(float value) {
  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;
  obj->kind = DYN_FLOAT;
  obj->data.v_float = value;
  return obj;
}

dyn_obj_t *dyn_new_string(const char *value) {
  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;

  int len = strlen(value);
  char *dst = malloc(len + 1);
  if (dst == NULL) {
    free(obj);
    return NULL;
  }
  strcpy(dst, value);

  obj->kind = DYN_STRING;
  obj->data.v_string = dst;
  return obj;
}

dyn_obj_t *dyn_new_vector3(dyn_obj_t *x, dyn_obj_t *y, dyn_obj_t *z) {
  if (x == NULL || y == NULL || z == NULL) return NULL;
  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;

  obj->kind = DYN_VECTOR3;
  obj->data.v_vector3 = (dyn_vector3_t){.x = x, .y = y, .z = z};
  
  dyn_refcount_inc(x);
  dyn_refcount_inc(y);
  dyn_refcount_inc(z);
  return obj;
}

dyn_obj_t *dyn_new_array(size_t size) {
  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;

  dyn_obj_t **elements = calloc(size, sizeof(dyn_obj_t *));
  if (elements == NULL) {
    free(obj);
    return NULL;
  }

  obj->kind = DYN_ARRAY;
  obj->data.v_array = (dyn_array_t){.size = size, .elements = elements};
  return obj;
}