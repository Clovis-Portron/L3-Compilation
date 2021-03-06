//
// Created by clovis on 30/03/17.
//

#include "Pascal.h"

int affectingIndex = false;

void Pascal_semanticFunctions(struct FuncList* functions)
{
    for(int i = 0; i < functions->length; i++)
    {
        for(int u = 0; u < functions->length; u++)
        {
            if(i == u || functions->list[i] == functions->list[u])
                continue;
            if(strcmp(functions->list[i]->disclaimer->name, functions->list[u]->disclaimer->name) == 0)
            {
                if(FuncDiclaimer_check(functions->list[i]->disclaimer, functions->list[u]->disclaimer) == true)
                {
                    if(u > i)
                    {
                        functions->list[i] = functions->list[u];
                    }
                    else
                    {
                        functions->list[u] = functions->list[i];
                    }
                }
                else
                {
                    Pascal_semantic(false, "Cant redeclare functions with different signatures.");
                }
            }
        }
    }
}

struct Variable* Pascal_run( struct Stack* stack, struct Env* env, struct FuncList* functions, struct Ast* ast, struct Env* local)
{
    if(ast == 0)
        return 0;
    if(TypeSystem_isInit() == false)
        TypeSystem_init();
    char nodeType = ast->nodetype;

    if(nodeType == 'C') // commandes
    {
        int ope = *(int*)ast->value;
        int tmp;
        struct Variable* tmp1;
        struct Variable* tmp2;
        struct Type* tmp3;
        struct Func* tmp4;
        struct Env* tmp5;
        struct Env* tmp7;
        struct Ast* tmp6;
        switch(ope) {
            case Se:
                Pascal_run(stack, env, functions, ast->left, local);
                Pascal_run(stack, env, functions, ast->right, local);
                Collector_clean(stack);
                break;
            case AfInd:
                tmp1 = Pascal_run(stack, env, functions, ast->left->left, local); // Tableau à modifier
                tmp2 = Pascal_run(stack, env, functions, ast->left->right, local); //Index a modifier
                tmp = Variable_arrayGet(tmp2, stack, 0);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local); //valeur a placer
                if(tmp2->type->desc != ARRAY) {
                    Stack_setValue(stack, stack->adr[tmp1->value] + tmp, Variable_arrayGet(tmp2, stack, 0));
#ifdef DEBUG
                    printf("AfInd: %d <- %d (tmp: %d adr: %d)\n", stack->adr[tmp1->value]+tmp, Variable_arrayGet(tmp2, stack, 0), tmp, stack->adr[tmp1->value]);
#endif
                }
                else
                {
                    Stack_setValue(stack, stack->adr[tmp1->value] + tmp, tmp2->value);
#ifdef DEBUG
                    printf("AfInd (array): %d <- %d (tmp: %d adr: %d)\n", stack->adr[tmp1->value]+tmp, tmp2->value, tmp, stack->adr[tmp1->value]);
#endif
                }
                break;
            case Af:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                if(tmp1->type->desc != ARRAY) {
#ifdef DEBUG
                    printf("%s <- %d\n", (char*)ast->left->value, Variable_arrayGet(tmp2, stack, 0));
#endif
                    Variable_arraySet(tmp1, stack,0, Variable_arrayGet(tmp2, stack, 0));
                }
                else {
                    tmp1->value = tmp2->value;
                }
                Collector_clean(stack);
                break;
            case Sk:
                Collector_clean(stack);
                break;
            case If:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                if(Variable_arrayGet(tmp1, stack, 0) == true)
                {
                    Pascal_run(stack, env, functions, ast->right->left, local);
                } else
                {
                    Pascal_run(stack, env, functions, ast->right->right, local);
                }
                Collector_clean(stack);
                break;
            case Wh:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                while(Variable_arrayGet(tmp1, stack, 0) == true)
                {
                    Pascal_run(stack, env, functions, ast->right, local);
                    tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                }
                Collector_clean(stack);
                break;
            case CallFUNC:
                //printf("Searching to call %s\n", (char*)ast->left->value);
                tmp4 = FuncList_search(functions, ((char*)ast->left->value));

                // création de l'environnement local
                // Ajout des variables arguments
                tmp5 = Env_concat(tmp4->disclaimer->args, tmp4->vars);
                tmp6 = ast->right;
                // Copie des valeurs des arguments dans le nouvel environnement
                for(int i = 0; i < tmp4->disclaimer->args->length; i++)
                {
                    tmp1 = Pascal_run(stack, env, functions, tmp6->right,local);
                    Env_set_value_index(tmp5, i, tmp1);
                    tmp6 = tmp6->left;
                }
                // execution de la fonction
                Pascal_run(stack, env, functions, tmp4->ast, tmp5);
                free(tmp5);
                Collector_clean(stack);
                break;
        }
        return 0;
    }
    else if(nodeType == 'E') // expressions
    {
        int ope = *(int*)ast->value;
        int tmp;
        struct Variable* var1;
        struct Variable* var2;
        struct Variable* tmp1;
        struct Variable* tmp2;
        struct Type* tmp3;
        struct Func* tmp4;
        struct Env* tmp5;
        struct Env* tmp7;
        struct Ast* tmp6;
        switch(ope)
        {
            case Pl:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = Variable_arrayGet(tmp1, stack, 0) + Variable_arrayGet(tmp2, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Mo:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = Variable_arrayGet(tmp1, stack, 0) - Variable_arrayGet(tmp2, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Mu:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = Variable_arrayGet(tmp1, stack, 0) * Variable_arrayGet(tmp2, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Or:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == true || Variable_arrayGet(tmp2, stack, 0) == true)
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Lt:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) < Variable_arrayGet(tmp2, stack, 0))
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Eq:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == Variable_arrayGet(tmp2, stack, 0))
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case And:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp2 = Pascal_run(stack, env, functions, ast->right, local);
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == true && Variable_arrayGet(tmp2, stack, 0) == true)
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Not:
                tmp1 = Pascal_run(stack, env, functions, ast->left, local);
                tmp = !Variable_arrayGet(tmp1, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case NewAr:
                tmp3 = (struct Type*)ast->left->value;
                tmp = Variable_arrayGet(Pascal_run(stack, env, functions, ast->right, local), stack, 0);
                tmp1 = Variable_arrayInit(stack, tmp3, tmp);
                tmp1->refs = 0;
                //printf("New Array (index: %d adr: %d size: %d) \n",tmp1->value, stack->adr[tmp1->value], tmp);
                //Stack_print(stack);
                return tmp1;
            case GetARR:
                // Tableau
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                // Index
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                tmp = Variable_arrayGet(tmp2, stack, 0);
                // Valeur Tableau[index]
                tmp = Variable_arrayGet(tmp1, stack, tmp);
                if(tmp1->type->child->desc != ARRAY)
                {
                    tmp2 = Variable_arrayInit(stack, tmp1->type->child, 1);
                    Variable_arraySet(tmp2, stack, 0, tmp);
                    tmp2->refs = 0;
                    return tmp2;
                }
                else {
                    tmp2 = Variable_arrayRef(tmp1->type->child, tmp);
                    tmp2->refs = 0;
                    return tmp2;
                }
            case CallFUNC:
                tmp4 = FuncList_search(functions, ((char*)ast->left->value));
                // création de l'environnement local
                // Ajout des variables arguments
                tmp5 = Env_concat(tmp4->disclaimer->args, tmp4->vars);
                tmp6 = ast->right;
                // Copie des valeurs des arguments dans le nouvel environnement
                for(int i = 0; i < tmp4->disclaimer->args->length; i++)
                {
                    tmp1 = Pascal_run(stack, env, functions, tmp6->right, local);
                    Env_set_value_index(tmp5, i, tmp1);
                    tmp6 = tmp6->left;
                }
                // Ajout d'une variable portant le nom de la fonction faisant office de return
                Env_add_value(tmp5, (char*)ast->left->value, Variable_arrayInit(stack, tmp4->disclaimer->type, 1));
                // execution de la fonction
                //printf("Calling... %p\n", tmp4->ast);
                Pascal_run(stack, env, functions, tmp4->ast, tmp5);
                //printf("Done\n");
                // Récupération de la variable faisant office de return
                tmp1 = Env_get_value(tmp5, (char*)ast->left->value);
                //printf("Returning value %d\n", Variable_arrayGet(tmp1, stack, 0));
                free(tmp5);
                tmp1->refs = 0;
                return tmp1;



        }
    }
    else if(nodeType == 'I') // constantes
    {
        int tmp = *(int*)ast->value;
        struct Variable* tmp1 = Variable_arrayInit(stack, Type_INT,1);
        Variable_arraySet(tmp1, stack, 0, tmp);
        tmp1->refs = 0;
        return tmp1;
    }
    else if(nodeType == 'B') // constantes
    {
        int tmp = *(int*)ast->value;
        struct Variable* tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
        Variable_arraySet(tmp1, stack, 0, tmp);
        tmp1->refs = 0;
        return tmp1;
    }
    else if(nodeType == 'V')
    {
        struct Variable* res = 0;
        if(local != 0)
            res = Env_get_value(local, (char*)ast->value);
        if(res == 0)
            res = Env_get_value(env, (char*)ast->value);
        return res;
    }
}

