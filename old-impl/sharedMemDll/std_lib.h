#include "compile.h"

/* this module is used to replace std_lib0.h if code is linked as dll   *
 * because some elementary library functions have to be replaced by the *
 * user.                                                                */

#ifdef MEX_COMPILE
   #define PRINTF_EXTERN
#endif

#ifdef REX_COMPILE
   #define PRINTF_EXTERN
#endif

typedef struct STD_P_s{
   int ___pascal (*dputchar_)(int c);
   int ___pascal (*dputs_)(const char *c);
#ifdef __WIN32__
   void * ___pascal (*pasc_malloc)(int size);
#else
   void FAR___ * ___pascal (*pasc_malloc)(int size);
#endif
#ifdef __WIN32__
   void ___pascal (*pasc_free)(void *p,int size);
#else
   void ___pascal (*pasc_free)(void FAR___ *p,int size);
#endif
   void ___pascal (*f_mul)(void *prod,void *f1,void *f2);
   void ___pascal (*f_div)(void *quot,void *f1,void *f2);
   void ___pascal (*f_add)(void *summ,void *s1,void *s2);
   void ___pascal (*f_sub)(void *diff,void *s1,void *s2);
   void ___pascal (*f_val)(void *val,char *s);
   void ___pascal (*l_val)(void *val,char *s);
   int ___pascal (*f_cmp)(void *val);
   void ___pascal (*f_to_i)(void *intval,void *floatval);
   void ___pascal (*f_floor)(void *intval,void *floatval);
   void ___pascal (*f_to_l)(void *longval,void *floatval);
   void ___pascal (*i_to_f)(void *floatval,void *intval);
   void ___pascal (*l_to_f)(void *floatval,void *intval);
   void ___pascal (*f_to_str)(char *str,void *floatval,int li,int fi);
   void ___pascal (*l_to_str)(char *str,void *longval,int li,int fi);
   void ___pascal (*str_to_f)(void *f,void *s);
   void ___pascal (*atan2)(void *res,void *real,void *imag);
   void ___pascal (*sinus)(void *res,void *x);
   void ___pascal (*cosinus)(void *res,void *x);
   void ___pascal (*arcsin)(void *res,void *x);
   void ___pascal (*arccos)(void *res,void *x);
   void ___pascal (*exp)(void *res,void *x);
   void ___pascal (*ln)(void *res,void *x);
   }STD_P;


#ifndef STD_DLL_LINK
#ifdef STD_DLL_USER
void set_stdlib(STD_P *stdp);
void FAR___ _export get_std_lib_p(STD_P **stdp);
/** copy this to IMPORTS section of dll-user *
GET_STD_LIB_p=STATDLL.GET_STD_LIB
 *********************************************/
#endif
#endif

#ifdef STD_DLL_LINK
   int std_printf(char *fmt, ...);
#else
   #ifdef PRINTF_EXTERN
      int std_printf(char FAR___ *fmt, ...);
   #else
      extern int (*std_printf)(char *fmt, ...);
   #endif
#endif
int ___pascal d_putchar(int c);
int ___pascal d_puts(const char *c);
void ___pascal float_mul(void *prod,void *f1,void *f2);
void ___pascal float_div(void *quot,void *f1,void *f2);
void ___pascal float_add(void *summ,void *s1,void *s2);
void ___pascal float_sub(void *diff,void *s1,void *s2);
void ___pascal float_val(void *val,char *s);
void ___pascal long_val(void *val,char *s);
int  ___pascal float_cmp(void *val);
void FAR___ * ___pascal pasc_malloc(int size);
#ifdef __WIN32__
void ___pascal pasc_free(void *p,int size);
#else
void ___pascal pasc_free(void FAR___ *p,int size);
#endif
void ___pascal float_to_int(void *intval,void *floatval);
void ___pascal float_floor(void *intval,void *floatval);
void ___pascal float_to_long(void *longval,void *floatval);
void ___pascal int_to_float(void *floatval,void *intval);
void ___pascal long_to_float(void *floatval,void *longval);
int ___pascal float_bigger(void *f,void *j);
int ___pascal float_equal(void *i,void *j);
void ___pascal float_sprintf(char *s,void *floatval,int li,int fi);
void ___pascal long_sprintf(char *s,void *longval,int li,int fi);
void ___pascal string_to_float(void *f,void *s);
void ___pascal itp_atan2(void *res,void *real,void *imag);
void ___pascal itp_sinus(void *res,void *x);
void ___pascal itp_cosinus(void *res,void *x);
void ___pascal itp_arcsinus(void *res,void *x);
void ___pascal itp_arccosinus(void *res,void *x);
void ___pascal itp_exp(void *res,void *x);
void ___pascal itp_ln(void *res,void *x);

extern long std_alloccnt;
