#include "dyn_vm.h"
#include <stdlib.h>

void vm_collect_garbage(vm_t *vm) {
  mark(vm);  
  trace(vm);  
  sweep(vm);
}

void mark(vm_t *vm) {
  for (size_t i = 0; i < vm->frames->count; i++) {
    frame_t *frame = vm->frames->data[i];
    for (size_t j = 0; j < frame->references->count; j++) {
      dyn_obj_t *obj = frame->references->data[j];
      obj->is_marked = true;
    }
  }
}

void trace_mark_object(stack_t *gray_objects, dyn_obj_t *obj) {
  if (obj == NULL || obj->is_marked) {
    return;
  }
  stack_push(gray_objects, obj);
  obj->is_marked = true;
}

void trace_blacken_object(stack_t *gray_objects, dyn_obj_t *ref) {
  dyn_obj_t *obj = ref;
  if (obj == NULL) return;

  switch (obj->kind) {
  case DYN_INTEGER:
  case DYN_FLOAT:
  case DYN_STRING:
    break;
  case DYN_VECTOR3: {
    dyn_vector3_t vec = obj->data.v_vector3;
    trace_mark_object(gray_objects, vec.x);
    trace_mark_object(gray_objects, vec.y);
    trace_mark_object(gray_objects, vec.z);
    break;
  }
  case DYN_ARRAY: {
    for (size_t i = 0; i < obj->data.v_array.size; i++) {
      trace_mark_object(gray_objects, obj->data.v_array.elements[i]);
    }
    break;
  }
  }
}

void trace(vm_t *vm) {
  stack_t *gray_objects = stack_new(8);
  if (gray_objects == NULL) return;

  for (size_t i = 0; i < vm->objects->count; i++) {
    dyn_obj_t *obj = vm->objects->data[i];
    if (obj->is_marked) {
      stack_push(gray_objects, obj);
    }
  }

  while (gray_objects->count > 0) {
    trace_blacken_object(gray_objects, stack_pop(gray_objects));
  }

  stack_free(gray_objects);
}

void sweep(vm_t *vm) {
  for (size_t i = 0; i < vm->objects->count; i++) {
    dyn_obj_t *obj = (dyn_obj_t *)vm->objects->data[i];
    
    if (obj->is_marked) {
      obj->is_marked = false;
    } else {
      dyn_obj_free(obj);
      vm->objects->data[i] = NULL;
    }
  }
  stack_remove_nulls(vm->objects);
}


vm_t *vm_new() {
  vm_t *vm = malloc(sizeof(vm_t));
  if (vm == NULL) return NULL;

  vm->frames = stack_new(8);
  vm->objects = stack_new(8);
  return vm;
}

void vm_free(vm_t *vm) {
  for (size_t i = 0; i < vm->frames->count; i++) {
    frame_free(vm->frames->data[i]);
  }
  stack_free(vm->frames);

  for (size_t i = 0; i < vm->objects->count; i++) {
    dyn_obj_free(vm->objects->data[i]);
  }
  stack_free(vm->objects);

  free(vm);
}

void vm_track_object(vm_t *vm, dyn_obj_t *obj) {
  stack_push(vm->objects, obj);
}

void vm_frame_push(vm_t *vm, frame_t *frame) { stack_push(vm->frames, frame); }

frame_t *vm_frame_pop(vm_t *vm) { return stack_pop(vm->frames); }

frame_t *vm_new_frame(vm_t *vm) {
  frame_t *frame = malloc(sizeof(frame_t));
  frame->references = stack_new(8);
  vm_frame_push(vm, frame);
  return frame;
}

void frame_free(frame_t *frame) {
  stack_free(frame->references);
  free(frame);
}

void frame_reference_object(frame_t *frame, dyn_obj_t *obj) {
  stack_push(frame->references, obj);
}