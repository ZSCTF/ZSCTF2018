/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_my_fast_framework.h"
#include "functions.h"

#include <time.h>

/* If you declare any globals in php_my_fast_framework.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(my_fast_framework)
*/

/* True global resources - no need for thread safety here */
static int le_my_fast_framework;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("my_fast_framework.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_my_fast_framework_globals, my_fast_framework_globals)
    STD_PHP_INI_ENTRY("my_fast_framework.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_my_fast_framework_globals, my_fast_framework_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_my_fast_framework_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_my_fast_framework_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "my_fast_framework", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_my_fast_framework_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_my_fast_framework_init_globals(zend_my_fast_framework_globals *my_fast_framework_globals)
{
	my_fast_framework_globals->global_value = 0;
	my_fast_framework_globals->global_string = NULL;
}
*/
/* }}} */

const zend_function_entry mff_functions[] = {
	PHP_ME(MFF, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(MFF, __destruct,  NULL, ZEND_ACC_PUBLIC|ZEND_ACC_DTOR)
    PHP_ME(MFF, run,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(MFF, debug_print, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

PHP_METHOD(MFF, __construct)
{
	zval *obj = getThis();
	long start_time = clock();

	zend_update_property_long(mff_ce, obj, ZEND_STRL("_start_time"), start_time);

}

PHP_METHOD(MFF, __destruct)
{
	zval *obj = getThis();
	long start_time = Z_LVAL_P(zend_read_property(mff_ce, obj, ZEND_STRL("_start_time"), 1 TSRMLS_CC));
	long delta = clock() - start_time;

	php_printf("<!-- generate by MFF %ldms -->", delta);
}

PHP_METHOD(MFF, run)
{
	zval *get_zarr = PG(http_globals)[TRACK_VARS_GET];
	char *action = get_action(get_zarr);
	perform_action(getThis(), action, 0);
}

PHP_METHOD(MFF, debug_print)
{
	php_printf("<!-- debug_print -->");
	zval *get_zarr = PG(http_globals)[TRACK_VARS_GET];
	debug_print_handler(get_zarr);
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(my_fast_framework)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	zend_class_entry mff;
	INIT_CLASS_ENTRY(mff, "MFF", mff_functions);
	mff_ce = zend_register_internal_class(&mff TSRMLS_CC);
	zend_declare_property_long(mff_ce,ZEND_STRL("_start_time"), 0,ZEND_ACC_PRIVATE TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(my_fast_framework)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(my_fast_framework)
{
	zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(my_fast_framework)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(my_fast_framework)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "Section", "Content");
	php_info_print_table_row(2, "extensions", get_ext_path());
	php_info_print_table_row(2, "my_fast_framework support", "enabled");
	php_info_print_table_row(2, "Class", "MFF");
	php_info_print_table_end();
	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ my_fast_framework_functions[]
 *
 * Every user visible function must have an entry in my_fast_framework_functions[].
 */
const zend_function_entry my_fast_framework_functions[] = {
	PHP_FE(confirm_my_fast_framework_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in my_fast_framework_functions[] */
};
/* }}} */

/* {{{ my_fast_framework_module_entry
 */
zend_module_entry my_fast_framework_module_entry = {
	STANDARD_MODULE_HEADER,
	"my_fast_framework",
	my_fast_framework_functions,
	PHP_MINIT(my_fast_framework),
	PHP_MSHUTDOWN(my_fast_framework),
	PHP_RINIT(my_fast_framework),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(my_fast_framework),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(my_fast_framework),
	PHP_MY_FAST_FRAMEWORK_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MY_FAST_FRAMEWORK
ZEND_GET_MODULE(my_fast_framework)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
