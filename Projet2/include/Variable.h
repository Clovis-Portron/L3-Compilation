#ifndef VARIABLE_H
#define VARIABLE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Stack.h"

#define INT -1
#define BOOL -2
#define VOID 0
#define ARRAY 1

#define false 0
#define true 1

// TP
struct Type
{
	int type; // BOOL|INT|ARRAY
	struct Type* child;
};

struct Variable
{
    int size;
    void* value;
    struct Type* type;
};

struct VariableList
{
	int size;
	struct Variable** list;
};

struct Type* Type_init(int type, struct Type* child);

int Type_check(struct Type* type1, struct Type* type2);


struct VariableList* VariableList_init();
void VariableList_append(struct VariableList* vlist, struct Variable* var);

/**
* Initialise une nouvelle variable 
* type : Type de la variable INT|BOOL|ARRAY(taille)
**/
struct Variable* Variable_init(struct Type* type);

void Variable_arraySetType(struct Variable* var, struct Type* type);

void Variable_set(struct Variable* var, int value);

void Variable_arraySet(struct Variable* var, int index, struct Type* value);

void Variable_arraySet2(struct Variable* var, struct Stack* stack, int index);
int Variable_get(struct Variable* var);

struct Variable* Variable_arrayGet(struct Variable* var, struct Stack* stack, int index);

void Variable_print(struct Variable* var);

/**
* Libère la mémoire associée à une variable 
* var: variable sur laquelle opérer
*/
void Variable_free(struct Variable* var);
#endif