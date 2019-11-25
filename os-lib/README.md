#  Simple libraries

This program creates static and 2 dynamic libraries then uses them. 

## Static library:
 * Compiles statically
 * Provides some functions
 
## First dynamic library:
 * Compiles dynamically
 * Links dynamically
 * Provides some functions

## Second dynamic library:
 * Compiles dynamically
 * Provides some functions

## Program:
 * Loads second lybrary in runtime and using ```dlopen(3)``` invokes provided functions