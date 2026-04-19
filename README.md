# gan_ppo_ambtc — C Implementation Package

This folder is a C-oriented project scaffold for the paper method **GAN-PPO-AMBTC: Security-Aware Payload Control and Residual Suppression for Data Hiding in AMBTC-Compressed Images**.

It includes reproducible AMBTC-domain preprocessing, deterministic split tools, payload allocation inference, lightweight quantization-compensation embedding, evaluation CSV generation, ablation/result CSV export, and smoke-test weights.

> Important: this C version is designed for reproducible project packaging and inference/evaluation scaffolding. Full PPO + DQN + U-Net + SRM-SRNet training is normally implemented in a tensor framework such as PyTorch, then exported to C. The included `policy_smoke.txt` is not a trained paper-quality checkpoint.

---

## 1. Folder layout

```text
gan_ppo_ambtc/
├── Makefile
├── README.md
├── configs/
│   ├── default.cfg
│   └── payloads.cfg
├── data/
│   └── splits/
│       └── README.md
├── examples/
│   ├── all_images.txt
│   └── lena_like_64.pgm
├── include/
│   ├── ambtc.h
│   ├── config.h
│   ├── csv.h
│   ├── generator.h
│   ├── image_io.h
│   ├── metrics.h
│   └── policy.h
├── results/
│   ├── ablation_0p4bpp.csv
│   └── security_pe.csv
├── src/
│   ├── ambtc.c
│   ├── config.c
│   ├── csv.c
│   ├── evaluate.c
│   ├── export_ablation.c
│   ├── generator.c
│   ├── image_io.c
│   ├── infer.c
│   ├── metrics.c
│   ├── policy.c
│   ├── split_tool.c
│   └── train_stub.c
└── weights/
    ├── README_weights.md
    └── policy_smoke.txt
```

---

## 2. Build

Linux / macOS / WSL:

```bash
make
```

Run the full smoke test:

```bash
make smoke
```

Expected outputs:

```text
results/stego_64.pgm
results/eval_smoke.csv
results/ablation_0p4bpp.csv
results/security_pe.csv
```

---

## 3. Main commands

### 3.1 Generate split lists

Input: a text file containing one image path per line.

```bash
bin/gan_ppo_ambtc_split examples/all_images.txt data/splits 0.7 0.1 0
```

Output:

```text
data/splits/train.txt
data/splits/val.txt
data/splits/test.txt
```

For paper-scale reproduction, build the list from BOSSbase 1.01 and BOWS-2 images. The protocol described in the paper holds out 5,000 BOSSbase images as an unseen test partition.

### 3.2 Inference / embedding

```bash
bin/gan_ppo_ambtc_infer examples/lena_like_64.pgm results/stego_64.pgm 0.4 weights/policy_smoke.txt
```

Arguments:

```text
<input.pgm> <output.pgm> [target_bpp] [policy.txt]
```

The program:

1. reads a grayscale PGM image;
2. partitions it into non-overlapping 4 × 4 AMBTC blocks;
3. extracts QLD and variance state;
4. selects block payload from `{1, 2, 4, 8}` bits/block;
5. applies lightweight bitmap embedding with quantization compensation;
6. decodes the stego AMBTC representation;
7. prints actual bpp, PSNR, and SSIM.

### 3.3 Evaluation CSV

```bash
printf "examples/lena_like_64.pgm\n" > data/splits/test.txt
bin/gan_ppo_ambtc_eval data/splits/test.txt results/eval_smoke.csv 0.4 weights/policy_smoke.txt
```

Output columns:

```csv
image_id,bpp,psnr,ssim,embedded_bits,bit_errors
```

### 3.4 Export paper-result CSVs

```bash
bin/gan_ppo_ambtc_export_ablation
```

This writes:

```text
results/ablation_0p4bpp.csv
results/security_pe.csv
```

These values mirror the result tables specified in the paper draft and are provided for plotting/report integration.

