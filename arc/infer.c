#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "image_io.h"
#include "ambtc.h"
#include "policy.h"
#include "generator.h"
#include "metrics.h"

static void usage(void){
    printf("Usage: gan_ppo_ambtc_infer <input.pgm> <output.pgm> [target_bpp] [policy.txt]\n");
}
static uint8_t pseudo_bit(unsigned idx){ idx^=idx>>17; idx*=0xed5ad4bbU; idx^=idx>>11; return (idx&1u)?1:0; }

int main(int argc,char **argv){
    if(argc<3){usage();return 1;}
    GPConfig cfg=default_config();
    if(argc>=4) cfg.target_bpp=(float)atof(argv[3]);
    GrayImage cover={0}, stego_img={0}; AMBTCImage amb={0}, stego={0};
    if(pgm_read(argv[1],&cover)!=0){fprintf(stderr,"Cannot read PGM: %s\n",argv[1]);return 2;}
    if(ambtc_encode(&cover,cfg.block_size,&amb)!=0){fprintf(stderr,"AMBTC encode failed\n");return 3;}
    stego=amb; stego.blocks=(AMBTCBlock*)calloc((size_t)amb.nblocks,sizeof(AMBTCBlock));
    if(!stego.blocks){fprintf(stderr,"OOM\n");return 4;}
    PPOPolicy policy; policy_init(&policy);
    if(argc>=5 && policy_load_text(&policy,argv[4])!=0) fprintf(stderr,"Warning: using built-in heuristic policy; cannot load %s\n",argv[4]);
    GeneratorLite gen; generator_init(&gen);
    int total_bits=0;
    for(int i=0;i<amb.nblocks;i++){
        float q,v; ambtc_block_state(&amb.blocks[i],&q,&v);
        float progress=(float)i/(float)amb.nblocks;
        int nbits=policy_select_action(&policy,q,v,cfg.target_bpp,progress);
        uint8_t bits[16]; for(int k=0;k<nbits;k++) bits[k]=pseudo_bit((unsigned)(i*31+k));
        generator_embed_block(&gen,&amb.blocks[i],&stego.blocks[i],bits,nbits);
        total_bits += nbits;
    }
    if(ambtc_decode(&stego,&stego_img)!=0){fprintf(stderr,"Decode failed\n");return 5;}
    if(pgm_write(argv[2],&stego_img)!=0){fprintf(stderr,"Cannot write PGM: %s\n",argv[2]);return 6;}
    double actual_bpp=(double)total_bits/(cover.w*cover.h);
    printf("embedded_bits=%d actual_bpp=%.4f PSNR=%.4f SSIM=%.6f\n",total_bits,actual_bpp,metric_psnr(&cover,&stego_img),metric_ssim_global(&cover,&stego_img));
    image_free(&cover); image_free(&stego_img); ambtc_free(&amb); free(stego.blocks);
    return 0;
}
