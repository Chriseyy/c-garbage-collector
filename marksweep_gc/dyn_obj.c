#include "dyn_obj.h"
#include "dyn_vm.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

dyn_obj_t *_new_dyn_object(vm_t *vm) {
  dyn_obj_t *obj = calloc(1, sizeof(dyn_obj_t));
  if (obj == NULL) return NULL;
  
  obj->is_marked = false;
  vm_track_object(vm, obj); 
  
  return obj;
}

dyn_obj_t *dyn_new_integer(vm_t *vm, int value) {
  dyn_obj_t *obj = _new_dyn_object(vm);
  if (obj == NULL) return NULL;
  obj->kind = DYN_INTEGER;
  obj->data.v_int = value;
  return obj;
}

dyn_obj_t *dyn_new_float(vm_t *vm, float value) {
  dyn_obj_t *obj = _new_dyn_object(vm);
  if (obj == NULL) return NULL;
  obj->kind = DYN_FLOAT;
  obj->data.v_float = value;
  return obj;
}

dyn_obj_t *dyn_new_string(vm_t *vm, const char *value) {
  dyn_obj_t *obj = _new_dyn_object(vm);
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

dyn_obj_t *dyn_new_vector3(vm_t *vm, dyn_obj_t *x, dyn_obj_t *y, dyn_obj_t *z) {
  if (x == NULL || y == NULL || z == NULL) return NULL;

  dyn_obj_t *obj = _new_dyn_object(vm);
  if (obj == NULL) return NULL;

  obj->kind = DYN_VECTOR3;
  obj->data.v_vector3 = (dyn_vector3_t){.x = x, .y = y, .z = z};
  return obj;
}

dyn_obj_t *dyn_new_array(vm_t *vm, size_t size) {
  dyn_obj_t *obj = _new_dyn_object(vm);
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

static uint32_t hash_string(const char* key) {
    uint32_t hash = 2166136261u;
    for (int i = 0; key[i] != '\0'; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

dyn_obj_t *dyn_new_dict(vm_t *vm) {
  dyn_obj_t *obj = _new_dyn_object(vm);
  if (obj == NULL) return NULL;

  obj->kind = DYN_DICT;
  obj->data.v_dict.capacity = 8;
  obj->data.v_dict.count = 0;
  obj->data.v_dict.entries = calloc(8, sizeof(dyn_dict_entry_t)); 
  
  return obj;
}
static void resize_dict(dyn_dict_t *dict) {
    size_t new_capacity = dict->capacity * 2;
    
    dyn_dict_entry_t *new_entries = calloc(new_capacity, sizeof(dyn_dict_entry_t));

    for (size_t i = 0; i < dict->capacity; i++) {
        dyn_dict_entry_t *entry = &dict->entries[i];
        
        if (entry->key != NULL) {
            uint32_t index = hash_string(entry->key->data.v_string) % new_capacity;
            

            while (new_entries[index].key != NULL) {
                index = (index + 1) % new_capacity;
            }
            
            new_entries[index] = *entry;
        }
    }

    free(dict->entries);
    dict->entries = new_entries;
    dict->capacity = new_capacity;
}

bool dyn_dict_set(vm_t *vm, dyn_obj_t *dict, dyn_obj_t *key, dyn_obj_t *value) {
    if (dict == NULL || key == NULL || dict->kind != DYN_DICT || key->kind != DYN_STRING) return false;

    dyn_dict_t *d = &dict->data.v_dict;

    if (d->count + 1 > (d->capacity * 3) / 4) {
        resize_dict(d); 
    }

    uint32_t index = hash_string(key->data.v_string) % d->capacity;

    while (d->entries[index].key != NULL) {
        if (strcmp(d->entries[index].key->data.v_string, key->data.v_string) == 0) {
            d->entries[index].value = value;
            return true; 
        }
        index = (index + 1) % d->capacity;
    }

    d->entries[index].key = key;
    d->entries[index].value = value;
    d->count++;

    return true;
}

dyn_obj_t *dyn_dict_get(dyn_obj_t *dict, dyn_obj_t *key) {
    if (dict == NULL || key == NULL || dict->kind != DYN_DICT || key->kind != DYN_STRING) return NULL;

    dyn_dict_t *d = &dict->data.v_dict;
    if (d->count == 0) return NULL; 

    uint32_t index = hash_string(key->data.v_string) % d->capacity;

    while (d->entries[index].key != NULL) {
        if (strcmp(d->entries[index].key->data.v_string, key->data.v_string) == 0) {
            return d->entries[index].value; 
        }
        index = (index + 1) % d->capacity;
    }

    return NULL; 
}


void dyn_obj_free(dyn_obj_t *obj) {
  if (obj == NULL) return;

  switch (obj->kind) {
  case DYN_INTEGER:
  case DYN_FLOAT:
    break;
  case DYN_STRING:
    free(obj->data.v_string);
    break;
  case DYN_VECTOR3: 
    break; 
  case DYN_ARRAY: 
    free(obj->data.v_array.elements); 
    break;
  case DYN_DICT:
    free(obj->data.v_dict.entries);
    break;
  }
  free(obj);
}

bool dyn_array_set(dyn_obj_t *array, size_t index, dyn_obj_t *value) {
  if (array == NULL || value == NULL || array->kind != DYN_ARRAY) return false;
  if (index >= array->data.v_array.size) return false;

  array->data.v_array.elements[index] = value;
  return true;
}

dyn_obj_t *dyn_array_get(dyn_obj_t *array, size_t index) {
  if (array == NULL || array->kind != DYN_ARRAY) return NULL;
  if (index >= array->data.v_array.size) return NULL;

  return array->data.v_array.elements[index];
}

dyn_obj_t *dyn_add(vm_t *vm, dyn_obj_t *a, dyn_obj_t *b) {
    if (a == NULL || b == NULL) {
        return NULL;
    }

    switch (a->kind) {
    case DYN_INTEGER:
        switch (b->kind) {
        case DYN_INTEGER:
            return dyn_new_integer(vm, a->data.v_int + b->data.v_int);
        case DYN_FLOAT:
            return dyn_new_float(vm, (float)a->data.v_int + b->data.v_float);
        default:
            return NULL;
        }
    case DYN_FLOAT:
        switch (b->kind) {
        case DYN_FLOAT:
            return dyn_new_float(vm, a->data.v_float + b->data.v_float);
        case DYN_INTEGER:
            return dyn_new_float(vm, a->data.v_float + (float)b->data.v_int);
        default:
            return NULL;
        }
    case DYN_STRING:
        if (b->kind == DYN_STRING) {
            size_t a_len = strlen(a->data.v_string);
            size_t b_len = strlen(b->data.v_string);
            char *dst = malloc(a_len + b_len + 1);
            if (dst == NULL) return NULL;

            strcpy(dst, a->data.v_string);
            strcat(dst, b->data.v_string);

            dyn_obj_t *obj = dyn_new_string(vm, dst);
            free(dst);
            return obj;
        }
        return NULL;

    case DYN_VECTOR3:
        if (b->kind == DYN_VECTOR3) {
            return dyn_new_vector3(
                vm,
                dyn_add(vm, a->data.v_vector3.x, b->data.v_vector3.x),
                dyn_add(vm, a->data.v_vector3.y, b->data.v_vector3.y),
                dyn_add(vm, a->data.v_vector3.z, b->data.v_vector3.z)
            );
        }
        return NULL;

    case DYN_ARRAY:
        if (b->kind == DYN_ARRAY) {
            size_t a_len = a->data.v_array.size;
            size_t b_len = b->data.v_array.size;
            dyn_obj_t *new_arr = dyn_new_array(vm, a_len + b_len);
            if (new_arr == NULL) return NULL;

            for (size_t i = 0; i < a_len; i++) {
                dyn_array_set(new_arr, i, dyn_array_get(a, i));
            }
            for (size_t i = 0; i < b_len; i++) {
                dyn_array_set(new_arr, i + a_len, dyn_array_get(b, i));
            }
            return new_arr;
        }
        return NULL;

    default:
        return NULL;
    }
}