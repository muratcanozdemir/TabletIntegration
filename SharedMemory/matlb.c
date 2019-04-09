#undef MSVC6
#define CYGWIN_GCC


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef MSVC6
	#ifndef CYGWIN_GCC
		#include <mem.h>
	#endif
#endif

#ifdef CYGWIN_GCC
	#define strtok_r strtok_r_eggert
#endif

#include "str_lib.h"
#include "matlb.h"



int MATLB_VSIZE[6]={8,4,4,2,2,1};


int matlab_double_write=0;

static const int int_zero=0;    /* needed for compatibility reasons with WATCOM 9.52 This compiler has a bug
                                 * related to assignment of zero in external data segments */

long filesize(FILE *stream)
{
   long curpos, length;

   curpos = ftell(stream);
   fseek(stream, (long)int_zero, SEEK_END);
   length = ftell(stream);
   fseek(stream, curpos, SEEK_SET);
   return(length);
};

unsigned long atol_radix_(const char *a,int radix){
char *p;
unsigned long v,base,i;
int ld_radix;

 ld_radix=(int)(log((double)radix)/log(2));
	p=(char *)a;
 if(radix!=10){
    v=ld_radix*(strlen(p)-1);
    base=(unsigned long)1;
    base=(unsigned long)(base << v);
 }
 else
    base=(unsigned long)(exp((double)(strlen(p)-1)*log(10.0)));
	v=(unsigned long)int_zero;
	while(*p!=(char)int_zero){
		i=(unsigned long)(*p);
		if(i>(unsigned long)96)
			i=i-87;
		else if (i>(unsigned long)64)
			i=i-55;
  else
   i=i-48;
		v+=base*i;
		p++;
  if(radix!=10)
   		base=base>>ld_radix;
  else
     base=base/radix;
	};
	return(v);
};


int c_to_int(char c){
char cs[2];
   cs[1]=(char)int_zero;
   cs[int_zero]=c;
   return((int)atol_radix_(cs,10));
};

void change(void *l,int size)
/* exchanges upper and lower bytes */
{
unsigned char *ba;
unsigned char  i,b;
  ba=l;
  for (i=(unsigned char)int_zero;i<size/2;i++){
    b=ba[size-i-1];
    ba[size-i-1]=ba[i];
    ba[i]=b;
  };
};

void macheadwrite(FILE *filout,mathead head)
{
  change(&head.typ,4);
  change(&head.ncols,4);
  change(&head.mrows,4);
  change(&head.namlen,4);
  change(&head.imagf,4);
  fwrite(&head,sizeof(head),1,filout);
};

void macstringwrite(FILE *filout,char *s,int l)
{
unsigned int buf[255];
int i;
  for (i=int_zero;i<strlen(s);i++)
    buf[i]=(unsigned int)(s[i]);
  for (i=strlen(s);i<l;i++)
    buf[i]=(unsigned int)' ';
  for (i=int_zero;i<l;i++)
    change(&(buf[i]),2);
  fwrite(buf,l*2,1,filout);
};

void macstringread(FILE *filein,char *s,int l)
{
unsigned int buf[255];
int i;
  fread(buf,l*2,1,filein);
  for (i=int_zero;i<l;i++)
    change(&buf[i],2);
  for (i=int_zero;i<l;i++)
    s[i]=(char)(buf[i]);
  s[l]=(char)int_zero;
};

void writematlabstring(FILE *file,char *variable_name,char *s,int out)
{
mathead head;
double d;
int i;
  if(out==(int)MATLB_PC) {
    if( matlab_double_write )
       head.typ=out*1000+101; /*row double string*/
    else
       head.typ=out*1000+151; /*row byte string*/
  }
  else
    head.typ=out*1000+141;/*row word string*/
  head.ncols=strlen(s);
  head.mrows=1;
  head.namlen=strlen(variable_name)+1;
  head.imagf=(long)int_zero;
  if( out==MATLB_PC )
    fwrite(&head,sizeof(head),1,file);
  else
    macheadwrite(file,head);
  fwrite(variable_name,(int)head.namlen,1,file);
  if( out==MATLB_PC ) {
    if( matlab_double_write ) {
       for(i=int_zero;i<strlen(s);i++){
          d=(double)(s[i]);
          fwrite(&d,sizeof(double),1,file);
       };
    }
    else
       fwrite(s,strlen(s),1,file);
  }
  else
    macstringwrite(file,s,strlen(s));
};


