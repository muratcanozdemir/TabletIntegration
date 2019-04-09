#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "std_lib.h"
#include "str_lib.h"


static char str_result[255];




char *str_copy_func(char *s,int start,int count){
char *p;
char *p1;
int i,l;


l=strlen(s);
if(start<1){
   count=count+start-1;
   start=1;
};
if(start+count-1>l)
   count=l-start+1;
if(start>l || count<1){
   *str_result=(char)0;
}
else{
   i=1;
   p=&(s[start-1]);
   memmove(str_result,p,count);
   p1=&(str_result[count]);
   *p1=(char)0;
};
return(str_result);
};



int str_back_search(char *ss,char *s){
int sl,ssl,i,cont;
sl=strlen(s);
ssl=strlen(ss);
i=sl;
cont=(i-ssl+1>0);
if(cont==0)
   i=0;
while(cont){
   if(strcmp(str_copy_func(s,i-ssl+1,ssl),ss)==0){
      cont=0;
   }
   else{
      i=i-1;
      cont=(i-ssl+1>0);
      if(cont==0)
         i=0;
   };
};
if(i>0)
   i=i-ssl+1;
return(i);
};


char *fext(char *s){
int l1,l2;
char extstr[255];
char hstr[255];

strcpy(hstr,fnam(s));
l1=strlen(hstr);
l2=strlen(s);
if(l1<l2-1)
   strcpy(str_result,str_copy_func(s,l1+2,l2-l1-1));
else
   strcpy(str_result,"");
return(str_result);
}

char *fnam(char *s){
int l;
l=str_back_search(".",s);
if(l==0){
   strcpy(str_result,s);
}
else{
   if(l>1)
      str_copy_func(s,1,l-1);
   else
      *str_result=(char)0;
};
return(str_result);
};


char *ch_ext(char *s,char *new_ext){
int l1,l2;
char extstr[255];
char hstr[255];

strcpy(hstr,fnam(s));
l1=strlen(hstr);
l2=strlen(s);
if(l1<l2)
   strcpy(extstr,str_copy_func(s,l1+1,l2-l1));
else
   *extstr=(char)0;

strcpy(str_result,hstr);
strcat(str_result,".");
strcat(str_result,new_ext);
return(str_result);
};



int get_number_from_end_of_string(char *s,char *numstr,int *index){
char mn[255];
char nc[255];
int i,l,nnf;
strcpy(mn,s);
rish(mn);
i=0;
nnf=0;
l=strlen(mn);
while (nnf==0){
   strcpy(nc,str_copy_func(mn,l-i,1));
   if(((int)(*nc)<48) | ((int)(*nc)>57))
      nnf=1;
   else if (i>l-2)
      nnf=-1;
   else
      i=i+1;
};

if (nnf==1){ /** nonumber found **/
   if(i>0){
      strcpy(numstr,str_copy_func(mn,strlen(mn)-i+1,i));
      *index=strlen(mn)-i+1;
   }
   else{
      numstr=(char)0;
      *index=0;
   };
}
else if (nnf==-1){
   strcpy(numstr,mn);
   *index=1;
}
else{
   numstr=(char)0;
   *index=0;
};

if(*index==0)
   return(-1);
return(atoi(numstr));
};


char *modify_name(char *hstr){
int l1,l2,index,num;
char hstr1[255];
char extstr[255];
char hstr_re[255];

strcpy(hstr1,fnam(hstr));
l1=strlen(hstr1);
l2=strlen(hstr);
if(l1<l2)
   strcpy(extstr,str_copy_func(hstr,l1+1,l2-l1));
else
   *extstr=(char)0;

num=get_number_from_end_of_string(hstr1,hstr_re,&index);
if(num<0){
	strcpy(hstr_re,hstr1);
   strcat(hstr_re,"0");
}
else{
   strcpy(hstr_re,str_copy_func(hstr1,1,index-1));
	/*itoa(num+1,hstr1,10);*/
	sprintf(hstr1,"%d",num+1);
   strcat(hstr_re,hstr1);
};
strcpy(str_result,hstr_re);
strcat(str_result,extstr);

return(str_result);
};



