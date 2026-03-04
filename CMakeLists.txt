cmake_minimum_required(VERSION 3.10.0)
project(c-garbage-collector VERSION 0.1.0 LANGUAGES C)

add_executable(gc_refcount 
    refcount_gc/main_refcount.c 
    refcount_gc/dyn_obj.c
)
target_include_directories(gc_refcount PRIVATE refcount_gc)


add_executable(gc_marksweep 
    marksweep_gc/main_marksweep.c 
    marksweep_gc/dyn_obj.c 
    marksweep_gc/dyn_stack.c 
    marksweep_gc/dyn_vm.c
)
target_include_directories(gc_marksweep PRIVATE marksweep_gc)