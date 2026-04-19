#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "policy.h"

static float relu(float x){return x>0?x:0;}

void policy_init(PPOPolicy *p){
    memset(p,0,sizeof(*p));
    p->action_bits[0]=1; p->action_bits[1]=2; p->action_bits[2]=4; p->action_bits[3]=8;
    p->has_weights=0;
}

int policy_load_text(PPOPolicy *p, const char *path){
    FILE *fp=fopen(path,"r"); if(!fp) return -1;
    for(int i=0;i<2;i++) for(int j=0;j<32;j++) if(fscanf(fp,"%f",&p->w1[i][j])!=1){fclose(fp);return -2;}
    for(int j=0;j<32;j++) if(fscanf(fp,"%f",&p->b1[j])!=1){fclose(fp);return -3;}
    for(int i=0;i<32;i++) for(int j=0;j<4;j++) if(fscanf(fp,"%f",&p->w2[i][j])!=1){fclose(fp);return -4;}
    for(int j=0;j<4;j++) if(fscanf(fp,"%f",&p->b2[j])!=1){fclose(fp);return -5;}
    fclose(fp); p->has_weights=1; return 0;
}

void policy_softmax(const PPOPolicy *p, float q, float v, float prob[4]){
    float logits[4]={0};
    if(p->has_weights){
        float h[32];
        for(int j=0;j<32;j++) h[j]=relu(q*p->w1[0][j]+v*p->w1[1][j]+p->b1[j]);
        for(int k=0;k<4;k++){ logits[k]=p->b2[k]; for(int j=0;j<32;j++) logits[k]+=h[j]*p->w2[j][k]; }
    } else {
        logits[0]=1.6f-3.2f*q-2.0f*v;
        logits[1]=0.8f+0.5f*q+0.2f*v;
        logits[2]=0.2f+2.2f*q+1.4f*v;
        logits[3]=-0.8f+3.6f*q+3.2f*v;
    }
    float m=logits[0]; for(int k=1;k<4;k++) if(logits[k]>m)m=logits[k];
    float s=0; for(int k=0;k<4;k++){prob[k]=expf(logits[k]-m); s+=prob[k];}
    for(int k=0;k<4;k++) prob[k]/=s;
}

int policy_select_action(const PPOPolicy *p, float q, float v, float target_bpp, float progress_ratio){
    float prob[4]; policy_softmax(p,q,v,prob);
    int best=0; float bestv=prob[0];
    for(int k=1;k<4;k++) if(prob[k]>bestv){best=k;bestv=prob[k];}
    /* budget-aware correction: target 0.4 bpp => avg 6.4 bits/block */
    float target_bits=target_bpp*16.0f;
    if(progress_ratio < 0.85f && target_bits >= 6.0f && best < 2 && (q+v)>0.75f) best=2;
    if(target_bits <= 2.0f && best > 1) best=1;
    return p->action_bits[best];
}
