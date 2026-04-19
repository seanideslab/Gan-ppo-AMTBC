#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "image_io.h"
#include "ambtc.h"
#include "policy.h"
#include "generator.h"
#include "metrics.h"
#include "csv.h"

static uint8_t pseudo_bit(unsigned idx){ idx^=idx>>16; idx*=0x7feb352dU; idx^=idx>>15; return (idx&1u)?1:0; }
static const char *base_name(const char *p){ const char *s=strrchr(p,'/'); return s?s+1:p; }

static int eval_one(const char *path, float target_bpp, PPOPolicy *policy, const char *csv){
    GrayImage cover={0}, rec={0}; AMBTCImage amb={0}, stego={0};
    if(pgm_read(path,&cover)!=0){fprintf(stderr,"skip unreadable %s\n",path);return -1;}
    if(ambtc_encode(&cover,4,&amb)!=0){image_free(&cover);return -2;}
    stego=amb; stego.blocks=(AMBTCBlock*)calloc((size_t)amb.nblocks,sizeof(AMBTCBlock));
    GeneratorLite gen; generator_init(&gen);
    int total_bits=0;
    for(int i=0;i<amb.nblocks;i++){
        float q,v; ambtc_block_state(&amb.blocks[i],&q,&v);
        int nbits=policy_select_action(policy,q,v,target_bpp,(float)i/amb.nblocks);
        uint8_t bits[16]; for(int k=0;k<nbits;k++) bits[k]=pseudo_bit((unsigned)(i*17+k));
        generator_embed_block(&gen,&amb.blocks[i],&stego.blocks[i],bits,nbits);
        total_bits+=nbits;
    }
    ambtc_decode(&stego,&rec);
    append_result_row(csv,base_name(path),(double)total_bits/(cover.w*cover.h),metric_psnr(&cover,&rec),metric_ssim_global(&cover,&rec),total_bits,0);
    image_free(&cover); image_free(&rec); ambtc_free(&amb); free(stego.blocks);
    return 0;
}

int main(int argc,char **argv){
    if(argc<4){printf("Usage: gan_ppo_ambtc_eval <list.txt> <out.csv> <target_bpp> [policy.txt]\n");return 1;}
    FILE *lst=fopen(argv[1],"r"); if(!lst){fprintf(stderr,"Cannot open list %s\n",argv[1]);return 2;}
    float bpp=(float)atof(argv[3]);
    PPOPolicy policy; policy_init(&policy); if(argc>=5) policy_load_text(&policy,argv[4]);
    write_results_header(argv[2]);
    char path[1024]; int n=0;
    while(fgets(path,sizeof(path),lst)){
        path[strcspn(path,"\r\n")]=0; if(!path[0]) continue;
        if(eval_one(path,bpp,&policy,argv[2])==0) n++;
    }
    fclose(lst); printf("evaluated=%d csv=%s\n",n,argv[2]); return 0;
}
