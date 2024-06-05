# 基于 NVM 的动态图处理系统 FTGraph
## 1. 数据集
- Orkut：无向图，下载链接：https://snap.stanford.edu/data/com-Orkut.html，下载 “com-orkut.ungraph.txt.gz” 文件，解压得到 “com-orkut.ungraph.txt”。
- Twitter：有向图，下载链接：https://github.com/ANLAB-KAIST/traces/releases/tag/twitter_rv.net，下载 4 个 gz 文件，通过 gunzip 合并得到 “twitter_rv.net”
- graph500-25：无向图，下载链接：https://ldbcouncil.org/benchmarks/graphalytics/，下载 “graph500-25.tar.zst” 文件，解压后使用目录里面的 “graph500-25.e” 文件
- graph500-26：无向图，下载链接：https://ldbcouncil.org/benchmarks/graphalytics/，下载 “graph500-26.tar.zst” 文件，解压后使用目录里面的 “graph500-26.e” 文件

## 2. 编译与运行
apps/config.h 是配置文件，可以配置数据集的目录地址 “edge_file_directory” 等。
进入 build/ 目录，执行：
```
cmake..
make
numactl --cpubind=1 ./graph_benchmark
```

如果创建的 mmap 映射文件在 /mnt/pmem0，上面的 numactl 就需要绑定 CPU0，即 “numactl --cpubind=1 ./graph_benchmark”，默认创建的 mmap 映射文件在 build/ 目录下.

在 apps/config.h 通过取消或添加注释来执行和关闭算法的运行：
```
// #define ONE_HOP
// #define TWO_HOP
// #define BFS
// #define SSSP
// #define CC
// #define PR
```