char *modify_file_name(char *s,int do_print_message,char *default_ext_1,char *default_ext_2){

FILE *fp_test;
FILE *fp_test1;

char filename[255];
char filename1[255];

	strcpy(filename,s);
	lesh(rish(filename));

	if(filename[0]==(char)0)
		strcpy(filename,"dat.");

	strcpy(filename,ch_ext(filename,""));

   fp_test1=(FILE *)1;
   strcpy(filename1,filename);
   while(fp_test1!=NULL){
		strcpy(filename,ch_ext(filename,default_ext_1));
      fp_test=fopen(filename,"r");
	   fp_test1=fp_test;
		if(fp_test!=NULL){     /** File exists **/
			fclose(fp_test);
			strcpy(filename,modify_name(filename));
		}
		else if(default_ext_2[0]!=(char)0){
			strcpy(filename,ch_ext(filename,default_ext_2));
			fp_test=fopen(filename,"r");
			fp_test1=fp_test;

			if(fp_test!=NULL){     /** File exists **/
				fclose(fp_test);
				strcpy(filename,modify_name(filename));
			};
		};
	};

	strcpy(filename,ch_ext(filename,""));
	strcpy(str_result,filename);

	if(strcmp(filename,filename1)!=0 && do_print_message!=0)
		std_printf("File renamed to %s !!\n",filename);
return(str_result);
};

char *modify_aefile_name(char *s,int do_print_message){

return(modify_file_name(s,do_print_message,"A","E"));
}

int c_pos(const char *ss,const char *s){
const char *p,*p1;
int i;
   p=strstr(s,ss);
   i=0;
   p1=s;
   if (p==NULL) return(i);
   i++;
   while(p1!=p){
      i++;
      p1++;
   }
   return(i);
}
/*--------------------------------------------------------*/
void str_copy(char *dest,char *src,int first,int cnt){
char *p1,*p2;
int i,l;
   i=(int)0;
   p1=dest;
   l=(int)strlen(src);
   if(first<(int)1)first=(int)1;
   if (first>l)first=l;
   if(first+cnt-1>l)cnt=l-first+1;
   p2=src+first-1;
   while(i<cnt && *p2!=(char)0){
      *p1=*p2;
      p2=p2+1;
      p1=p1+1;
      i++;
   };
   *p1=(char)0;
   return;
};
/*--------------------------------------------------------*/
char *strstr_n(char *s,char *ss,int n){
int sl;
char c;
char *cp,*crp;
   if(s==NULL || ss==NULL)return(NULL);
   sl=strlen(s);
   if(sl<n)n=sl;
   cp=s+n;
   c=*cp;
   *cp=(char)0;
   crp=strstr(s,ss);
   *cp=c;
   return(crp);
}
/*--------------------------------------------------------*/
/* version of strstr that does not react on double occurance of ss :**/
char *strstr_r(char *s,char *ss){
char *p1,*p2,*p3;
char dbl[50];
int l2;
   strcpy(dbl,ss);
   strcat(dbl,ss);
   l2=2*strlen(ss);
   if(s==NULL)
      p2=NULL;
   else
      p2=strstr(s,dbl);
   if(s==NULL)
      p1=NULL;
   else
      p1=strstr(s,ss);

   if(p2==NULL || (long)p1<(long)p2)
      return(p1);
   p1=NULL;
   while(p2!=NULL && *(p2+l2)!=(char)0){
      p3=p2+l2;
      p2=strstr(p3,dbl);
      p1=strstr(p3,ss);
      if(p2==NULL || (long)p1<(long)p2)
	 return(p1);
      p1=NULL;
   };
   return(p1);
};
/*--------------------------------------------------------*/
/* version of strstr_n that does not react on double occurance of ss */
char *strstr_n_r(char *s,char *ss,int n){
int sl;
char c;
char *cp,*crp;
   if(s==NULL || ss==NULL)return(NULL);
   sl=strlen(s);
   if(sl<n)n=sl;
   cp=s+n;
   c=*cp;
   *cp=(char)0;
   crp=strstr_r(s,ss);
   *cp=c;
   return(crp);
}
/*--------------------------------------------------------*/
/** version of strtok, that does not seperate tokens on double **
 ** occurance of trenn :                                       **/