### 3.5 Training stub

```bash
bin/gan_ppo_ambtc_train_stub configs/default.cfg
```

This does **not** train a real GAN/PPO model in C. It records the hyperparameter configuration and writes a deterministic `results/training_log.csv` for pipeline testing.

---

## 4. Method mapping to paper

| Paper component | C package file | Status |
|---|---|---|
| AMBTC 4 × 4 block coding | `src/ambtc.c` | Implemented |
| State = `(QLD, variance)` | `ambtc_block_state()` | Implemented |
| Action set `{1,2,4,8}` bits/block | `src/policy.c` | Implemented |
| PPO actor policy | `PPOPolicy` MLP loader + heuristic fallback | Inference scaffold |
| DQN reward shaping | config fields + training stub | Documented scaffold |
| U-Net generator | `GeneratorLite` interface | Lightweight C approximation |
| Double-Tanh simulator | `double_tanh()` | Implemented utility |
| Quantization compensation `(H′, L′)` | `generator_embed_block()` | Implemented lightweight approximation |
| SRM-SRNet discriminator | result CSV / training interface placeholder | Not trained in pure C |
| Evaluation metrics | `src/metrics.c` | PSNR / global SSIM implemented |
| Ablation/result CSVs | `src/export_ablation.c`, `results/*.csv` | Included |
| Trained weights | `weights/policy_smoke.txt` | Smoke-test only |

---

## 5. Hyperparameters included

`configs/default.cfg` includes the core settings reported in the paper:

```text
block_size=4
epochs=200
target_bpp=0.4
ppo_clip_eps=0.2
gae_lambda=0.95
gamma=0.99
lr_actor=0.0003
lr_critic=0.0003
lr_generator=0.0001
lr_discriminator=0.0004
d_updates_per_g=4
dqn_replay_capacity=10000
dqn_tau=0.005
lambda_payload=1.0
lambda_distortion=0.4
lambda_security=0.6
lambda_budget=0.1
```

---

## 6. Image format

The C I/O module currently supports binary grayscale PGM format `P5`.

To convert PNG/JPG datasets to PGM, use ImageMagick:

```bash
magick input.png -colorspace Gray output.pgm
```

For 512 × 512 datasets:

```bash
magick input.png -resize 512x512! -colorspace Gray output.pgm
```

---

## 7. Weight export format

`weights/policy_smoke.txt` uses plain text floats in this order:

```text
w1: 2 × 32 floats
b1: 32 floats
w2: 32 × 4 floats
b2: 4 floats
```

The policy takes normalized `(QLD, variance)` and outputs logits over 4 actions. The action index maps to:

```text
0 → 1 bit/block
1 → 2 bits/block
2 → 4 bits/block
3 → 8 bits/block
```

---

## 8. Reproduction boundary

This package is honest about what can and cannot be reproduced in pure C:

- Implemented: AMBTC pipeline, block state extraction, policy inference interface, lightweight payload allocation, quantization compensation, PSNR/SSIM evaluation, split generation, CSV output.
- Not included as real trained assets: paper-quality PPO/DQN/U-Net/SRM-SRNet weights.
- Reason: paper-quality trained weights require access to BOSSbase/BOWS-2 and substantial GPU training. Providing fake trained weights would make the project misleading.

Recommended workflow:

1. train the model in PyTorch using the Python project version;
2. export the actor MLP weights to `weights/policy.txt`;
3. optionally export generator weights to a C inference backend;
4. run this C package for AMBTC-domain inference/evaluation.

---

## 9. Citation note

The implementation follows the uploaded manuscript’s method description: AMBTC 4 × 4 block representation, state `(QLD, variance)`, action set `{1,2,4,8}`, PPO clipping `ε = 0.2`, DQN dynamic reward shaping, U-Net quantization compensation, Double-Tanh embedding simulator, SRM-SRNet discriminator, and evaluation at 0.1 / 0.2 / 0.4 bpp.
