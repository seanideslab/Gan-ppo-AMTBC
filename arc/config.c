#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"

GPConfig default_config(void) {
    GPConfig c;
    c.block_size = 4; c.image_w = 512; c.image_h = 512;
    c.epochs = 200; c.batch_size = 8;
    c.target_bpp = 0.4f;
    c.ppo_clip_eps = 0.2f; c.gae_lambda = 0.95f; c.gamma = 0.99f;
    c.lr_actor = 3e-4f; c.lr_critic = 3e-4f;
    c.lr_generator = 1e-4f; c.lr_discriminator = 4e-4f;
    c.d_updates_per_g = 4;
    c.dqn_replay_capacity = 10000; c.dqn_tau = 0.005f;
    c.lambda_payload = 1.0f; c.lambda_distortion = 0.4f; c.lambda_security = 0.6f; c.lambda_budget = 0.1f;
    return c;
}

static void set_float(const char *key, const char *val, GPConfig *c) {
    float f = (float)atof(val);
    if (!strcmp(key,"target_bpp")) c->target_bpp = f;
    else if (!strcmp(key,"ppo_clip_eps")) c->ppo_clip_eps = f;
    else if (!strcmp(key,"gae_lambda")) c->gae_lambda = f;
    else if (!strcmp(key,"gamma")) c->gamma = f;
    else if (!strcmp(key,"lr_actor")) c->lr_actor = f;
    else if (!strcmp(key,"lr_critic")) c->lr_critic = f;
    else if (!strcmp(key,"lr_generator")) c->lr_generator = f;
    else if (!strcmp(key,"lr_discriminator")) c->lr_discriminator = f;
    else if (!strcmp(key,"dqn_tau")) c->dqn_tau = f;
    else if (!strcmp(key,"lambda_payload")) c->lambda_payload = f;
    else if (!strcmp(key,"lambda_distortion")) c->lambda_distortion = f;
    else if (!strcmp(key,"lambda_security")) c->lambda_security = f;
    else if (!strcmp(key,"lambda_budget")) c->lambda_budget = f;
}

static void set_int(const char *key, const char *val, GPConfig *c) {
    int i = atoi(val);
    if (!strcmp(key,"block_size")) c->block_size = i;
    else if (!strcmp(key,"image_w")) c->image_w = i;
    else if (!strcmp(key,"image_h")) c->image_h = i;
    else if (!strcmp(key,"epochs")) c->epochs = i;
    else if (!strcmp(key,"batch_size")) c->batch_size = i;
    else if (!strcmp(key,"d_updates_per_g")) c->d_updates_per_g = i;
    else if (!strcmp(key,"dqn_replay_capacity")) c->dqn_replay_capacity = i;
}

int load_config_kv(const char *path, GPConfig *cfg) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char key[128], val[128];
        if (sscanf(line, " %127[^=]=%127s", key, val) == 2) {
            if (!strncmp(key,"lr_",3) || strstr(key,"lambda") || strstr(key,"bpp") || strstr(key,"eps") || strstr(key,"gamma") || strstr(key,"tau"))
                set_float(key, val, cfg);
            else set_int(key, val, cfg);
        }
    }
    fclose(fp);
    return 0;
}

void print_config(const GPConfig *c) {
    printf("block_size=%d\nimage=%dx%d\nepochs=%d\ntarget_bpp=%.3f\nppo_clip_eps=%.2f\n",
           c->block_size, c->image_w, c->image_h, c->epochs, c->target_bpp, c->ppo_clip_eps);
}
