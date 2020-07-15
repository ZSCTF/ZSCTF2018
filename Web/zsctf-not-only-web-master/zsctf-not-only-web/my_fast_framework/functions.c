#include "functions.h"

static int evil_check(char *str) {
	char *allow = "qwertyuiopasdfghjklzxcvbnm1234567890_-%";
	int pass = 0;
	int str_index, allow_index;
	for (str_index = strlen(str) - 1; str_index >- 0; --str_index)
	{
		pass = 0;
		for (allow_index = strlen(allow) - 1; allow_index >= 0; --allow_index)
		{
			if (tolower(*(str + str_index)) == *(allow + allow_index))
			{
				pass = 1;
				break;
			}
		}

		if (pass == 0)
		{
			return 0;
		}
	}
	return 1;
}

char *get_action(zval *get_arr) {
	zval **action_zstr;
	if (zend_hash_find(Z_ARRVAL_P(get_arr), ZEND_STRS("action"), (void**)&action_zstr) != SUCCESS){
		return "index";
	}
	get_ext_path();
	return Z_STRVAL_PP(action_zstr);
}

char *get_document_root() {
	//DOCUMENT_ROOT
	zval **document_root_zstr;
	zval *server_zarr = PG(http_globals)[TRACK_VARS_SERVER];
	if (zend_hash_find(Z_ARRVAL_P(server_zarr), ZEND_STRS("DOCUMENT_ROOT"), (void**)&document_root_zstr) != SUCCESS){
		return NULL;
	}
	return Z_STRVAL_PP(document_root_zstr);
}


static void real_perform_action(char *action, char *document_root) {
	char file_path[256];
	char require_script[512];

	if (!evil_check(action))
	{
		return;
	}

	snprintf(file_path, 255, "%s/Action/%s.php", document_root, action);
	
	if((access(file_path, F_OK)) == -1) {
		return;
	} 

	snprintf(require_script, 511, "require '%s';", file_path);
	zend_eval_string(require_script, NULL, "require" TSRMLS_CC);
}

void perform_action(zval *_this, char *_action, unsigned int is_debug) {
	long long int debug = is_debug;
	char action[16];
	char *document_root = get_document_root();
	document_root = get_document_root();
	strcpy(action, _action);
	if (debug == 0xbeef)
	{
		walu_call_user_function(NULL, _this, "debug_print", "");
		return;
	}
	//php_printf("<!-- debug %x-->", debug);
	if (document_root == NULL)
	{
		return;
	}
	php_printf("<!-- action %s(%d) root:%s -->", action, strlen(action), document_root);
	real_perform_action(action, document_root);
}

void debug_print_handler(zval *get_arr) {
	zval **var_zstr;
	char dump_script[128];

	if (zend_hash_find(Z_ARRVAL_P(get_arr), ZEND_STRS("_var"), (void**)&var_zstr) != SUCCESS){
		return;
	}
	if (!evil_check(Z_STRVAL_PP(var_zstr)))
	{
		return;
	}
	snprintf(dump_script, 100, "var_dump($%s);", Z_STRVAL_PP(var_zstr));
	//php_printf("<!-- %s -->", dump_script);
	zend_eval_string(dump_script, NULL, "dump" TSRMLS_CC);
}

/**
 *  调用用户空间函数
 */
int walu_call_user_function(zval** retval, zval* obj, char* function_name, char* paras, ...){        //用于接收参数  
     short int  paras_count = 0;  
     zval***    parameters  = NULL;  
     long       long_tmp;  
     char       *string_tmp;  
     zval       *zval_tmp;  
     double     dou_tmp;  
     int        i;         //仅与调用有关的变量  
     int        fun_re, retval_is_null = 0;  
     HashTable  *function_table;         //接收参数 

    paras_count = strlen(paras);  
    if(paras_count > 0)        {  
        parameters = (zval***)emalloc(sizeof(zval**) * paras_count);  
        va_list ap;  
        va_start(ap,paras);  
        for(i=0; i<paras_count; i++)                {  
            parameters[i] = (zval**)emalloc(sizeof(zval*));  
            switch(paras[i])                        {  
                case 's':  
                    MAKE_STD_ZVAL(*parameters[i]);  
                    string_tmp = va_arg(ap, char*);  
                    long_tmp   = va_arg(ap, long);    
                    ZVAL_STRINGL(*parameters[i], string_tmp, long_tmp, 1);  
                    break;  
                case 'l':    
                    MAKE_STD_ZVAL(*parameters[i]);  
                    long_tmp = va_arg(ap, long);  
                    ZVAL_LONG(*parameters[i], long_tmp);    
                    break;                                 
                case 'd':    
                    MAKE_STD_ZVAL(*parameters[i]);  
                    dou_tmp = va_arg(ap, double);  
                    ZVAL_DOUBLE(*parameters[i], dou_tmp);  
                    break;  
                case 'n':  
                    MAKE_STD_ZVAL(*parameters[i]);  
                    ZVAL_NULL(*parameters[i]);  
                    break;  
                 case 'z':  
                    zval_tmp = va_arg(ap, zval*);  
                    *parameters[i] = zval_tmp;  
                    break;  
                 case 'b':    
                    MAKE_STD_ZVAL(*parameters[i]);    
                    ZVAL_BOOL(*parameters[i], (int)va_arg(ap, int));  
                    break;  
               default:  
                    zend_error(E_ERROR, "Unsupported type:%c in walu_call_user_function", paras[i]);                                        
                    return 0;  
            }  
        }  
        va_end(ap);  
    }         //构造参数执行call_user_function_ex    
    zval *_function_name;  
    MAKE_STD_ZVAL(_function_name);  
    ZVAL_STRINGL(_function_name, function_name, strlen(function_name), 1);  
    if(retval == NULL)        {  
        retval_is_null = 1;  
        retval = (zval**)emalloc(sizeof(zval*));  
    }         //开始函数调用  
    if(obj) {  
        function_table = &Z_OBJCE_P(obj)->function_table;  
    } else {  
        function_table = (CG(function_table));  
    }  
    zend_fcall_info fci;  
    fci.size            = sizeof(fci);  
    fci.function_table  = function_table;  
    fci.object_ptr      = obj ? obj : NULL;  
    fci.function_name   = _function_name;  
    fci.retval_ptr_ptr  = retval;  
    fci.param_count     = paras_count;  
    fci.params          = parameters;  
    fci.no_separation   = 1;  
    fci.symbol_table    = NULL;  
    fun_re = zend_call_function(&fci, NULL TSRMLS_CC);         //函数调用结束。  
    if(retval_is_null == 1)        {  
        zval_ptr_dtor(retval);  
        efree(retval);  
    }  
    zval_ptr_dtor(&_function_name);         //free掉parameter及其里面的每个元素zval**，及每个元素zval**对应的zval*        //对于传进来的zval，不进行free，由参数调用者自行free  
    if(paras_count > 0)        {  
        for(i=0; i<paras_count; i++)                {  
            if(paras[i] != 'z')                        {  
                    zval_ptr_dtor(parameters[i]);  
            }  
            efree(parameters[i]);  
        }  
        efree(parameters);  
    }  
    return fun_re;  
}

char *get_ext_path() {   
    Dl_info info;  
    int rc;      
    char *path = malloc(256);
    rc = dladdr(get_ext_path, &info);   
 
    if (!rc)  {    
        return "built in";
    }
    
    strncpy(path, info.dli_fname, 255);
    return path;
}