static char *nexttok=NULL;
char *strtok_r(char *s,char *trenn){
char *p1,*p2;
int l;
   l=strlen(trenn);
   if(s==NULL)
      p1=nexttok;
   else
      p1=s;
   p2=strstr_r(p1,trenn);
   if(p2!=NULL){
      *p2=(char)0;
      nexttok=p2+l;
   }
   else
      nexttok=NULL;
   return(p1);
};
/*--------------------------------------------------------*/
void str_insert(char *is,char *s,int pos){
char *p1,*p2,*p3,*p4;
   if (is==NULL || s==NULL)return;
   if (strlen(is)==0)return;
   if (pos<1)pos=1;
   if (pos>strlen(s))pos=strlen(s)+1;
   p1=is;
   p2=s+pos-1;
   p3=s+strlen(s);
   p4=p3+strlen(is);
   do{
      *p4=*p3;
      p4--;
      p3--;
   }while(p3+1!=p2);
   p3++;
   while((int)(*p1)!=0){
      *p3=*p1;
      p3++;
      p1++;
   };
}
/*--------------------------------------------------------*/
char *lesh(char *s){
char *p1,*p2;
   p1=s;
   p2=s;
   while((int)(*p1)!=0 && *p1==' ')p1++;
   if(p1!=p2){
      do{
	 *p2=*p1;
	 p1++;
	 p2++;
      }while((int)(*(p1-1))!=0);

   };
   return(s);
};
/*--------------------------------------------------------*/
char *rish(char *s){
char *p1;
   if (*s==(char)0)return(s);
   p1=s;
   while(*p1!=(char)0)p1++;
   p1--;
   while(*p1==' '){
      *p1=(char)0;
      p1--;
   };
   return(s);
};
/*--------------------------------------------------------*/
void no_number(void *nn){
long *xp;
   xp=(long *)(nn);
   *xp=-1;
   return;
};

int not_a_number(void *x){
long int *xp;
   xp=(long int *)(x);
   if(*xp==-1)return(-1);
   else return(0);
};
/*--------------------------------------------------------*/
int numchar(char c){
int i;
   switch( c){
      case '%':
      case '.':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':i=-1; break;
      default:i=0;
   }
   return(i);
};

/*--------------------------------------------------------*/
int wordchar(char *c){
  if (  (    (int)*c>=(int)'a' && (int)*c<=(int)'z'   ) ||
	(    (int)*c>=(int)'A' && (int)*c<=(int)'Z'   ) ||
	(    (int)*c>=(int)'0' && (int)*c<=(int)'9'   ) ||
	(    (int)*c==(int)'_'                       ) ||
	(    (int)*c==(int)'%'                       ) ||
	(    (int)*c==(int)'„'                       ) ||
	(    (int)*c==(int)''                       ) ||
	(    (int)*c==(int)'”'                       ) ||
	(    (int)*c==(int)'Ž'                       ) ||
	(    (int)*c==(int)'š'                       ) ||
	(    (int)*c==(int)'™'                       ) ||
	(    (int)*c==(int)'‚'                       ) ||
	(    (int)*c==(int)'ƒ'                       ) ||
	(    (int)*c==(int)'…'                       ) ||
	(    (int)*c==(int)'ˆ'                       ) ||
	(    (int)*c==(int)'Š'                       ) ||
	(    (int)*c==(int)'‹'                       ) ||
	(    (int)*c==(int)'Œ'                       ) ||
	(    (int)*c==(int)''                       ) ||
	(    (int)*c==(int)''                       ) ||
	(    (int)*c==(int)'“'                       ) ||
	(    (int)*c==(int)'•'                       ) ||
	(    (int)*c==(int)'–'                       ) ||
	(    (int)*c==(int)'—'                       ) ||
	(    (int)*c==(int)' '                       ) ||
	(    (int)*c==(int)'¡'                       ) ||
	(    (int)*c==(int)'¢'                       ) ||
	(    (int)*c==(int)'£'                       )


      )return(1);
  return(0);
}

