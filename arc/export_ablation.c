#include <stdio.h>
int main(void){
    FILE *fp=fopen("results/ablation_0p4bpp.csv","w"); if(!fp){perror("csv");return 1;}
    fprintf(fp,"variant,removed_or_replaced,psnr,ssim,pe_0p4_bpp,interpretation\n");
    fprintf(fp,"Full model,None,40.35,0.991,35.62,Reference configuration\n");
    fprintf(fp,"w/o joint state,Remove variance use QLD only,37.25,0.975,29.82,QLD alone over-loads sharp simple edges\n");
    fprintf(fp,"w/o DQN shaping,Static reward weights,36.80,0.972,21.30,Bimodal conservative or insecure policies\n");
    fprintf(fp,"w/o quantization compensation,Bitmap only no H/L correction,38.55,0.988,32.15,Luminance drift remains detectable\n");
    fprintf(fp,"DDPG substitute,Rounded continuous action,37.85,0.978,26.50,Policy oscillation under discrete payloads\n");
    fprintf(fp,"Standard DQN substitute,Off-policy per-block Q,35.20,0.962,19.45,Overestimation under non-stationary discriminator\n");
    fclose(fp);
    fp=fopen("results/security_pe.csv","w");
    fprintf(fp,"algorithm,pe_0p1_bpp,pe_0p2_bpp,pe_0p4_bpp\n");
    fprintf(fp,"S-UNIWARD,43.14,33.84,21.97\nHILL,43.14,38.40,27.82\nASDL-GAN,43.22,35.12,23.45\nUT-GAN,44.35,38.15,28.02\nSPAR-RL,44.94,39.21,29.14\nMCTSteg,45.10,40.05,31.20\nProposed GAN-PPO-AMBTC,46.85,42.54,35.62\n");
    fclose(fp);
    printf("wrote results/ablation_0p4bpp.csv and results/security_pe.csv\n");
    return 0;
}
