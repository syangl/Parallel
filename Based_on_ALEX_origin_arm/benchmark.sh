#on x86 by removing ‘qemu-aarch64’
qemu-aarch64 ./build/benchmark --keys_file=/root/Labo/lognormal_200M_uint64.bin --keys_file_type=binary --init_num_keys=3000 --total_num_keys=10000 --batch_size=2500 --insert_frac=0.3 --print_batch_stats