void delete(char *st,int s,int c){
int l;
char *p1,*p2;
   l=strlen(st);
   if(s<1 || s>l || c<1)return;
   if(s+c>l+1) c=l-s+1;
   p1=st+(s-1);
   p2=p1+c;
   do{
      *p1=*p2;
      p1++;
      p2++;
   }while((int)(*(p2-1))!=0);
   return;
}
/*--------------------------------------------------------*/
char *wordpos(char *ss,char *s){
char *c;
int i;
   i=c_pos(ss,s);
   if(i==0)c=NULL;
   else c=s+i-1;
   if (c==NULL)return(c);
   if (c==s && wordchar(c+strlen(ss))==0) return(c);
   if (c+strlen(ss)==s+strlen(s) && wordchar(c-1)==0)return(c);
   if (c+strlen(ss)!=s+strlen(s) && c!=s &&
       wordchar(c-1)==0 && wordchar(c+strlen(ss))==0) return(c);
   return(NULL);
}
/*--------------------------------------------------------*/
char *wordpos_quote(char *trenn,char *s,char *qc){
/* searches for trenn outside of quotations */
char *res;
char *sp1,*sp2;
int quote,resl;
   res=strdup_(s,&resl);
   sp1=s;
   sp2=strstr_r(sp1,qc);
   quote=(sp1==sp2);
   sp1=strtok_r(sp1,qc);
   sp2=NULL;
   while(sp1!=NULL && sp2==NULL){
      if(quote==0){
	 sp2=wordpos(trenn,sp1);
      };
      quote=(!quote);
      sp1=strtok_r(NULL,qc);
   };
   strcpy(s,res);
   pasc_free(res,resl);
   return(sp2);
}
/*--------------------------------------------------------*/
char *pos_quote(char *trenn,char *s,char *qc){
/* searches for trenn outside of quotations */
char *res;
char *sp1,*sp2;
int quote,resl;
   res=strdup_(s,&resl);
   sp1=s;
   sp2=strstr_r(sp1,qc);
   quote=(sp1==sp2);
   sp1=strtok_r(sp1,qc);
   sp2=NULL;
   while(sp1!=NULL && sp2==NULL){
      if(quote==0){
	 sp2=strstr(sp1,trenn);
      };
      quote=(!quote);
      sp1=strtok_r(NULL,qc);
   };
   strcpy(s,res);
   pasc_free(res,resl);
   return(sp2);
}
/*--------------------------------------------------------*/
int str_ge(char *s1,char *s2){
char *p1,*p2;
int i;
   p1=s1;
   p2=s2;
   while(*p1!=(char)0 && *p2!=(char)0 &&
	 (int)*p1>=(int)*p2){
      p1++;
      p2++;
   };
   if((int)*p1>=(int)*p2)i=-1;
   else if(*p2==(char)0)i=-1;
   else i=0;
   return(i);
}
/*--------------------------------------------------------*/
char *strdup_(const char *s,int *l){
char *p,*p1;
int i;
   *l=strlen(s);
	 p=(char *)pasc_malloc(*l+1);
	 if(p==NULL)
			return(NULL);
   for(i=0,p1=p;i<*l+1;i++)
      *(p1++)=(char)0;
   if(*l>0)strcpy(p,s);
   *l=*l+1;
   return(p);
}
/*--------------------------------------------------------*/
int scan_char(int c)
{
   return(isprint(c) || wordchar((char *)&c));
}
/*--------------------------------------------------------*/
void scan_readln(FILE *infn,char *s){
char *p;
int i,fe;
long fpos;

   p=s;
   if(feof(infn)){
      strcpy(p,"");
      return;
   };
   do{
      fpos=ftell(infn);
      *p=(char)fgetc(infn);
      fe=feof(infn);
      i=(scan_char((int)*p)==0);
      if(i!=0)
	 *p++=(char)0;
      else if(fe)
	 *(++p)=(char)0;
      else
	 p++;
   }while(i==0 && fe==0);
   while(i!=0 && fe==0){
      fpos=ftell(infn);
      *p=(char)fgetc(infn);
      fe=feof(infn);
      i=(scan_char((int)*p)==0);
   };
   if(i==0){
      *(p+1)=(char)0;
      fe=fseek(infn,fpos,SEEK_SET);
      if((ungetc(*p,infn)-EOF)==0){
	 std_printf("Error in fputc!!!\n");
	 exit(0);
      };
      fe=fseek(infn,fpos,SEEK_SET);


   };
   return;
}
/*--------------------------------------------------------*/
int
#ifdef DLL_LINK
DLLEXPORT
#endif
textin(char *name,S_PTR **root){
FILE *inf;
S_PTR *p1,*p2,*w;
char cb[500];
    if((inf=fopen(name,"rt+"))==NULL){
       std_printf("can't open file %s!!\n",name);
       exit(0);
    };
    p2=NULL;
    w=NULL;
    while(feof(inf)==0){
			p1=(S_PTR *)pasc_malloc(sizeof(S_PTR));
			if(p1==NULL){
				 fclose(inf);
				 return(-1);
			};
      scan_readln(inf,cb);
			p1->sa=strdup_(cb,&(p1->si));
			if(p1->sa==NULL){
				 fclose(inf);
				 return(-1);
			};
      p1->s_p=NULL;
      if(p2==NULL)
	w=p1;
      else
	p2->s_p=p1;
      p2=p1;
    };
    fclose(inf);
    *root=w;
		return(0);
}
/*--------------------------------------------------------*/
void
#ifdef DLL_LINK
DLLEXPORT
#endif
disposestr(S_PTR **p1){
   pasc_free((*p1)->sa,(*p1)->si);
   pasc_free(*p1,sizeof(S_PTR));
   *p1=NULL;
   return;
}
/*--------------------------------------------------------*/
void
#ifdef DLL_LINK
DLLEXPORT
#endif
delstree(S_PTR **p){
S_PTR *p1,*p2;
   p1=*p;
   while(p1!=NULL){
      p2=p1->s_p;
      disposestr(&p1);
      p1=p2;
   };
   *p=NULL;
   return;
}
/*--------------------------------------------------------*/
void
#ifdef DLL_LINK
DLLEXPORT
#endif
changestr(S_PTR *p,char *s){
   pasc_free(p->sa,p->si);
   p->sa=strdup_(s,&(p->si));
   return;
}
/*--------------------------------------------------------*/
void
#ifdef DLL_LINK
DLLEXPORT
#endif
newstring(S_PTR **p,char *s){
   *p=(S_PTR *)pasc_malloc(sizeof(S_PTR));
   (*p)->sa=strdup_(s,&((*p)->si));
   (*p)->s_p=NULL;
   return;
}
/*--------------------------------------------------------*/
void inc_sptr_c(S_PTR **w,char **c){
   if(**c!=(char)0)(*c)++;
   else if((*w)->s_p){
      *w=(*w)->s_p;
      *c=(*w)->sa;
   }
   else {
     *w=NULL;
     *c=NULL;
   };
   return;
}
/*--------------------------------------------------------*/
int search_next(S_PTR **w,char **cs,char *item,S_PTR *stop_p,char *stop_c){
char *p;
S_PTR *zp;
int brk;
   zp=*w;
   p=*cs;
   brk=0;
   while(zp!=NULL && (stop_p==NULL || stop_p->s_p!=zp) && brk==0){
      if(p)p=strstr(p,item);
      if(p)brk=1;
      else{
	 *w=zp;
	 if(stop_p==NULL || zp!=stop_p)
	    *cs=zp->sa+strlen(zp->sa);
	 else
	    *cs=stop_c;
	 zp=zp->s_p;
	 if(zp)p=zp->sa;
	 else p=NULL;
      };
   };
   if(brk){
      *w=zp;
      if(stop_p==NULL || zp!=stop_p || p<=stop_c)
	 *cs=p;
      else{
	 *cs=stop_c;
	 brk=0;
      };
   };
   return(brk);
}
/*--------------------------------------------------------*/
void remove_block(S_PTR *ps,char *cs,S_PTR *pe,char *ce){
/* cs points to the first character to be deleted       **
 * ce points one character after the last to be deleted **/
char *hstr,*hstr1,*hstr2;
int hl,hl1,hl2;
char c;
S_PTR *zp,*zp1,*zp2;
char *p;
   zp=ps;
   zp2=zp;
   p=cs;
   if(zp==NULL)return;
   c=*p;
   *p=(char)0;
   hstr=strdup_(zp->sa,&hl);
   *p=c;
   if(zp!=pe)zp=zp->s_p;
   while(zp!=pe && zp!=NULL){
      zp1=zp->s_p;
      pasc_free(zp->sa,zp->si);
      pasc_free(zp,sizeof(S_PTR));
      zp=zp1;
   };
   if(zp){
      hstr2=strdup_(ce,&hl2);
      if(zp2!=zp){
	 pasc_free(zp2->sa,zp2->si);
	 zp2->sa=hstr;
	 zp2->si=hl;
	 pasc_free(zp->sa,zp->si);
	 zp->sa=hstr2;
	 zp->si=hl2;
	 zp2->s_p=zp;
      }
      else{
	 hl1=strlen(hstr2)+strlen(hstr)+1;
	 hstr1=(char *)pasc_malloc(hl1);
	 strcpy(hstr1,hstr);
	 strcat(hstr1,hstr2);
	 pasc_free(hstr,hl);
	 pasc_free(zp->sa,zp->si);
	 pasc_free(hstr2,hl2);
	 zp->sa=hstr1;
	 zp->si=hl1;
      };
   }
   else
      zp2->s_p=NULL;
}
/*--------------------------------------------------------*/
void concat_str(S_PTR *p1,char *s,int n){
char *hstr,*hs1;
int l,hl;
   if(n>strlen(s)) n=strlen(s);
   if(n<1) return;
   hl=strlen(p1->sa)+n+1;
   hstr=(char *)pasc_malloc(hl);
   strcpy(hstr,p1->sa);
   hs1=strdup_(s,&l);
   *(hs1+n)=(char)0;
   strcat(hstr,hs1);
   pasc_free(hs1,l);
   changestr(p1,hstr);
   pasc_free(hstr,hl);
   return;
};

