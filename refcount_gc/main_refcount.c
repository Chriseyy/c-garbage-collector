#include "dyn_obj.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void test_primitives() {
    printf("Teste Integer und Float... ");
    dyn_obj_t *my_int = dyn_new_integer(42);
    dyn_obj_t *my_float = dyn_new_float(3.14f);

    assert(my_int != NULL);
    assert(my_int->kind == DYN_INTEGER);
    assert(my_int->data.v_int == 42);
    assert(my_int->refcount == 1);

    assert(my_float != NULL);
    assert(my_float->kind == DYN_FLOAT);
    assert(my_float->data.v_float == 3.14f); 

    dyn_refcount_dec(my_int);
    dyn_refcount_dec(my_float);
    printf("OK!\n");
}

void test_string() {
    printf("Teste String (Malloc, Kopie & NULL-Check)... ");
    
    dyn_obj_t *my_str = dyn_new_string("Hallo Welt");
    assert(my_str != NULL);
    assert(my_str->kind == DYN_STRING);
    assert(strcmp(my_str->data.v_string, "Hallo Welt") == 0);
    assert(my_str->refcount == 1);
    dyn_refcount_dec(my_str);

    dyn_obj_t *null_str = dyn_new_string(NULL);
    assert(null_str == NULL);

    printf("OK!\n");
}


void test_vector() {
    printf("Teste Vector3 (Adoption von Objekten)... ");
    dyn_obj_t *x = dyn_new_integer(1);
    dyn_obj_t *y = dyn_new_integer(2);
    dyn_obj_t *z = dyn_new_integer(3);

    dyn_obj_t *vec = dyn_new_vector3(x, y, z);
    
    assert(vec != NULL);
    assert(vec->kind == DYN_VECTOR3);
    assert(x->refcount == 2); 


    dyn_refcount_dec(vec);
    assert(x->refcount == 1);
    
    dyn_refcount_dec(x);
    dyn_refcount_dec(y);
    dyn_refcount_dec(z);
    printf("OK!\n");
}

void test_array_logic() {
    printf("Teste Array (Setzen, Überschreiben, Lesen)... ");
    dyn_obj_t *arr = dyn_new_array(3);
    assert(arr != NULL);
    assert(arr->kind == DYN_ARRAY);
    assert(arr->data.v_array.size == 3);

    dyn_obj_t *val1 = dyn_new_integer(100);
    dyn_obj_t *val2 = dyn_new_integer(200);


    assert(dyn_array_set(arr, 0, val1) == true);
    assert(val1->refcount == 2);


    dyn_obj_t *fetched = dyn_array_get(arr, 0);
    assert(fetched == val1);

    assert(dyn_array_set(arr, 0, val2) == true);
    assert(val1->refcount == 1); 
    assert(val2->refcount == 2); 

    assert(dyn_array_set(arr, 99, val1) == false);
    assert(dyn_array_get(arr, 99) == NULL);

    dyn_refcount_dec(val1);
    dyn_refcount_dec(val2);
    dyn_refcount_dec(arr);
    printf("OK!\n");
}

void test_manual_refcount() {
    printf("Teste manuelle Refcount-Kontrolle... ");
    dyn_obj_t *obj = dyn_new_integer(99);
    
    dyn_refcount_inc(obj);
    assert(obj->refcount == 2);
    
    dyn_refcount_dec(obj);
    assert(obj->refcount == 1);
    
    dyn_refcount_dec(obj); 
    printf("OK!\n");
}

void test_dictionary() {
    printf("Teste Dictionary (Hash Map, Kollisionen, Memory)... ");
    dyn_obj_t *dict = dyn_new_dictionary(8);
    assert(dict != NULL);

    dyn_obj_t *val_name = dyn_new_string("Bob");
    dyn_obj_t *val_age = dyn_new_integer(25);

    assert(dyn_dict_set(dict, "name", val_name) == true);
    assert(dyn_dict_set(dict, "age", val_age) == true);

    assert(val_name->refcount == 2); 
    
    dyn_obj_t *fetched_name = dyn_dict_get(dict, "name");
    assert(fetched_name == val_name);

    dyn_obj_t *val_new_age = dyn_new_integer(26);
    assert(dyn_dict_set(dict, "age", val_new_age) == true);
    assert(val_age->refcount == 1); 

    dyn_refcount_dec(val_name);
    dyn_refcount_dec(val_age);
    dyn_refcount_dec(val_new_age);
    dyn_refcount_dec(dict);

    printf("OK!\n");
}

void test_hash_function() {
    printf("Teste FNV-1a Hash-Funktion... ");
    
    uint32_t hash1 = hash_string_fnv1a("Apfel");
    uint32_t hash2 = hash_string_fnv1a("Apfel");
    assert(hash1 == hash2);

    uint32_t hash3 = hash_string_fnv1a("Banane");
    assert(hash1 != hash3);

    uint32_t hash4 = hash_string_fnv1a("apfel");
    assert(hash1 != hash4);
    
    uint32_t hash_empty = hash_string_fnv1a("");
    assert(hash_empty == 2166136261U);

    printf("OK!\n");
}

int main() {
    printf("=== Starte Garbage Collector Tests ===\n\n");

    test_primitives();
    test_string();
    test_vector();
    test_array_logic();
    test_dictionary();
    test_hash_function();
    test_manual_refcount();

    printf("\n=== ALLE TESTS ERFOLGREICH BESTANDEN! ===\n");
    return 0;
}