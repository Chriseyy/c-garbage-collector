#include "dyn_obj.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
  case DYN_DICTIONARY: {
    for (size_t i = 0; i < obj->data.v_dict.capacity; i++) {
      dyn_dict_entry_t *entry = obj->data.v_dict.buckets[i];
      while (entry != NULL) {
        dyn_dict_entry_t *next = entry->next;
        free(entry->key);             
        dyn_refcount_dec(entry->value); 
        free(entry);                   
        entry = next;
      }
    }
    free(obj->data.v_dict.buckets); 
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
  if (value == NULL) return NULL;

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

static unsigned long hash_string(const char *str) {
    // DJB2 Hash-Algorithmus
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash;
}
uint32_t hash_string_fnv1a(const char *str) {
    // FNV-1a Hash-Algorithmus
    uint32_t hash = 2166136261U; 
    uint32_t prime = 16777619U;

    while (*str) {
        hash ^= (unsigned char)(*str);
        hash *= prime;
        str++;
    }
    
    return hash;
}


dyn_obj_t *dyn_new_dictionary(size_t capacity) {
  if (capacity == 0) capacity = 16; 

  dyn_obj_t *obj = _new_dyn_object();
  if (obj == NULL) return NULL;

  dyn_dict_entry_t **buckets = calloc(capacity, sizeof(dyn_dict_entry_t *));
  if (buckets == NULL) {
    free(obj);
    return NULL;
  }

  obj->kind = DYN_DICTIONARY;
  obj->data.v_dict.capacity = capacity;
  obj->data.v_dict.size = 0;
  obj->data.v_dict.buckets = buckets;
  return obj;
}

bool dyn_dict_set(dyn_obj_t *dict, const char *key, dyn_obj_t *value) {
  if (dict == NULL || dict->kind != DYN_DICTIONARY || key == NULL || value == NULL) return false;

  size_t index = hash_string(key) % dict->data.v_dict.capacity;
  dyn_dict_entry_t *entry = dict->data.v_dict.buckets[index];

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      dyn_refcount_inc(value);
      dyn_refcount_dec(entry->value);
      entry->value = value;
      return true;
    }
    entry = entry->next;
  }

  dyn_dict_entry_t *new_entry = malloc(sizeof(dyn_dict_entry_t));
  if (new_entry == NULL) return false;

  new_entry->key = malloc(strlen(key) + 1);
  if (new_entry->key == NULL) {
    free(new_entry);
    return false;
  }
  strcpy(new_entry->key, key);

  dyn_refcount_inc(value);
  new_entry->value = value;

  new_entry->next = dict->data.v_dict.buckets[index];
  dict->data.v_dict.buckets[index] = new_entry;
  dict->data.v_dict.size++;

  return true;
}

dyn_obj_t *dyn_dict_get(dyn_obj_t *dict, const char *key) {
  if (dict == NULL || dict->kind != DYN_DICTIONARY || key == NULL) return NULL;

  size_t index = hash_string(key) % dict->data.v_dict.capacity;
  dyn_dict_entry_t *entry = dict->data.v_dict.buckets[index];

  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry->value; 
    }
    entry = entry->next;
  }

  return NULL; 
}