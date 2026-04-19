#include <math.h>
#include <string.h>
#include "generator.h"

static int popcount16(uint16_t x){ int c=0; while(x){c+=x&1u; x>>=1;} return c; }
static unsigned bitpos_score(const AMBTCBlock *b, int i){
    unsigned mixed=(unsigned)(b->BM ^ (b->H<<8) ^ b->L ^ (i*1103515245u));
    mixed ^= mixed>>16; mixed*=0x7feb352dU; mixed^=mixed>>15; return mixed;
}
static unsigned char clampi(int x){ if(x<0)return 0; if(x>255)return 255; return (unsigned char)x; }

void generator_init(GeneratorLite *g){ g->alpha_pos=8.0f; g->beta_neg=8.0f; g->threshold=0.15f; g->compensation_gain=0.35f; }
float double_tanh(float x, float tau, float alpha, float beta){
    return 0.5f*(tanhf(alpha*(x-tau)) + tanhf(beta*(x+tau)));
}

void generator_embed_block(const GeneratorLite *g, const AMBTCBlock *cover, AMBTCBlock *stego, const uint8_t *bits, int nbits){
    *stego=*cover;
    int flips= nbits<16 ? nbits : 16;
    int before_ones=popcount16(cover->BM);
    for(int k=0;k<flips;k++){
        int pos=(int)(bitpos_score(cover,k)%16u);
        if(bits && bits[k]) stego->BM |= (uint16_t)(1u<<pos);
        else stego->BM &= (uint16_t)~(1u<<pos);
    }
    int after_ones=popcount16(stego->BM);
    int delta_ones=after_ones-before_ones;
    int spread=(int)(cover->H-cover->L);
    int correction=(int)lrintf(g->compensation_gain * delta_ones * spread / 16.0f);
    stego->H=clampi((int)stego->H-correction);
    stego->L=clampi((int)stego->L-correction);
    if(stego->H<stego->L){ unsigned char t=stego->H; stego->H=stego->L; stego->L=t; }
    stego->qld=(float)(stego->H-stego->L);
}
