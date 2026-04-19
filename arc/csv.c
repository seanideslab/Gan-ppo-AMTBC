#include <stdio.h>
#include "csv.h"
int write_results_header(const char *path){ FILE *fp=fopen(path,"w"); if(!fp)return -1; fprintf(fp,"image_id,bpp,psnr,ssim,embedded_bits,bit_errors\n"); fclose(fp); return 0; }
int append_result_row(const char *path,const char *image_id,double bpp,double psnr,double ssim,int bits,int bit_errors){ FILE *fp=fopen(path,"a"); if(!fp)return -1; fprintf(fp,"%s,%.4f,%.4f,%.6f,%d,%d\n",image_id,bpp,psnr,ssim,bits,bit_errors); fclose(fp); return 0; }
