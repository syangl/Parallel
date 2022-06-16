pssh -h $PBS_NODEFILE mkdir -p /home/s2013458/mpi # 这一步是在集群的所有节点的相同路径下创建文件夹
pscp.pssh -h $PBS_NODEFILE /home/s2013458/mpi/mpi-only /home/s2013458/mpi/ #这一步是把要执行的程序发发送到各个节点上
