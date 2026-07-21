#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "std_lib.h"
#ifdef STD_DLL_LINK
   #include <windows.h>
#endif

#ifdef BORLAND_C45
   #define utoa( x , y, z ) ultoa( x, y, z )
#endif

#ifndef STD_DLL_LINK

   #ifdef MEX_COMPILE
      #define dprintf(x) mexPrintf(x)
      #define dputs(x) mexPrintf(x)
      #define dputchar(x) mexPrintf("%c",(char)x)
      #include "mex.h"
   #endif


	#ifndef CYGWIN_GCC
		#include <windows.h>
	#endif

   #ifdef REX_COMPILE
      #include <signal.h>
      #ifndef CYGWIN_GCC
      	#include <conio.h>
      #endif
      #include <time.h>
      #include <sys/types.h>
      #include <sys/irqinfo.h>
      #include <sys/proxy.h>
      #include <sys/seginfo.h>
      #include <sys/kernel.h>
      #include "../../hdr/sys.h"
   #endif
#endif

#ifdef STD_DLL_LINK
static STD_P std_lib_globals;
static STD_P *stdp;

#ifdef __WIN32__
void DLLEXPORT get_std_lib_p(STD_P **std_p){
#else
void DLLEXPORT get_std_lib_p(STD_P **std_p){
#endif
*std_p=&std_lib_globals;
stdp=&std_lib_globals;
return;
};
#endif

long std_alloccnt=(long)0;

#if defined( PRINTF_EXTERN ) || defined( STD_DLL_LINK )

/*
 * Scaled down printf for dll.  Recognizes following formats:
 *      %s- string          %c- char
 *      %d- decimal         %o- octal       %u- unsigned        %x- hex
 *      %D- long decimal    %O- long octal  %U- long unsigned   %X- long hex
 */

#ifdef STD_DLL_LINK
int std_printf(char *fmt, ...)
#else
int std_printf(char FAR___ *fmt, ...)
#endif
{
va_list ap;
#define DPF_SSIZ 100
   char str[DPF_SSIZ];
   long lval;
   short int ival,radix;
   unsigned long luval ;
   unsigned short uval;
   char cval;
   char *sval;
   char hstr[2];
   double dval;
   long double ldval;
   float fval;

   va_start(ap, fmt);
   for(;;) {
      while((cval= *fmt++) != '%') {
         if(cval == '\0') {
             goto done;
         }
         d_putchar(cval);
      }

      /*
       * Format specifier.
       */
      switch(*fmt) {
      case 'l':  /** could be lX, LO, lD !! */
         if ((*(fmt+1)=='X') || (*(fmt+1)=='x') ||
             (*(fmt+1)=='O') || (*(fmt+1)=='o') ||
             (*(fmt+1)=='D') || (*(fmt+1)=='d')){
             fmt++;
             goto long_vals_lable;
         }
         else{
            d_puts("Bad format spec. in dprintf: ");
            hstr[1]=(char)0;
            hstr[0]=*fmt;
            d_puts(hstr);
            d_puts("\n");
         };
      break;
      case 'f':
            fval=va_arg(ap,float);
            dval=fval;
            goto std_double_conv;
      break;
      case 'L':
          if(*(fmt+1)=='f'){
            fmt++;
            ldval=va_arg(ap,long double);
std_double_conv:
            if(ldval<0.0){
               std_printf("%s","-");
               dval=-dval;
            };
            lval=floor((double)ldval);
            std_printf("%D",lval);
            std_printf(".");
            ldval=(ldval-lval)*10000.0;
            lval=floor(ldval);
            std_printf("%D",lval);
          };
      break;
      case 'd':
      case 'o':
      case 'x':
         switch(*fmt){
            case 'd':
               radix=10;
               ival= va_arg(ap, short int);
               /*itoa(ival, str,radix);*/
               sprintf(str,"%d",ival);
            break;
            case 'x':
               radix=16;
               uval= va_arg(ap, unsigned short);
               utoa(uval, str,radix);
            break;
            case 'o':
               radix=8;
               uval= va_arg(ap, unsigned short);
               utoa(uval, str,radix);
            break;
         };
         /*
          * Must call version that takes far pointers
          * when called from interrupt level
          */
         d_puts(str);
         break;
      case 'u':
      case 'U':
         radix=10;
         switch(*fmt){
            case 'U':
               luval= va_arg(ap, unsigned long);
                 ultoa(luval, str,radix);
            break;
            case 'u':
               uval= va_arg(ap, unsigned short);
                 utoa(uval, str,radix);
            break;
         };
         d_puts(str);
         break;
      case 'D':
      case 'O':
      case 'X':
long_vals_lable:
         lval= va_arg(ap, long);

         /*
          * Must call version that takes far pointers
          * when called from interrupt level.
          */
         switch(*fmt){
            case 'D':radix=10; break;
            case 'X':radix=16; break;
            case 'O':radix=8; break;
         };
         ltoa(lval, str, radix);
         d_puts(str);
         break;
      case 's':
         sval= va_arg(ap, char *);
         d_puts(sval);
         break;
      case 'c':
         cval= va_arg(ap, char);
         d_putchar(cval);
         break;
      default:
         d_puts("Bad format spec. in dprintf: ");
         hstr[1]=(char)0;
         hstr[0]=*fmt;
         d_puts(hstr);
         d_puts("\n");
      }
      fmt++;
   }
done:
   va_end(ap);
 return(0);
};

#else
   int (*std_printf)(char *fmt, ...)=&printf;
#endif
/************  hook functions *******************/
#ifndef STD_DLL_LINK
int ___pascal dputchar_(int c){
   printf("%c",(char)c);
   return(0);
};
#endif

int ___pascal d_putchar(int c){
#ifdef STD_DLL_LINK
   stdp->dputchar_(c);
#else
   #ifdef PRINTF_EXTERN
      dputchar(c);
   #else
      dputchar_(c);
   #endif
#endif
   return(0);
};

#ifndef STD_DLL_LINK
int ___pascal dputs_(const char *c){
   printf("%s",c);
   return(0);
};
#endif

int ___pascal d_puts(const char *c){
#ifdef STD_DLL_LINK
   return(stdp->dputs_(c));
#else
   #ifdef PRINTF_EXTERN
      dputs(c);
      return(0);
   #else
      return(dputs_(c));
   #endif
#endif
};

#ifndef STD_DLL_LINK
void ___pascal f_mul(void *prod,void *f1,void *f2){
         *(float *)prod=(*(float *)f1)*(*(float *)f2);
         return;
};
#endif

void ___pascal float_mul(void *prod,void *f1,void *f2){
#ifdef STD_DLL_LINK
   stdp->f_mul(prod,f1,f2);
#else
   f_mul(prod,f1,f2);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_div(void *quot,void *f1,void *f2){
         *(float *)quot=(*(float *)f1)/(*(float *)f2);
         return;
};
#endif

void ___pascal float_div(void *quot,void *f1,void *f2){
#ifdef STD_DLL_LINK
   stdp->f_div(quot,f1,f2);
#else
   f_div(quot,f1,f2);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_add(void *summ,void *s1,void *s2){
         *(float *)summ=(*(float *)s1)+(*(float *)s2);
         return;
};
#endif

void ___pascal float_add(void *summ,void *s1,void *s2){
#ifdef STD_DLL_LINK
   stdp->f_add(summ,s1,s2);
#else
   f_add(summ,s1,s2);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_sub(void *diff,void *s1,void *s2){
         *(float *)diff=(*(float *)s1)-(*(float *)s2);
         return;
};
#endif

void ___pascal float_sub(void *diff,void *s1,void *s2){
#ifdef STD_DLL_LINK
   stdp->f_sub(diff,s1,s2);
#else
   f_sub(diff,s1,s2);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_val(void *val,char *s){
double x;
         x=atof(s);
         *(float *)val=(float)x;
         return;
};
#endif

void ___pascal float_val(void *val,char *s){
#ifdef STD_DLL_LINK
   stdp->f_val(val,s);
#else
   f_val(val,s);
#endif
   return;
};


#ifndef STD_DLL_LINK
void ___pascal l_val(void *val,char *s){
double x;
         x=atof(s);
         *(long *)val=(long)x;
         return;
};
#endif

void ___pascal long_val(void *val,char *s){
#ifdef STD_DLL_LINK
   stdp->l_val(val,s);
#else
   l_val(val,s);
#endif
   return;
};

#ifndef STD_DLL_LINK
int  ___pascal f_cmp(void *val){
int i;
         if((float)0.0==(*(float *)val))i=0;
         else if((float)0>(*(float *)val))i=-1;
         else i=1;
         return(i);
};
#endif

int  ___pascal float_cmp(void *val){
int i;
#ifdef STD_DLL_LINK
   i=stdp->f_cmp(val);
#else
   i=f_cmp(val);
#endif
   return(i);
};

#ifndef STD_DLL_LINK
#ifdef __WIN32__
void * ___pascal p_malloc(int size){
void *p;
#else
void FAR___ * ___pascal p_malloc(int size){
void FAR___ *p;
#endif
/* std_printf("A3 size=%d ",size); */
         p=malloc(size);
/* std_printf("A4\n"); */
         std_alloccnt++;
         return(p);
};
#endif

void FAR___ * ___pascal pasc_malloc(int size){
void FAR___ *p;
#ifdef STD_DLL_LINK
   p=(void *)(stdp->pasc_malloc(size));
#else
   p=p_malloc(size);
#endif
   return(p);
};

#ifndef STD_DLL_LINK
#ifdef __WIN32__
void ___pascal p_free(void *p,int size){
#else
void ___pascal p_free(void FAR___ *p,int size){
#endif
         size=(int)0;
         free(p);
         std_alloccnt--;
         return;
};
#endif

#ifdef __WIN32__
void ___pascal pasc_free(void *p,int size){
#else
void ___pascal pasc_free(void FAR___ *p,int size){
#endif

#ifdef STD_DLL_LINK
   stdp->pasc_free(p,size);
#else
   p_free(p,size);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_to_i(void *intval,void *floatval){
         *(short int *)intval=(short int)(*(float *)floatval);
         return;
};
#endif

void ___pascal float_to_int(void *intval,void *floatval){
#ifdef STD_DLL_LINK
   stdp->f_to_i(intval,floatval);
#else
   f_to_i(intval,floatval);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_floor(void *intval,void *floatval){
         *(short int *)intval=(short int)floor(*(float *)floatval);
         return;
};
#endif

void ___pascal float_floor(void *intval,void *floatval){
#ifdef STD_DLL_LINK
   stdp->f_floor(intval,floatval);
#else
   f_floor(intval,floatval);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal f_to_l(void *longval,void *floatval){
         *(long *)longval=(long)(*(float *)floatval);
         return;
};
#endif

void ___pascal float_to_long(void *longval,void *floatval){
#ifdef STD_DLL_LINK
   stdp->f_to_l(longval,floatval);
#else
   f_to_l(longval,floatval);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal i_to_f(void *floatval,void *intval){
         *(float *)floatval=(float)(*(short int *)intval);
         return;
};
#endif

void ___pascal int_to_float(void *floatval,void *intval){
#ifdef STD_DLL_LINK
   stdp->i_to_f(floatval,intval);
#else
   i_to_f(floatval,intval);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal l_to_f(void *floatval,void *longval){
         *(float *)floatval=(float)(*(long *)longval);
         return;
};
#endif

void ___pascal long_to_float(void *floatval,void *longval){
#ifdef STD_DLL_LINK
   stdp->l_to_f(floatval,longval);
#else
   l_to_f(floatval,longval);
#endif
   return;
};

int ___pascal float_bigger(void *f,void *j){
long z;
int i;
#ifdef STD_DLL_LINK
   stdp->f_sub(&z,f,j);
   i=stdp->f_cmp(&z);
#else
   f_sub(&z,f,j);
   i=f_cmp(&z);
#endif
   if(i>(int)0)return(1);
   else return(0);
};

int ___pascal float_equal(void *i,void *j){
long z;
int k;
#ifdef STD_DLL_LINK
   stdp->f_sub(&z,i,j);
   k=stdp->f_cmp(&z);
#else
   f_sub(&z,i,j);
   k=f_cmp(&z);
#endif
   if(k==(int)0)return(1);
   else return(0);
};

#ifndef STD_DLL_LINK
void ___pascal f_to_str(char *s,void *floatval,int li,int fi){
char fstr[20];
         sprintf(fstr,"%%%d.%df",li,fi);
         sprintf(s,fstr,*(float *)floatval);
};
#endif

void ___pascal float_sprintf(char *s,void *floatval,int li,int fi){
#ifdef STD_DLL_LINK
   stdp->f_to_str(s,floatval,li,fi);
#else
   f_to_str(s,floatval,li,fi);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal l_to_str(char *s,void *longval,int li,int fi){
char fstr[20];
int ll;
         if(fi>(int)0)
                  ll=li-(int)2;
         else
                  ll=li;
         if(ll<0)
                  ll=(int)0;
         sprintf(fstr,"%%%dld",ll);
         sprintf(s,fstr,*(long *)longval);
         if (fi>(int)0){
                  ll=strlen(s);
                  strcat(s,".0");
                  fi--;
                  while(fi>0){
      strcat(s,"0");
      fi--;
                  };
         };
         return;
};
#endif


void ___pascal long_sprintf(char *s,void *longval,int li,int fi){
#ifdef STD_DLL_LINK
   stdp->l_to_str(s,longval,li,fi);
#else
   l_to_str(s,longval,li,fi);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal str_to_f(void *f,void *s){
double x;
         x=atof(s);
         *((float *)f)=(float)x;
         return;
};
#endif


void ___pascal string_to_float(void *f,void *s){
#ifdef STD_DLL_LINK
   stdp->str_to_f(f,s);
#else
   str_to_f(f,s);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal atan2_(void *res,void *real,void *imag){
         *((float *)res)=(float)atan2(
                           (double)  (*((float *)imag)),
                           (double)  (*((float *)real))
                                                                                                                                                                                                                                                                );
         return;
};
#endif


void ___pascal itp_atan2(void *res,void *real,void *imag){
#ifdef STD_DLL_LINK
   stdp->atan2(res,real,imag);
#else
   atan2_(res,real,imag);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal sinus(void *res,void *x){
         *((float *)res)=(float)sin(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif


void ___pascal itp_sinus(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->sinus(res,x);
#else
   sinus(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal cosinus(void *res,void *x){
         *((float *)res)=(float)cos(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif

void ___pascal itp_cosinus(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->cosinus(res,x);
#else
   cosinus(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal arcsin(void *res,void *x){
         *((float *)res)=(float)asin(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif

void ___pascal itp_arcsinus(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->arcsin(res,x);
#else
   arcsin(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal arccos(void *res,void *x){
         *((float *)res)=(float)acos(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif

void ___pascal itp_arccosinus(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->arccos(res,x);
#else
   arccos(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal exp_(void *res,void *x){
         *((float *)res)=(float)exp(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif

void ___pascal itp_exp(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->exp(res,x);
#else
   exp_(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
void ___pascal ln(void *res,void *x){
         *((float *)res)=(float)log(
                           (double)  (*((float *)x))
                                                                                                                                                                                                                                                                );
         return;
};
#endif

void ___pascal itp_ln(void *res,void *x){
#ifdef STD_DLL_LINK
   stdp->ln(res,x);
#else
   ln(res,x);
#endif
   return;
};

#ifndef STD_DLL_LINK
#ifdef DLL_USER
/** routine to initialize hook functions **/
void set_stdlib(STD_P *stdp){
   stdp->dputchar_=d_putchar;
   stdp->dputs_=d_puts;
   stdp->pasc_malloc=p_malloc;
   stdp->pasc_free=p_free;
   stdp->f_mul=f_mul;
   stdp->f_div=f_div;
   stdp->f_add=f_add;
   stdp->f_sub=f_sub;
   stdp->f_val=f_val;
   stdp->l_val=l_val;
   stdp->f_cmp=f_cmp;
   stdp->f_to_i=f_to_i;
   stdp->f_floor=f_floor;
   stdp->f_to_l=f_to_l;
   stdp->i_to_f=i_to_f;
   stdp->l_to_f=l_to_f;
   stdp->f_to_str=f_to_str;
   stdp->l_to_str=l_to_str;
   stdp->str_to_f=str_to_f;
   stdp->atan2=atan2_;
   stdp->sinus=sinus;
   stdp->cosinus=cosinus;
   stdp->arcsin=arcsin;
   stdp->arccos=arccos;
   stdp->exp=exp_;
   stdp->ln=ln;
   return;
};
#endif
#endif

