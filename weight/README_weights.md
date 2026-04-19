# Weights

`policy_smoke.txt` is a deterministic smoke-test MLP policy file. It is **not** a trained paper-quality checkpoint.

For paper-quality experiments, train the PPO/DQN/U-Net/SRM-SRNet pipeline on BOSSbase 1.01 + BOWS-2, export the policy MLP to this text format, and optionally replace the lightweight generator interface with exported convolution weights.