void Pascal_semantic(int cond, char* error)
{
    if(cond == false)
    {
        printf("SEMANTIC: %s\n", error);
        exit(-1);
    }
}

void Pascal_semanticAnalyse(struct Stack* stack, struct Env* global, struct FuncList* functions, struct Ast* ast, struct Env* local)
{
#ifdef DEBUG
    printf("SEMANTIC ANALYSE\n");
#endif
    struct Env* tmp;
    // Lancement de l'analyse sur les AST des fonctions
    for(int i = 0; i < functions->length; i++)
    {

        Pascal_semantic(Env_checkEnvCollision(functions->list[i]->disclaimer->args, functions->list[i]->vars), "Cant re-declare vars.");
        tmp = Env_concat(functions->list[i]->disclaimer->args, functions->list[i]->vars);
        Pascal_semantic(Env_checkEnvCollision(global, tmp), "Cant re-declare vars.");
        Pascal_semantic(Env_key_exists(global, functions->list[i]->disclaimer->name) == false, "Cant declare var with reserved func/proc name.");
        Pascal_semantic(Env_key_exists(tmp, functions->list[i]->disclaimer->name) == false, "Cant declare var with reserved func/proc name.");
        Env_add_value(tmp, functions->list[i]->disclaimer->name, Variable_arrayInit(stack, functions->list[i]->disclaimer->type, 1));
        Pascal_semanticAnalyseEval(stack, global, functions, functions->list[i]->ast, tmp);
        Env_free(tmp);
    }
    Pascal_semanticAnalyseEval(stack, global, functions, ast, local);
#ifdef DEBUG
    printf("SEMANTIC ANALYSE DONE\n========\n");
#endif

}

