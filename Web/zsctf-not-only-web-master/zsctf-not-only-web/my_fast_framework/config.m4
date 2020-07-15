dnl $Id$
dnl config.m4 for extension my_fast_framework

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(my_fast_framework, for my_fast_framework support,
dnl Make sure that the comment is aligned:
dnl [  --with-my_fast_framework             Include my_fast_framework support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(my_fast_framework, whether to enable my_fast_framework support,
Make sure that the comment is aligned:
[  --enable-my_fast_framework           Enable my_fast_framework support])

if test "$PHP_MY_FAST_FRAMEWORK" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-my_fast_framework -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/my_fast_framework.h"  # you most likely want to change this
  dnl if test -r $PHP_MY_FAST_FRAMEWORK/$SEARCH_FOR; then # path given as parameter
  dnl   MY_FAST_FRAMEWORK_DIR=$PHP_MY_FAST_FRAMEWORK
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for my_fast_framework files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       MY_FAST_FRAMEWORK_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$MY_FAST_FRAMEWORK_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the my_fast_framework distribution])
  dnl fi

  dnl # --with-my_fast_framework -> add include path
  dnl PHP_ADD_INCLUDE($MY_FAST_FRAMEWORK_DIR/include)

  dnl # --with-my_fast_framework -> check for lib and symbol presence
  dnl LIBNAME=my_fast_framework # you may want to change this
  dnl LIBSYMBOL=my_fast_framework # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MY_FAST_FRAMEWORK_DIR/$PHP_LIBDIR, MY_FAST_FRAMEWORK_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_MY_FAST_FRAMEWORKLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong my_fast_framework lib version or lib not found])
  dnl ],[
  dnl   -L$MY_FAST_FRAMEWORK_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(MY_FAST_FRAMEWORK_SHARED_LIBADD)

  PHP_NEW_EXTENSION(my_fast_framework, my_fast_framework.c functions.c, $ext_shared)
fi