void insert_cr(S_PTR *p1,char *cp){
/** inserts a CR before cp */
S_PTR *p2;
char *sp1,*hstr;
int hl;
char c;
   if(p1==NULL || cp==NULL )return;
   sp1=p1->sa;
   newstring(&p2,cp);
       /** cut old string */
   c=*cp;
   *cp=(char)0;
   hstr=strdup_(sp1,&hl);
   *cp=c;
   changestr(p1,hstr);
   pasc_free(hstr,hl);
      /** handle pointers */
   p2->s_p=p1->s_p;
   p1->s_p=p2;
   return;
}
/*--------------------------------------------------------*/
void erase_empty_lines(S_PTR **w){
S_PTR *p0,*p1,*p2;
   p2=NULL;
   p0=*w;
   while(p0!=NULL){
      p0->sa=lesh(p0->sa);
      if(strlen(p0->sa)==0){
	 p1=p0->s_p;
	 if(p2==NULL)*w=p1;
	 else p2->s_p=p1;
	 pasc_free(p0->sa,p0->si);
	 pasc_free(p0,sizeof(S_PTR));
	 p0=p1;
      }
      else {
	 p2=p0;
	 p0=p0->s_p;
      };
   };
}
/*--------------------------------------------------------*/
void erase_comments(S_PTR **w,char *trenna,char *trenne){
S_PTR *p0,*p1,*p2,*p3;
char *c0,*c1,*c2,*c3;
int i;
   p0=*w;
   c0=p0->sa;
   while(search_next(&p0,&c0,trenna,NULL,NULL)){
      p1=p0;
      c1=c0;
      for(i=0;i<strlen(trenna);i++)
	 inc_sptr_c(&p0,&c0);
      p2=p1;
      c2=c1;
      if(search_next(&p2,&c2,trenne,NULL,NULL)){
	 p3=p0;
	 c3=c0;
	 while(search_next(&p3,&c3,trenna,p2,c2)){
	    p1=p3;
	    c1=c3;
	    for(i=0;i<strlen(trenna);i++)
	       inc_sptr_c(&p3,&c3);
	 };
	 for(i=0;i<strlen(trenne);i++)
	    inc_sptr_c(&p2,&c2);
	 remove_block(p1,c1,p2,c2);
      }
      else{
	 std_printf("Open Comment!!!\n");
	 exit(0);
      }
      p0=*w;
      c0=p0->sa;
   };

   erase_empty_lines(w);
};


