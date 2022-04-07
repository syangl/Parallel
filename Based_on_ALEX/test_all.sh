#on x86 by removing ‘qemu-aarch64’
qemu-aarch64 ./build/test_all --keys_file=/root/Labo/uniform_aran_200M_int --keys_file_type=binary --init_num_keys=1000000 --total_num_keys=2000000