void writematlabstrarr(FILE *filout,char *variable_name,S_PTR *strarr,int out)
{
mathead head;
long i,maxlen;
S_PTR *s;
char blanks[200];
double dbuf[255];
double dblbuf[255];
int j;
  memset(blanks,(int)' ',sizeof(blanks));
  s=strarr;i=(long)int_zero;maxlen=(long)int_zero;
  while (s!=NULL) {
    i++;
    if( maxlen<strlen(s->sa) )
      maxlen=strlen(s->sa);
    s=s->s_p;
  };
  if( out==MATLB_PC ) {
    if( matlab_double_write ) {
       for (j=int_zero;j<255;j++)
          dblbuf[j]=(double)(' ');
       head.typ=out*1000+101; /*row double string*/
    }
    else
       head.typ=out*1000+151; /*row byte string*/
  }
  else
    head.typ=out*1000+141;/*row word string*/
  head.ncols=maxlen;
  head.mrows=i;
  head.namlen=strlen(variable_name)+1;
  head.imagf=(long)int_zero;
  if( out==MATLB_PC )
    fwrite(&head,sizeof(head),1,filout);
  else
    macheadwrite(filout,head);
  fwrite(variable_name,(int)head.namlen,1,filout);
  s=strarr;
  while (s!=NULL){
    if( out==MATLB_PC ) {
      if( matlab_double_write ) {
         for (j=int_zero;j<strlen(s->sa);j++)
            dbuf[j]=(double)(s->sa[j]);
         fwrite(dbuf,strlen(s->sa)*sizeof(double),1,filout);
         fwrite(dblbuf,(int)(maxlen-strlen(s->sa))*sizeof(double),1,filout);
      }
      else {
         fwrite(s->sa,strlen(s->sa),1,filout);
         fwrite(blanks,(int)(maxlen-strlen(s->sa)),1,filout);
      };
    }
    else
      macstringwrite(filout,s->sa,(int)maxlen);
    s=s->s_p;
  };
};


void readmatlabstrarr(FILE *filein,S_PTR **strarr)
/** append strings to strarr **
 ** file must be positioned on header start before calling **/
{
mathead head;
long i,j;
S_PTR *s,*s1;
char name[200],blanks[200],mopt[10];
double dbuf[255];
  memset(blanks,(int)' ',sizeof(blanks));
  s=*strarr;
  while (s!=NULL && s->s_p!=NULL)
     s=s->s_p;
  fread(&head,sizeof(head),1,filein);
  /*ltoa(head.typ,mopt,10);*/
  sprintf_s(mopt,sizeof(mopt),"%d",head.typ);
  if( strlen(mopt)>4 ) {
    change(&head.typ,4);
    /*ltoa(head.typ,mopt,10);*/
  	sprintf(mopt,"%d",head.typ);
    change(&head.mrows,4);
    change(&head.ncols,4);
    change(&head.imagf,4);
    change(&head.namlen,4);
  };
  while (strlen(mopt)<4)
    str_insert("0",mopt,1);

  fread(name,(int)head.namlen,1,filein);
  name[(int)head.namlen]=(char)int_zero;

  for (i=(long)int_zero;i<head.mrows;i++) {
    if( mopt[int_zero]=='0' ) {
      switch (mopt[2]){
         case '5':fread(blanks,(int)head.ncols,1,filein);
         break;
         case '0':
            fread(dbuf,(int)(head.ncols*sizeof(double)),1,filein);
            for (j=(long)int_zero;j<head.ncols;j++)
               blanks[(int)j]=(char)(dbuf[(int)j]);
         break;
      };
      blanks[(int)head.ncols]=(char)int_zero;
    }
    else
      macstringread(filein,blanks,(int)head.ncols);
    rish(blanks);
    newstring(&s1,blanks);
    if( s==NULL ) {
       s=s1;
       *strarr=s;
    }
    else
       s->s_p=s1;
    s=s1;
  };
};


