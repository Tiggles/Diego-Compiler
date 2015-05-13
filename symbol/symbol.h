#ifndef __SYMBOL_H
#define __SYMBOL_H
#define HASH_SIZE 317
#include "../parserscanner/memory.h"


typedef enum TYPES_SUPPORTED {
    SYMBOL_FUNCTION, 
    SYMBOL_INT, 
    SYMBOL_BOOL, 
    SYMBOL_ID, 
    SYMBOL_RECORD, 
    SYMBOL_ARRAY, 
    SYMBOL_NULL, 
    SYMBOL_UNKNOWN 
} TYPES_SUPPORTED;

typedef struct SYMBOLTYPE {
  int visited;
  struct SYMBOLTYPE *nextArrayType; // for arrays
  struct SYMBOLTYPE *return_type; // for functions
  struct SYMBOLTABLE *child;
  TYPES_SUPPORTED type;
  struct TYPE *array;
  struct TYPE *declaration_type;
  struct FUNCTION *func;
  struct VAR_DECL_LIST *parameters;
  int arguments;
}SYMBOLTYPE;


typedef struct SYMBOL {
  char *name;
  int value;
  int noArguments;
  SYMBOLTYPE *symboltype;
  struct SYMBOL *next;
  struct PAR_DECL_LIST *parameters;
  struct FUNCTION *func;
  struct TYPE *declarationtype;
  struct TYPE *returntype;
  struct TYPE *array;
  struct TYPE *realtype;
  char *uniquename;
  int offset;
  int arraySize;
  int tableid;
} SYMBOL;

typedef struct SYMBOLTABLE {
    SYMBOL *table[HASH_SIZE];
    struct SYMBOLTABLE *next;
    int temps;
    int localVars;
    int id;
} SYMBOLTABLE;

int Hash(char *str);

SYMBOLTABLE *initSymbolTable(int id);

SYMBOLTABLE *scopeSymbolTable(SYMBOLTABLE *t, int parentid);

SYMBOL *putSymbol(SYMBOLTABLE *t, char *name, int value, SYMBOLTYPE *symbolT);

SYMBOL *getSymbol(SYMBOLTABLE *t, char *name);

void dumpSymbolTable(SYMBOLTABLE *t);

void dumpTable(SYMBOLTABLE *t);

void destroySymbolTable(SYMBOLTABLE *t);

void dumpSymbol(SYMBOL *symbol);
#endif /* END OF __SYMBOL_H */
