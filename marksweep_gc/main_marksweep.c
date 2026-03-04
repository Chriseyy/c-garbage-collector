#include "dyn_vm.h"
#include "dyn_obj.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_primitives_gc() {
    printf("Teste Primitives (Integer, Float, String) in VM... ");
    vm_t *vm = vm_new();
    frame_t *f1 = vm_new_frame(vm);

    dyn_obj_t *my_int = dyn_new_integer(vm, 42);
    dyn_obj_t *my_str = dyn_new_string(vm, "Hallo GC");
    
    frame_reference_object(f1, my_int);
    frame_reference_object(f1, my_str);

    assert(vm->objects->count == 2);

    vm_collect_garbage(vm);
    assert(vm->objects->count == 2);

    frame_free(vm_frame_pop(vm));
    vm_collect_garbage(vm);
    
    assert(vm->objects->count == 0);
    
    vm_free(vm);
    printf("OK!\n");
}

void test_nested_objects_gc() {
    printf("Teste verschachtelte Objekte (Vector & Array)... ");
    vm_t *vm = vm_new();
    frame_t *f1 = vm_new_frame(vm);

    dyn_obj_t *x = dyn_new_integer(vm, 10);
    dyn_obj_t *y = dyn_new_integer(vm, 20);
    dyn_obj_t *z = dyn_new_integer(vm, 30);
    
    dyn_obj_t *vec = dyn_new_vector3(vm, x, y, z);
    
    frame_reference_object(f1, vec);

    assert(vm->objects->count == 4);

    vm_collect_garbage(vm);
    assert(vm->objects->count == 4);

    frame_free(vm_frame_pop(vm));
    vm_collect_garbage(vm);
    assert(vm->objects->count == 0);

    vm_free(vm);
    printf("OK!\n");
}

void test_circular_reference_gc() {
    printf("Teste ZIRKULÄRE Referenzen (Der ultimative Test!)... ");
    vm_t *vm = vm_new();
    frame_t *f1 = vm_new_frame(vm);

    dyn_obj_t *arr1 = dyn_new_array(vm, 1);
    dyn_obj_t *arr2 = dyn_new_array(vm, 1);

    dyn_array_set(arr1, 0, arr2);
    dyn_array_set(arr2, 0, arr1);

    frame_reference_object(f1, arr1);

    assert(vm->objects->count == 2);

    frame_free(vm_frame_pop(vm));
    
    vm_collect_garbage(vm);
    
    assert(vm->objects->count == 0);

    vm_free(vm);
    printf("OK!\n");
}

void test_addition_gc() {
    printf("Teste Addition & String-Verkettung GC... ");
    vm_t *vm = vm_new();
    frame_t *f1 = vm_new_frame(vm);

    dyn_obj_t *a = dyn_new_integer(vm, 10);
    dyn_obj_t *b = dyn_new_integer(vm, 32);
    
    dyn_obj_t *res = dyn_add(vm, a, b);

    assert(res->data.v_int == 42);
    assert(vm->objects->count == 3); 

    frame_reference_object(f1, res);

    vm_collect_garbage(vm);
    assert(vm->objects->count == 1); 
    assert(vm->objects->data[0] == res);

    dyn_obj_t *s1 = dyn_new_string(vm, "Hello ");
    dyn_obj_t *s2 = dyn_new_string(vm, "World");
    dyn_obj_t *s_res = dyn_add(vm, s1, s2);

    assert(strcmp(s_res->data.v_string, "Hello World") == 0);
    
    frame_free(vm_frame_pop(vm)); 
    vm_collect_garbage(vm);
    assert(vm->objects->count == 0);

    vm_free(vm);
    printf("OK!\n");
}

int main() {
    printf("=== Starte MARK-AND-SWEEP GC Tests ===\n\n");

    test_primitives_gc();
    test_nested_objects_gc();
    test_circular_reference_gc();
    test_addition_gc(); 

    printf("\n=== ALLE MARK-AND-SWEEP TESTS ERFOLGREICH BESTANDEN! ===\n");
    return 0;
}