void read_first_matlabstring(FILE *filein,S_PTR **strarr)
/** append strings to strarr **
 ** file must be positioned on header start before calling **/
{
mathead head;
long i,j;
S_PTR *s,*s1;
char name[200],blanks[200],mopt[10];
double dbuf[255];
  memset(blanks,(int)' ',sizeof(blanks));
  s=*strarr;
  while (s!=NULL && s->s_p!=NULL)
     s=s->s_p;
  fread(&head,sizeof(head),1,filein);
  /*ltoa(head.typ,mopt,10);*/
  sprintf(mopt,"%d",head.typ);
  if( strlen(mopt)>4 ) {
    change(&head.typ,4);
    /*ltoa(head.typ,mopt,10);*/
    sprintf(mopt,"%d",head.typ);
    change(&head.mrows,4);
    change(&head.ncols,4);
    change(&head.imagf,4);
    change(&head.namlen,4);
  };
  while (strlen(mopt)<4)
    str_insert("0",mopt,1);

  fread(name,(int)head.namlen,1,filein);
  name[(int)head.namlen]=(char)int_zero;

  for (i=(long)int_zero;i<1;i++) {
    if( mopt[int_zero]=='0' ) {
      switch (mopt[2]){
         case '5':fread(blanks,(int)head.ncols,1,filein);
         break;
         case '0':
            fread(dbuf,(int)(head.ncols*sizeof(double)),1,filein);
            for (j=(long)int_zero;j<head.ncols;j++)
               blanks[(int)j]=(char)(dbuf[(int)j]);
         break;
      };
      blanks[(int)head.ncols]=(char)int_zero;
    }
    else
      macstringread(filein,blanks,(int)head.ncols);
    rish(blanks);
    newstring(&s1,blanks);
    if( s==NULL ) {
       s=s1;
       *strarr=s;
    }
    else
       s->s_p=s1;
    s=s1;
  };
};

void writematlabvalue(FILE *filout,char *variable_name,void *value,int vtyp,int out)
{
mathead head;
  head.typ=out*1000+10*vtyp; /*col type matrix*/
  head.ncols=1;
  head.mrows=1;
  head.namlen=strlen(variable_name)+1;
  head.imagf=(long)int_zero;
  if( out==MATLB_PC )
    fwrite(&head,sizeof(head),1,filout);
  else
    macheadwrite(filout,head);
  fwrite(variable_name,(int)head.namlen,1,filout);
  if( out==MATLB_MAC )
    change(value,MATLB_VSIZE[vtyp]);
  fwrite(value,MATLB_VSIZE[vtyp],1,filout);
};


int check_head(mathead h){
char mopt[10];
int macpc,rowcol,vartyp,styp;

   /*ltoa(h.typ,mopt,10);*/
   sprintf(mopt,"%d",h.typ);
   if( (h.imagf!=(long)int_zero) && (h.imagf!=1) )
      return(int_zero);
   else if( strlen(mopt)>4 )
      return(int_zero);
   else if( (h.namlen>127) || (h.namlen<1) )
      return(int_zero);
   else if( h.mrows<1 )
      return(int_zero);
   else if( h.ncols<1 )
      return(int_zero);
   else {
      while (strlen(mopt)<4)
         str_insert("0",mopt,1);
      macpc=c_to_int(mopt[int_zero]);
      rowcol=c_to_int(mopt[1]);
      vartyp=c_to_int(mopt[2]);
      styp=c_to_int(mopt[3]);
      if( (macpc!=MATLB_MAC) && (macpc!=MATLB_PC) )
          return(int_zero);
      else if( (rowcol!=int_zero) && (rowcol!=1) )
          return(int_zero);
      else if( (vartyp<int_zero) || (vartyp>5) )
          return(int_zero);
      else if( (styp!=int_zero) && (styp!=1) )
          return(int_zero);
   };
   return(1);
};


int is_matlab_file(FILE *f)
{
mathead head;
char mopt[200];
long fp;
   fp=ftell(f);
   fseek(f,(long)int_zero,SEEK_SET);
   if( filesize(f)-sizeof(head)<=(long)int_zero )
      return(int_zero);
   fread(&head,sizeof(head),1,f);
   /*ltoa(head.typ,mopt,10);*/
   sprintf(mopt,"%d",head.typ);
   if( strlen(mopt)>4 ) {
      change(&head.typ,4);
      /*ltoa(head.typ,mopt,10);*/
      sprintf(mopt,"%d",head.typ);
      change(&head.mrows,4);
      change(&head.ncols,4);
      change(&head.imagf,4);
      change(&head.namlen,4);
   };
   fseek(f,fp,SEEK_SET);
   return(check_head(head));
};

