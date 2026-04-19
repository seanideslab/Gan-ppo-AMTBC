#include <math.h>
#include "metrics.h"

double metric_mse(const GrayImage *a,const GrayImage *b){
    if(!a||!b||a->w!=b->w||a->h!=b->h) return -1;
    double s=0; int n=a->w*a->h;
    for(int i=0;i<n;i++){ double d=(double)a->data[i]-b->data[i]; s+=d*d; }
    return s/n;
}
double metric_psnr(const GrayImage *a,const GrayImage *b){ double mse=metric_mse(a,b); if(mse<=0) return 99.0; return 10.0*log10(255.0*255.0/mse); }
double metric_ssim_global(const GrayImage *a,const GrayImage *b){
    int n=a->w*a->h; double ma=0,mb=0,va=0,vb=0,cov=0;
    for(int i=0;i<n;i++){ma+=a->data[i]; mb+=b->data[i];} ma/=n; mb/=n;
    for(int i=0;i<n;i++){ double da=a->data[i]-ma, db=b->data[i]-mb; va+=da*da; vb+=db*db; cov+=da*db; }
    va/=(n-1); vb/=(n-1); cov/=(n-1);
    const double c1=6.5025, c2=58.5225;
    return ((2*ma*mb+c1)*(2*cov+c2))/((ma*ma+mb*mb+c1)*(va+vb+c2));
}
int bit_error_count(const uint8_t *a,const uint8_t *b,int nbits){ int e=0; for(int i=0;i<nbits;i++) if(a[i]!=b[i]) e++; return e; }
