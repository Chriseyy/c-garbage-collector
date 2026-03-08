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

void test_dict_gc() {
    printf("Teste Dictionary GC, Get & Resize... ");
    vm_t *vm = vm_new();
    frame_t *f1 = vm_new_frame(vm);

    dyn_obj_t *dict = dyn_new_dict(vm);
    frame_reference_object(f1, dict); 

    dyn_obj_t *k1 = dyn_new_string(vm, "name");
    dyn_obj_t *v1 = dyn_new_string(vm, "C-VM");
    dyn_dict_set(vm, dict, k1, v1);

    dyn_obj_t *k2 = dyn_new_string(vm, "version");
    dyn_obj_t *v2 = dyn_new_integer(vm, 1);
    dyn_dict_set(vm, dict, k2, v2);

    dyn_obj_t *res1 = dyn_dict_get(dict, k1);
    assert(res1 != NULL && strcmp(res1->data.v_string, "C-VM") == 0);

    dyn_obj_t *res2 = dyn_dict_get(dict, k2);
    assert(res2 != NULL && res2->data.v_int == 1);

    for (int i = 0; i < 8; i++) {
        char buf[16];
        sprintf(buf, "dyn_key_%d", i);
        dyn_obj_t *k = dyn_new_string(vm, buf);
        dyn_obj_t *v = dyn_new_integer(vm, i * 100);
        dyn_dict_set(vm, dict, k, v);
    }

    assert(dict->data.v_dict.capacity == 16); 
    assert(dict->data.v_dict.count == 10);    

    dyn_obj_t *res1_after = dyn_dict_get(dict, k1);
    assert(res1_after != NULL && strcmp(res1_after->data.v_string, "C-VM") == 0);

    assert(vm->objects->count == 21);

    vm_collect_garbage(vm);
    
    assert(vm->objects->count == 21);

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
    test_dict_gc(); // <-- NEU HINZUGEFÜGT

    printf("\n=== ALLE MARK-AND-SWEEP TESTS ERFOLGREICH BESTANDEN! ===\n");
    return 0;
}