S_PTR *matlab_who(FILE *f)
  /** fileposition is not changed.           **/
{
mathead head;
char mopt[10],nam[200],title[200];
long size,fsize;
long fp,fp0;
S_PTR *spw,*sp1,*sp2;
int vartyp,i;

   fsize=filesize(f);
   fp0=ftell(f);
   fseek(f,(long)int_zero,SEEK_SET);
   i=int_zero;
   spw=NULL;
   sp2=NULL;
   while (ftell(f)<fsize){
      i++;
      fread(&head,sizeof(head),1,f);
      /*ltoa(head.typ,mopt,10);*/
      sprintf(mopt,"%d",head.typ);
      if( strlen(mopt)>4 ) {
         change(&head.typ,4);
         /*ltoa(head.typ,mopt,10);*/
         sprintf(mopt,"%d",head.typ);
         change(&head.mrows,4);
         change(&head.ncols,4);
         change(&head.imagf,4);
         change(&head.namlen,4);
      };
      while (strlen(mopt)<4)
         str_insert("0",mopt,1);
      vartyp=c_to_int(mopt[2]);
      fread(nam,(int)head.namlen,1,f);
      size=head.mrows*head.ncols*MATLB_VSIZE[vartyp];
      switch( head.imagf){
         case 1:size=size*2;
         break;
      };
      /*ltoa(i,title,10);*/
      sprintf(title,"%d",i);
      while(strlen(title)<5)
         strcat(title," ");
      strcat(title,":");
      strcat(title,nam);
      newstring(&sp1,title);
      sp1->s_p=NULL;
      if( sp2==NULL )
         spw=sp1;
      else
         sp2->s_p=sp1;
      sp2=sp1;
      fp=ftell(f);
      fseek(f,fp+size,SEEK_SET);
   };
   fseek(f,fp0,SEEK_SET);
   return(spw);
};

int is_column_matrix(FILE *f,char *name)
  /** fileposition is not changed.           **
   ** variable name must exist in the file!! **/
{
mathead head;
char mopt[10],nam[200];
long size,fpos0,fp;
long fsize;
int vartyp,found;

   fsize=filesize(f);
   fpos0=ftell(f);
   fseek(f,(long)int_zero,SEEK_SET);
   found=int_zero;
   while(fsize>ftell(f) && !found){
      fread(&head,sizeof(head),1,f);
      /*ltoa(head.typ,mopt,10);*/
      sprintf(mopt,"%d",head.typ);
      if( strlen(mopt)>4 ) {
         change(&head.typ,4);
         /*ltoa(head.typ,mopt,10);*/
         sprintf(mopt,"%d",head.typ);
         change(&head.mrows,4);
         change(&head.ncols,4);
         change(&head.imagf,4);
         change(&head.namlen,4);
      };
      while (strlen(mopt)<4)
         str_insert("0",mopt,1);
      vartyp=c_to_int(mopt[2]);
      fread(nam,(int)head.namlen,1,f);
      size=head.mrows*head.ncols*MATLB_VSIZE[vartyp];
      switch(head.imagf){
         case 1:size=size*2;
         break;
      };
      found=strcmp(nam,name)==int_zero;
      fp=ftell(f);
      if( !found )
         fseek(f,fp+size,SEEK_SET);
   };
   fseek(f,fpos0,SEEK_SET);
   return(mopt[1]=='0');
};

