#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_my_fast_framework.h"
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

char *get_action(zval *);
void perform_action(zval *, char *, unsigned int);
char *get_document_root();
void debug_print_handler(zval *);
int walu_call_user_function(zval**, zval *, char *, char *, ...);
char *get_ext_path();

#endif
