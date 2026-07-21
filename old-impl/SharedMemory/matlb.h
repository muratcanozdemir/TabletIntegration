
#define  dynrec 1
#define  MATLB_TDOUBLE 0
#define  MATLB_TSINGLE 1
#define  MATLB_TLONGINT 2
#define  MATLB_TINT 3
#define  MATLB_TWORD 4
#define  MATLB_TBYTE 5
#define  MATLB_MAC 1
#define  MATLB_PC 0
extern int  MATLB_VSIZE[];
extern int matlab_double_write;

typedef struct mh{
   long typ,mrows,ncols,imagf,namlen;
}mathead;

void writematlabstring(FILE *file,char *variable_name,char *s,int out);
void writematlabstrarr(FILE *filout,char *variable_name,S_PTR *strarr,int out);
void readmatlabstrarr(FILE *filein,S_PTR **strarr);
void read_first_matlabstring(FILE *filein,S_PTR **strarr);
void writematlabvalue(FILE *filout,char *variable_name,void *value,int vtyp,int out);
void macheadwrite(FILE *filout,mathead head);
void macstringwrite(FILE *filout,char *s,int l);
void macstringread(FILE *filein,char *s,int l);
int is_matlab_file(FILE *f);
S_PTR *matlab_who(FILE *f);
int is_column_matrix(FILE *f,char *name);
void get_matlab_attr(FILE *f,char *name,long *pstart,long *pend,long *w,int *vartyp,int *is_string);
void write_matlab_attr(FILE *filout,char *variable_name,long h,long w,int vtyp,int row_on,int str_on,int out);
double read_matlab_value(FILE *f,int el_type);
void write_matrix_head(FILE *outf,char *name,long i,long j,int el_typ,int comp_typ);
void convert_head_to_PC(mathead *head,char *mopt);