void get_matlab_attr(FILE *f,char *name,long *pstart,long *pend,long *w,int *vartyp,int *is_string)
       /** returns pstart==-1 on error **/
{
mathead head;
char mopt[10],nam[200];
long size,fp;
long fsize;
int found;

   fsize=filesize(f);
   fseek(f,(long)int_zero,SEEK_SET);
   found=int_zero;
   *pstart=-1;
   while (fsize>ftell(f) && !found) {
      fread(&head,sizeof(head),1,f);
      /*ltoa(head.typ,mopt,10);*/
      sprintf(mopt,"%d",head.typ);
      if( strlen(mopt)>4 ) {
         change(&head.typ,4);
         /*ltoa(head.typ,mopt,10);*/
         sprintf(mopt,"%d",head.typ);
         change(&head.mrows,4);
         change(&head.ncols,4);
         change(&head.imagf,4);
         change(&head.namlen,4);
      };
      while (strlen(mopt)<4)
         str_insert("0",mopt,1);
      *vartyp=c_to_int(mopt[2]);
      fread(nam,(int)head.namlen,1,f);
      size=head.mrows*head.ncols*MATLB_VSIZE[*vartyp];
      switch( head.imagf){
         case 1:size=size*2;
         break;
      };
      found=strcmp(nam,name)==int_zero;
      fp=ftell(f);
      if( !found ) {
         *pstart=-1;
         fseek(f,fp+size,SEEK_SET);
      }
      else {
         *pstart=fp;
         *pend=fp+size;
         /**
         if( mopt[2]=='0' )
            w=head.mrows
         else
         **/
         *is_string=(mopt[3]=='1');
         *w=head.ncols;
      };
   };

};

void write_matlab_attr(FILE *filout,char *variable_name,long h,long w,int vtyp,int row_on,int str_on,int out)
       /** returns pstart==-1 on error **/
{
mathead head;

  head.typ=out*1000+10*vtyp; /*col type matrix*/
  if( row_on )
     head.typ=head.typ+100;
  if( str_on )
     head.typ=head.typ+1;
  head.ncols=w;
  head.mrows=h;
  head.namlen=strlen(variable_name)+1;
  head.imagf=(long)int_zero;
  if( out==MATLB_PC )
    fwrite(&head,sizeof(head),1,filout);
  else
    macheadwrite(filout,head);
  fwrite(variable_name,(int)head.namlen,1,filout);
  return;
};

double read_matlab_value(FILE *f,int el_type)
/** f has to be positioned at the value !! **
 ** file pointer is changed!! **/
{
double d;
float s;
unsigned char b;
unsigned int w;
int i;
long l;

  fread(&d,MATLB_VSIZE[el_type],1,f);
  switch (el_type){
     case MATLB_TDOUBLE:
     break;
     case MATLB_TSINGLE:
        memmove(&s,&d,MATLB_VSIZE[el_type]);
        d=(double)s;
     break;
     case MATLB_TLONGINT:
        memmove(&l,&d,MATLB_VSIZE[el_type]);
        d=(double)l;
     break;
     case MATLB_TINT:
        memmove(&i,&d,MATLB_VSIZE[el_type]);
        d=(double)i;
     break;
     case MATLB_TWORD:
        memmove(&w,&d,MATLB_VSIZE[el_type]);
        d=(double)w;
     break;
     case MATLB_TBYTE:
        memmove(&b,&d,MATLB_VSIZE[el_type]);
        d=(double)b;
     break;
  };
  return(d);
};


void write_matrix_head(FILE *outf,char *name,long i,long j,int el_typ,int comp_typ)
{
mathead head;
   head.typ=comp_typ*1000+100*1+el_typ*10; /*row double matrix*/
   head.namlen=strlen(name)+1;
   head.mrows=i;
   head.ncols=j;
   head.imagf=(long)int_zero;
   if( comp_typ==MATLB_MAC )
     macheadwrite(outf,head);
   else
     fwrite(&head,sizeof(head),1,outf);
   fwrite(name,(int)head.namlen,1,outf);
};

void convert_head_to_PC(mathead *head,char *mopt)
{
   /*ltoa((*head).typ,mopt,10);*/
   sprintf(mopt,"%d",(*head).typ);
   if( strlen(mopt)>4 ) {
     change(&(*head).typ,4);
     /*ltoa((*head).typ,mopt,10);*/
     sprintf(mopt,"%d",(*head).typ);
     change(&(*head).mrows,4);
     change(&(*head).ncols,4);
     change(&(*head).imagf,4);
     change(&(*head).namlen,4);
   };
   while (strlen(mopt)<4 )
      str_insert("0",mopt,1);
};


