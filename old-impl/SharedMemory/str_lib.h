typedef struct S_PTR_s {
		struct S_PTR_s *s_p;
		char *sa;
                int si;
		} S_PTR;




int str_back_search(char *ss,char *s);
char *fnam(char *s);
char *fext(char *s);
char *str_copy_func(char *s,int start,int count);
int get_number_from_end_of_string(char *s,char *numstr,int *index);
char *modify_name(char *hstr);
char *ch_ext(char *s,char *new_ext);
char *modify_aefile_name(char *s,int do_print_message);
char *modify_file_name(char *s,int do_print_message,char *default_ext_1,char *default_ext_2);

int c_pos(const char *ss,const char *s);
void str_copy(char *dest,char *src,int first,int cnt);
char *strstr_n(char *s,char *ss,int n);
char *strstr_r(char *s,char *ss);
char *strstr_n_r(char *s,char *ss,int n);
char *strtok_r(char *s,char *trenn);
void str_insert(char *is,char *s,int pos);
char *lesh(char *s);
char *rish(char *s);
void no_number(void *nn);
int not_a_number(void *x);
int numchar(char c);
int wordchar(char *c);
void delete(char *st,int s,int c);
char *wordpos(char *ss,char *s);
char *wordpos_quote(char *trenn,char *s,char *qc);
char *pos_quote(char *trenn,char *s,char *qc);
int str_ge(char *s1,char *s2);
char *strdup_(const char *s,int *l);



int scan_char(int c);
void scan_readln(FILE *infn,char *s);
int
#ifdef DLL_LINK
DLLEXPORT
#endif
textin(char *name,S_PTR **root);
void
#ifdef DLL_LINK
DLLEXPORT
#endif
disposestr(S_PTR **p1);
void
#ifdef DLL_LINK
DLLEXPORT
#endif
delstree(S_PTR **p);
void
#ifdef DLL_LINK
DLLEXPORT
#endif
changestr(S_PTR *p,char *s);
void
#ifdef DLL_LINK
DLLEXPORT
#endif

newstring(S_PTR **p,char *s);
void inc_sptr_c(S_PTR **w,char **c);
void concat_str(S_PTR *p1,char *s,int n);
int search_next(S_PTR **w,char **cs,char *item,S_PTR *stop_p,char *stop_c);
void remove_block(S_PTR *ps,char *cs,S_PTR *pe,char *ce);
void insert_cr(S_PTR *p1,char *cp);
void erase_empty_lines(S_PTR **w);
void erase_comments(S_PTR **w,char *trenna,char *trenne);



