#include "dyn_obj.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void test_array_set() {
    dyn_obj_t *foo = dyn_new_integer(1);
    dyn_obj_t *array = dyn_new_array(1);
    
    dyn_array_set(array, 0, foo);
    assert(foo->refcount == 2);

    dyn_refcount_dec(array);
    assert(foo->refcount == 1);
    
    dyn_refcount_dec(foo);
}

void test_vector3_refcounting() {
    dyn_obj_t *x = dyn_new_integer(10);
    dyn_obj_t *y = dyn_new_integer(20);
    dyn_obj_t *z = dyn_new_integer(30);

    dyn_obj_t *vec = dyn_new_vector3(x, y, z);
    assert(x->refcount == 2);
    
    dyn_refcount_dec(vec);
    
    assert(x->refcount == 1);
    assert(y->refcount == 1);
    assert(z->refcount == 1);

    dyn_refcount_dec(x);
    dyn_refcount_dec(y);
    dyn_refcount_dec(z);
}

void test_allocated_string() {
    dyn_obj_t *str = dyn_new_string("Hello Garbage Collector!");
    assert(str->refcount == 1);
    
    dyn_refcount_inc(str);
    assert(str->refcount == 2);

    dyn_refcount_dec(str);
    assert(str->refcount == 1);
    
    dyn_refcount_dec(str);
}

int main() {
    printf("Starting Garbage Collector Tests...\n");

    test_array_set();
    test_vector3_refcounting();
    test_allocated_string();

    printf("SUCCESS: All tests passed perfectly!\n");
    return 0;
}


void test_deep_nesting() {
    printf("Starte Deep Nesting (Matroschka) Test...\n");

    dyn_obj_t *str_x = dyn_new_string("X-Achse");
    dyn_obj_t *str_y = dyn_new_string("Y-Achse");
    dyn_obj_t *str_z = dyn_new_string("Z-Achse");
    dyn_obj_t *vec = dyn_new_vector3(str_x, str_y, str_z);

    dyn_obj_t *inner_array = dyn_new_array(1);
    dyn_array_set(inner_array, 0, vec);

    dyn_obj_t *root_array = dyn_new_array(1);
    dyn_array_set(root_array, 0, inner_array);

    dyn_refcount_dec(str_x);
    dyn_refcount_dec(str_y);
    dyn_refcount_dec(str_z);
    dyn_refcount_dec(vec);
    dyn_refcount_dec(inner_array);

    dyn_refcount_dec(root_array); 
}