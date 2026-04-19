CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Iinclude
LDFLAGS ?= -lm
SRC_COMMON = src/config.c src/image_io.c src/ambtc.c src/policy.c src/generator.c src/metrics.c src/csv.c
BIN_DIR = bin

all: dirs infer evaluate split train_stub export_ablation

dirs:
	mkdir -p $(BIN_DIR) results

infer: $(SRC_COMMON) src/infer.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/gan_ppo_ambtc_infer $^ $(LDFLAGS)

evaluate: $(SRC_COMMON) src/evaluate.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/gan_ppo_ambtc_eval $^ $(LDFLAGS)

split: src/split_tool.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/gan_ppo_ambtc_split $^ $(LDFLAGS)

train_stub: src/config.c src/train_stub.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/gan_ppo_ambtc_train_stub $^ $(LDFLAGS)

export_ablation: src/export_ablation.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/gan_ppo_ambtc_export_ablation $^ $(LDFLAGS)

smoke: all
	$(BIN_DIR)/gan_ppo_ambtc_infer examples/lena_like_64.pgm results/stego_64.pgm 0.4 weights/policy_smoke.txt
	printf "examples/lena_like_64.pgm\n" > data/splits/test.txt
	$(BIN_DIR)/gan_ppo_ambtc_eval data/splits/test.txt results/eval_smoke.csv 0.4 weights/policy_smoke.txt
	$(BIN_DIR)/gan_ppo_ambtc_export_ablation

clean:
	rm -rf $(BIN_DIR) results/*.pgm results/*.csv
