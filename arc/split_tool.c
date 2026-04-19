#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { char *s; unsigned r; } Item;

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static unsigned lcg(unsigned *x){ *x = (*x)*1664525u + 1013904223u; return *x; }
static int cmp_item(const void *a,const void *b){ const Item *x=(const Item*)a,*y=(const Item*)b; return (x->r>y->r)-(x->r<y->r); }

int main(int argc,char **argv){
    if(argc<6){printf("Usage: gan_ppo_ambtc_split <all_images.txt> <out_dir> <train_ratio> <val_ratio> <seed>\n");return 1;}
    FILE *fp=fopen(argv[1],"r"); if(!fp){perror("list");return 2;}
    Item *items=NULL; int n=0,cap=0; char line[1024]; unsigned seed=(unsigned)atoi(argv[5]);
    while(fgets(line,sizeof(line),fp)){
        line[strcspn(line,"\r\n")]=0; if(!line[0])continue;
        if(n==cap){cap=cap?cap*2:1024; items=(Item*)realloc(items,sizeof(Item)*(size_t)cap); if(!items){fclose(fp); return 3;}}
        items[n].s=xstrdup(line); items[n].r=lcg(&seed); n++;
    }
    fclose(fp); qsort(items,(size_t)n,sizeof(Item),cmp_item);
    double tr=atof(argv[3]), vr=atof(argv[4]); int ntr=(int)(n*tr), nv=(int)(n*vr);
    char p[1024]; snprintf(p,sizeof(p),"%s/train.txt",argv[2]); FILE *ft=fopen(p,"w");
    snprintf(p,sizeof(p),"%s/val.txt",argv[2]); FILE *fv=fopen(p,"w");
    snprintf(p,sizeof(p),"%s/test.txt",argv[2]); FILE *fs=fopen(p,"w");
    if(!ft||!fv||!fs){perror("output split"); return 4;}
    for(int i=0;i<n;i++){ FILE *o=(i<ntr)?ft:((i<ntr+nv)?fv:fs); fprintf(o,"%s\n",items[i].s); free(items[i].s); }
    fclose(ft); fclose(fv); fclose(fs); free(items);
    printf("n=%d train=%d val=%d test=%d\n",n,ntr,nv,n-ntr-nv); return 0;
}