struct Variable* Pascal_semanticAnalyseEval(struct Stack* stack, struct Env* env, struct FuncList* functions, struct Ast* ast, struct Env* local)
{
    if(ast == 0)
        return 0;
    if(TypeSystem_isInit() == false)
        TypeSystem_init();
    char nodeType = ast->nodetype;

    if(nodeType == 'C') // commandes
    {
        int ope = *(int*)ast->value;
        int tmp;
        struct Variable* tmp1;
        struct Variable* tmp2;
        struct Type* tmp3;
        struct Func* tmp4;
        struct Env* tmp5;
        struct Env* tmp7;
        struct Ast* tmp6;
        char* msg = malloc(0);
        switch(ope) {
            case Se:
                Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Collector_clean(stack);
                break;
            case AfInd:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left->left, local); // Tableau à modifier
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left->right, local); //Index a modifier
                tmp = Variable_arrayGet(tmp2, stack, 0);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local); //valeur a placer
                Pascal_semantic(Type_check(tmp1->type->child, tmp2->type), "Cant allocate different types.");
                free(msg);
                msg = malloc(0);
                sprintf(msg, "Index out of range %d >= %d", tmp, stack->size[tmp1->value]);
                Pascal_semantic(tmp < stack->size[tmp1->value], msg);
                if(tmp2->type->desc != ARRAY) {
                    Stack_setValue(stack, stack->adr[tmp1->value] + tmp, Variable_arrayGet(tmp2, stack, 0));
#ifdef DEBUG
                    printf("AfInd: %d <- %d (tmp: %d adr: %d)\n", stack->adr[tmp1->value]+tmp, Variable_arrayGet(tmp2, stack, 0), tmp, stack->adr[tmp1->value]);
#endif
                }
                else
                {
                    Stack_setValue(stack, stack->adr[tmp1->value] + tmp, tmp2->value);
#ifdef DEBUG
                    printf("AfInd (array): %d <- %d (tmp: %d adr: %d)\n", stack->adr[tmp1->value]+tmp, tmp2->value, tmp, stack->adr[tmp1->value]);
#endif
                }
                break;
            case Af:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(Type_check(tmp1->type, tmp2->type), "Cant allocate different types.");
                if(tmp1->type->desc != ARRAY) {
#ifdef DEBUG
                    printf("%s <- %d\n", (char*)ast->left->value, Variable_arrayGet(tmp2, stack, 0));
#endif
                    Variable_arraySet(tmp1, stack,0, Variable_arrayGet(tmp2, stack, 0));
                }
                else {
                    tmp1->value = tmp2->value;
                }
                Collector_clean(stack);
                break;
            case Sk:
                Collector_clean(stack);
                break;
            case If:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Pascal_semantic(tmp1->type->desc == BOOL, "If condition must be boolean.");
                Pascal_semanticAnalyseEval(stack, env, functions, ast->right->left, local);
                Pascal_semanticAnalyseEval(stack, env, functions, ast->right->right, local);
                Collector_clean(stack);
                break;
            case Wh:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Pascal_semantic(tmp1->type->desc == BOOL, "While condition must be boolean.");
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Collector_clean(stack);
                break;
            case CallFUNC:
                //printf("Searching to call %s\n", (char*)ast->left->value);
                tmp4 = FuncList_search(functions, ((char*)ast->left->value));
                Pascal_semantic(tmp4 != 0, "Undeclared func/proc.");

                // création de l'environnement local
                // Ajout des variables arguments
                tmp5 = Env_concat(tmp4->disclaimer->args, tmp4->vars);

                tmp6 = ast->right;
                // Copie des valeurs des arguments dans le nouvel environnement
                for(int i = 0; i < tmp4->disclaimer->args->length; i++)
                {
                    Pascal_semantic(tmp6->right != 0, "Too few arguments.");
                    tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, tmp6->right,local);
                    Pascal_semantic(Type_check(tmp1->type, Env_get_value_index(tmp4->disclaimer->args, i)->type) != false, "Type mismatch.");
                    Env_set_value_index(tmp5, i, tmp1);
                    tmp6 = tmp6->left;
                }
                Pascal_semantic(tmp6 == 0, "Too many arguments.");
                // execution de la fonction
                free(tmp5);
                Collector_clean(stack);
                break;
        }
        return 0;
    }
    else if(nodeType == 'E') // expressions
    {
        int ope = *(int*)ast->value;
        int tmp;
        struct Variable* var1;
        struct Variable* var2;
        struct Variable* tmp1;
        struct Variable* tmp2;
        struct Type* tmp3;
        struct Func* tmp4;
        struct Env* tmp5;
        struct Env* tmp7;
        struct Ast* tmp6;
        switch(ope)
        {
            case Pl:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == INT && tmp2->type->desc == INT, "Cant PL on non-integer.");
                tmp = 0;
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Mo:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == INT && tmp2->type->desc == INT, "Cant MO on non-integer.");
                tmp = Variable_arrayGet(tmp1, stack, 0) - Variable_arrayGet(tmp2, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Mu:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == INT && tmp2->type->desc == INT, "Cant MU on non-integer.");
                tmp = Variable_arrayGet(tmp1, stack, 0) * Variable_arrayGet(tmp2, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_INT,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Or:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == BOOL && tmp2->type->desc == BOOL, "Cant OR on non-bool.");
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == true || Variable_arrayGet(tmp2, stack, 0) == true)
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Lt:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == INT && tmp2->type->desc == INT, "Cant LT on non-integer.");
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) < Variable_arrayGet(tmp2, stack, 0))
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Eq:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == INT && tmp2->type->desc == INT, "Cant EQ on non-integer.");
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == Variable_arrayGet(tmp2, stack, 0))
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case And:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp1->type->desc == BOOL && tmp2->type->desc == BOOL, "Cant AND on non-bool.");
                tmp = false;
                if(Variable_arrayGet(tmp1, stack, 0) == true && Variable_arrayGet(tmp2, stack, 0) == true)
                    tmp = true;
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case Not:
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Pascal_semantic(tmp1->type->desc == BOOL, "Cant NOT on non-bool.");
                tmp = !Variable_arrayGet(tmp1, stack, 0);
                tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
                Variable_arraySet(tmp1, stack, 0, tmp);
                tmp1->refs = 0;
                return tmp1;
            case NewAr:
                tmp3 = (struct Type*)ast->left->value;
                Pascal_semantic(tmp3->desc == ARRAY, "Wrong type.\n");
                tmp = Variable_arrayGet(Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local), stack, 0);
                Pascal_semantic(tmp > 0, "array size must be > 0.\n");
                tmp1 = Variable_arrayInit(stack, tmp3, tmp);
                tmp1->refs = 0;
                return tmp1;
            case GetARR:
                // Tableau
                tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, ast->left, local);
                Pascal_semantic(tmp1->type->desc == ARRAY, "Cant access index on non-array.");
                // Index
                tmp2 = Pascal_semanticAnalyseEval(stack, env, functions, ast->right, local);
                Pascal_semantic(tmp2->type->desc == INT, "Index must be numeric.");
                tmp = Variable_arrayGet(tmp2, stack, 0);
                // Valeur Tableau[index]
                Pascal_semantic(tmp < stack->size[tmp1->value],"Index out of range.");
                tmp = Variable_arrayGet(tmp1, stack, tmp);
                if(tmp1->type->child->desc != ARRAY)
                {
                    tmp2 = Variable_arrayInit(stack, tmp1->type->child, 1);
                    Variable_arraySet(tmp2, stack, 0, tmp);
                    tmp2->refs = 0;
                    return tmp2;
                }
                else {
                    tmp2 = Variable_arrayRef(tmp1->type->child, tmp);
                    tmp2->refs = 0;
                    return tmp2;
                }
            case CallFUNC:
                tmp4 = FuncList_search(functions, ((char*)ast->left->value));
                Pascal_semantic(tmp4 != 0, "Undeclared func/proc.");
                // création de l'environnement local
                // Ajout des variables arguments
                tmp5 = Env_concat(tmp4->disclaimer->args, tmp4->vars);
                tmp6 = ast->right;
                // Copie des valeurs des arguments dans le nouvel environnement
                for(int i = 0; i < tmp4->disclaimer->args->length; i++)
                {
                    Pascal_semantic(tmp6->right != 0, "Too few arguments.");
                    tmp1 = Pascal_semanticAnalyseEval(stack, env, functions, tmp6->right, local);
                    Pascal_semantic(Type_check(tmp1->type, Env_get_value_index(tmp4->disclaimer->args, i)->type) != false, "Type mismatch.");
                    Env_set_value_index(tmp5, i, tmp1);
                    tmp6 = tmp6->left;
                }
                Pascal_semantic(tmp6 == 0, "Too many arguments.");
                // Ajout d'une variable portant le nom de la fonction faisant office de return
                Env_add_value(tmp5, (char*)ast->left->value, Variable_arrayInit(stack, tmp4->disclaimer->type, 1));
                // execution de la fonction
                //printf("Calling... %p\n", tmp4->ast);
                //Pascal_semanticAnalyseEval(stack, env, functions, tmp4->ast, tmp5);
                //printf("Done\n");
                // Récupération de la variable faisant office de return
                tmp1 = Env_get_value(tmp5, (char*)ast->left->value);
                //printf("Returning value %d\n", Variable_arrayGet(tmp1, stack, 0));
                free(tmp5);
                tmp1->refs = 0;
                return tmp1;



        }
    }
    else if(nodeType == 'I') // constantes
    {
        int tmp = *(int*)ast->value;
        struct Variable* tmp1 = Variable_arrayInit(stack, Type_INT,1);
        Variable_arraySet(tmp1, stack, 0, tmp);
        tmp1->refs = 0;
        return tmp1;
    }
    else if(nodeType == 'B') // constantes
    {
        int tmp = *(int*)ast->value;
        struct Variable* tmp1 = Variable_arrayInit(stack, Type_BOOL,1);
        Variable_arraySet(tmp1, stack, 0, tmp);
        tmp1->refs = 0;
        return tmp1;
    }
    else if(nodeType == 'V')
    {
        struct Variable* res = 0;
        if(local != 0)
            res = Env_get_value(local, (char*)ast->value);
        if(res == 0)
            res = Env_get_value(env, (char*)ast->value);
        char* msg = malloc(0);
        sprintf(msg,  "Var %s not declared", (char*)ast->value);
        Pascal_semantic(res != 0, msg);
        return res;
    }
}

