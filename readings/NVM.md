[TOC]

<title>NVM 调研报告</title>
<author>刘丰源 2017011313</author>

[maketitle]

# NVM 概要

## NVM 简介

NVM(Non-volatile memory) 是一种新兴的非易失性存储器。首个 NVM 设备 Optane 由 Intel 和 Micron 联合开发，采用 3D XPoint 介质技术。其有点有：非易失、性能高（接近 DRAM）、存储密度高、能耗低、按字节读取、可由用户程序直接读写（direct access）等。

尽管优点很多，但是它也有一些不足，比如：读写速度不对称（读远快于写）、寿命有限（需要像 ssd 一样考虑磨损均衡）。

## NVM 的定位

NVM 同时具有 DRAM 和 SSD 的特性，是一种介于二者之间的存储设备。其具有与 DRAM 相同的访问方式（按字节存取）和相近的性能（约十分之一），同时又具非易失性（断电后数据不会丢失）。

目前已经可以量产的 NVM 设备 Optane 具有两种工作模式：App Direct Mode 和 Memory Mode 。两种不同的使用形态，可以用于不同的场景。

- **作为高速硬盘**

相比于普通的 SSD ，NVM 具有更高的读写性能，而且无需对数据进行擦除，可以直接写覆盖。对于需要读写大规模数据的程序（如数据库）有极大性能提升。尽管提供了按字节读写的 API ，但是如果真的按字节读写并不能很好的发挥 NVM 的性能优势。这是由于实际上每次都是 64 字节读写（历史遗留问题），最佳是 256 字节读写（因为有 256 字节的缓冲区）。该模式下的 DAX 特性使得对 NVM 的读写可以绕过操作系统，进一步提高了性能和一致性保证。

- **作为扩展内存**

无论是性能还是使用方式，Optane 都与 DRAM 接近，但是 Optane 的价格却比 DRAM 低得多。与 DRAM 混合使用可以在损失极少性能的同时，极大的增加内存容量。动态调整对 NVM 和 DRAM 的访问频率还可以有效回避 NVM 寿命较短的问题。当选择内存模式时，控制器会定期以加密方式擦除 NVM 的数据，从而模仿 DRAM 的易失性。

目前有两个较为实用的拓展方法：

1. 将 NVM 作为 Linux 的交换分区，4.11 以上的内核版本可以更好的发挥 NVM 的性能（此版本以下的内核的交换分区基于 HDD 设计，对性能的考量较少）。
2. 在物理机和操作系统之间增加一层虚拟机，将 NVM 和 DRAM 虚拟成大容量内存，对操作系统透明，对内核版本无要求。虚拟化后，NVM 作为 DRAM 的缓存，由虚拟机管理其分配和释放。

# NVM 编程

NVM 设备具有很多新的硬件特性，基于 NVM 的软件编程开始逐渐兴起，但是随之而来的还有相应的问题。比如如何更好的利用 NVM 的性能和非易失性、如何保证数据的一致、如何降低编程难度增加易用性等。为此，以下解决方案应运而生。

## 新的硬件指令

现代计算机为了提高性能、增加容量，采用了多级 CPU 缓存（SRAM）和层次存储结构。尽管 CPU 的一致性协议保证了数据在内存中的更新顺序，但是常规的用于保证一致性的指令（X86）：mfence、clflush 都不能在 NVM 上很好的工作。mfence 指令构造的内存屏障只能保证在屏障前和屏障后的内存操作有序，并不能保证数据何时被写入内存中（稍后详细说明）；而 clfush 要求执行时必须严格有序，并且包含不必要的清空缓存的操作，大大降低了 CPU 的性能，得不偿失。

为此，Intel 为 NVM 专门提供了新的指令：

- CLFLUSHOPT：功能与 clflush 相似，但并行度更高，可以乱序执行。
- CLWB：功能与 clflush 相似，但是不会使缓存失效，而是将其状态改为“未修改”。
- PCOMMIT（已弃用）：强制将数据刷写到 NVM 设备中。

编程模型如下：

```asm
MOV X1, 10
MOV X2, 20
...
MOV R1, X1  ; Stores to X1 and X2 are globally visible, but potentially volatile
...
CLWB X1
CLWB X2
SFENCE
PCOMMIT
SFENCE
```

![](./pic/store2NVM_path.jpg)

如上图所示，第一个 sfence 只能保证数据被刷入内存子系统，不能保证被写入 NVM 。如果此时发生断电等 crash 情况，仍可能会导致数据不一致。后面的 PCOMMIT 和 SFENCE 用于确保 WPQ(内存子系统的写入挂起队列) 中的数据被刷入 NVM 中。

如此看来 PCOMMIT 是十分有必要的，但是为什么该指令后来被废弃了呢？这需要提到一个新的技术：ADR(异步 DRAM 刷新) 。这是一种平台级特性，自身断电时，会通过电容的电量通知其它系统组件即将断电，从而将内存子系统中的 WPQ 刷入 NVM 。

最早设计持久性内存编程模型时，开发人员担心 ADR 是一个难以获得的平台特性，因此添加了 PCOMMIT 指令，以确保在未配备 ADR 的设备上实现持久性。 最终，计划支持英特尔 DIMM 的平台也计划支持 ADR，便不再需要 PCOMMIT。 因此，推出了更简单的单个编程模型（将原模型的 PCOMMIT 和其后面的 SFENCE 去掉即可）。 出于上述原因，PCOMMIT 被弃用，不会再有软件支持它（opcode 生成无效异常）。

## 工具库

为了能够更好的应用 NVM 的各种特性，减小编程难度，各种工具库应运而生，这里介绍官方提供的工具库 NVML 和第三方制作的 NVthreads(Publication:EuroSys '17: Proceedings of the Twelfth European Conference on Computer Systems, April 2017, Pages 468–482, https://doi.org/10.1145/3064176.3064204) 。

### NVML

NVML(NON-VOLATILE MEMORY LIBRARY)$^{[2]}$ 是 Intel 提供的一个用于操作 NVM 的工具库，也叫 PMDK(Persistent Memory Development Kit) ，使用了 DAX 访问特性和可持久性。用户态程序可以使用标准 API 直接操作 NVM ；也可以将 NVM 设备视为一个文件，通过 mmap 映射到内存空间中。NVML 还提供了类似 libc 中 malloc、free 等函数，便于对内存进行管理。无论是何种操作，都会转换为对 NVM 的 load 和 store 操作，中间不存在 page cache ，这是 DAX 模式文件系统和普通文件系统最大的区别之一。

NVML 中最基础（底层）的库是 libpmem ，包含了以下四类操作：

1. 基本操作：对文件和内存进行 map/unmap 、把数据刷到文件中。
2. flush 操作：把数据从 cpu cache 刷到内存中。
3. copy 操作：把数据从普通内存拷贝到 NVM 中。
4. check 操作：检察设备是否为 NVM 。

更多与 pmdk 相关的源码分析和学习笔记位于 $pmdk/*$ 。

### NVthreads

官方提供的 NVML 功能齐全，提供了新的“事务”编程模型，但是入门门槛高，使用难度高，而 NVthreads$^{[8]}$ 完美的解决了这个问题，其使用极其简便，支持多线程编程，最小化了从传统磁盘到 NVM 的移植成本。以下是它的编程模型：

```cpp
if (crashed()) {
    nvrecover(lables, size, 'lables');
} else {
    lables = (type*)nvmalloc(size, 'lables');
    ...
    nvcheckpoint();
}
```

程序崩溃后，再次运行，会恢复到最后一个 nvcheckpoint 继续执行。

其中使用的 pthreads 是被专门修改过的，在同步互斥操作上（mutex: lock/unlock, condvar: wait, signal）做了特殊处理，使得其能够根据临界区自动推断崩溃一致性的原子区。

对于线程共享的 NVM 部分，每个线程会有将其在内存中拷贝一份副本，以及一份主副本。进行修改时，该线程会先修改自己副本，让后将这部分内容持久化到 NVM 上。然后再将自己的副本的更新部分合并到主副本，最后将主副本持久化到 NVM 上。

# NVM 性能测试

为了更好的理解 NVM 的特性、加深对 NVM 的理解，我向 [SMARTX 科技公司](https://www.smartx.com) 借了一台搭载了 Optane 设备的机器，通过 ssh 使用。我先对 NVM 的作为裸盘和其通过文件系统（ext4-dax）读写的性能进行了简单的测试，然后对 SMARTX 公司提供的文件系统进行修改，使得其能通过 NVML 对 NVM 进行读写，对修改前后的读写性能进行了简单的分析。

## 裸盘与文件系统

- **设备配置**

```shell
# 裸盘
ndctl create-namespace --type=pmem --mode=devdax --region=X [--align=4k]
ls /dev/dax*

# 文件系统
ndctl create-namespace --type=pmem --mode=fsdax --region=X [--align=4k]
mkfs.ext4 /dev/pmemX # 需要选择支持 dax 的文件系统（ext4、xfs）
mkdir /mnt/pmem
mount /dev/pmemX /mnt/pmem -o dax
ls /mnt/pmem
```

通过 fio 对 Optane 进行压力测试，读写块大小从 4K 至 256K 。原始输入输出位于 fio/bare 和 fio/ext4 目录下，数据整理位于 fio/report.md 。以下针对 4K 块的读写进行分析。

![](./pic/latency_cmp.png)

随机读写时，无论是裸盘还是 ext4 ，写延迟都远大于读延迟。顺序读写时，裸盘的延迟有明显降低，而 ext4 变化不明显，可能是由于文件系统的缓存发挥了较好的效果。

![](./pic/bytewidth_cmp.png)

通过上图可以明显看出 NVM 的读写不对称性，随机读的带宽约为随机写的两倍。ext4 性能略优于裸盘，可能是读写块较小，文件系统缓存再次发挥了较好的效果。

## libc 与 libpmem

SMARTX 提供的文件系统通过 libc 对硬盘进行读写，我将其部分修改为通过 NVML(libpmem) 对磁盘进行读写。为了更好的说明数据的相对大小，我把 Hybrid DRAM/NVM 作为内存（Memory Mode），挂载成磁盘，测试了该文件系统通过 libc 对内存的读写性能。

![](./pic/time_cmp.png)

可以看出，无论何种形式，随机读的性能都远优于随机写。三者的读性能接近，而 NVM(libc) 的写性能比预料的低得多，这可能是由于提供的文件系统的特性导致的。该文件系统基于分布式存储技术，且对容灾能力有极高的要求，可能是为了提高数据的可靠性和一致性，写的时候记录、拷贝了额外的数据，导致性能损失。

## 结论

尽管实验数据有一些抖动，但是与理论数据基本吻合。对于 NVM 的实际应用，还需要进一步探索。比如在编写文件系统时，其缓存对于读写性能的优化十分明显，ext4 的读写性能就明显优于 SMARTX 提供的文件系统。尽管二者面向的用户不同，标准和要求不同，不完全具有可比性，但是其数据也值得思考。

# NVM 应用

正如前面所看到的性能分析，显然，软件针对 NVM 设备的特性优化是十分有必要的。合理的利用 NVM 的特性可以大幅增加软件的性能，这给编程人员带来了新的机遇和挑战。

## NOVA 文件系统

### 概述

NOVA(NOn-Volatile memory Accelerated log-structured file system) 是一个基于 Volatile/Non-volatile Main Memories 混合架构的文件系统。本小节会将 NOVA 与传统 LFS 进行对比，分析 NOVA 做出了哪些改变，这么做的优势是什么。最后针对部分实现细节进行具体分析，并给出总结和思考。

### 与传统 LFS 比较

- **log 基于 inode**

传统 LFS 都是基于磁盘设计的。受制于物理上的寻道时间，磁盘的读写延迟难以低于 10ms ，因此才把整个磁盘看做一个 append only log ，永远都是顺序写入。每当我们写入新文件时，总是顺序地追加在 log 的最后，对文件的更改并不修改现有内容，而是把增量追加在硬盘的最后。

NVM 的随机读写能力与顺序读写能力接近，因此不再使用 append only log ，而是为每个 inode 独立创建一份 log 。因此允许同时对多个文件并发更新，避免了传统 LFS 对 log 锁的征用，充分发挥了现代多核 CPU 的性能。在恢复阶段同理，可以同时恢复多个文件。除此之外，NOVA 还会周期性清理无效 log ，加快对 log 的扫描速度。

- **不在 log 中记录数据**

NOVA 采用了 COW(copy on write) 机制修改数据页（与 OS fork 中的 COW 完全不是一回事），即当需要对数据页进行写操作时，并不修改原始数据，而是将原始数据进行拷贝，修改拷贝后的数据，然后将文件指针由旧文件改为新文件。因此在 NOVA 中，inode 的 log 不保存任何文件数据，极大的缩小了日志的大小。

相比于在传统磁盘中使用 COW ，在 NVM 中使用磁盘的优势尤为明显。传统磁盘的顺序读写能力远强于随机读写。COW 会导致文件数据随机分散，因此采用 COW 的文件系统对大文件的读写性能极差。而 NVM 随机读写能力强，同时在 NOVA 的支持下，可以并发对文件进行读写，因此对数据分散的大文件也有着较好的的写性能。

- **在 DRAM 中维护复杂的数据结构**

由于 NOVA 在 NVM 设备上保存 log 和文件数据，并且在 DRAM 中构建 radix tree 来加速检索和查询操作，radix tree 的叶子结点为 log 中的一条记录，这条记录中会保存指向实际数据的指针。

这样做的原因是，NOVA 的作者认为在 NVM 中维护树形数据结构十分影响性能$^{[3]}$。但事实并非如此，这是因为**他们读了过时的文献！**他们并没有通过实验验证 NVM 中树形数据结构的性能，而是直接引用了其他人论文的结论。我阅读了他们引用的四篇论文，那四篇论文都认为：为了在 NVM 中保证数据的一致性，反复调用了 clflush 和 mfence ，这个问题在树形数据结构中尤为明显，从而导致性能较差$^{[4,5,6,7]}$。**但是！**这四篇论文的发表时间都在 2015 年及以前，而在 2016 年，Intel 专门开发了 CLFLUSHOPT 和 CLWB 硬件指令以解决该问题，而且 NOVA 的实现中也用到了这两条指令。这也许时由于他们只读了论文的结论而没有读原因导致的问题。不过结合 NOVA 其它特性，将复杂的数据结构放于 DRAM 中维护，仍然是一个很棒的思路，因为对 DRAM 操作的确是比对 NVM 操作快一些。关于在 DRAM 中维护带来的易失性问题，如果 DRAM 正常退出，数据结构信息可以被写到 NVM 当中；而如果 DRAM 发生意外掉电导致数据结构信息丢失，我们可以根据文件重新构建 B 树。

### 原子性和顺序一致性保证

- **原子性**

现代 CPU 提供 64-bit 原子更新功能修改 DRAM ，也可以通过相同的方式修改 NVM 。该机制保证了提交 inode log 操作，即更新 inode log tail 指针的原子性。由于每个 inode 只记录自己的 log ，因此 write、msync、chmod 等操作的原子性得到了保证。

在 NOVA 中，主要通过 journal 来保证跨多个 inode 访问的操作是原子的。journal 是一个 4KB 大小的 circular buffer ，NOVA 通过指针 <enqueue, dequeue> 来管理每一个 journal。

当发生跨多个 inode 的操作时，NOVA 首先在此次受影响的 inode log 中添加对应的 log 记录；然后把每个受影响的 inode log 的当前 tail 指针追加到对应的 journal ，更新其 enqueue 指针为 tail 指针；最后，在所有受影响的 inode log 写入完成，且更新 tail 指针后，NOVA 更新 dequeue 的指针等于 enqueue 并完成此次操作。

- **顺序一致性**

NOVA 充分利用了 Intel 为 NVM 设备提供的新指令来解决写入顺序一致性的问题。例如在更新 inode log 的 tail 指针前，必须要先把日志记录在 inode log 中，即写入 NVM 设备。

```c
new_tail = append_to_log(inode->tail, entry);
// writes back the log entry cachelines
clwb(inode->tail, entry->length);
sfence(); 			// orders subsequent PCOMMIT
PCOMMIT();			// commits entry to NVMM
sfence();			// orders subsequent stor
inode->tail = new_tail;
```

如上述代码所示，NOVA 通过 clwb、PCOMMIT 和 sfence 来实现对 Inode log 写入的原子性，如果平台不支持上述新指令，NOVA 会采用 movntq 非时序移动指令绕过 CPU 高速缓存层以便直接写 NVM，并且结合 clflush 和 sfence 确保写顺序性。

除此之外，在进行跨多个 inode 操作时，要先保证 journal 中的数据已经写入 NVM 设备，然后才能提交事务；在通过 COW 机制更新数据时，必须先提交新修改的 data page，然后再回收过期的 data page。

# 参考资料

1. Intel 官网. https://software.intel.com/persistent-memory
2. Persistent Memory Development Kit. https://pmem.io/pmdk
3. Jian Xu and Steven Swanson. NOVA: A Log-structured File System for Hybrid Volatile/Non-volatile Main Memories. Fast 2016.
4. S. Chen and Q. Jin. Persistent B+-trees in Non-volatile Main Memory. Proc. VLDB Endow., 8(7):786–797, Feb. 2015.
5. I. Moraru, D. G. Andersen, M. Kaminsky, N. Tolia, P. Ran- ganathan, and N. Binkert. Consistent, Durable, and Safe Mem- ory Management for Byte-addressable Non Volatile Main Memory. In Proceedings of the First ACM SIGOPS Confer- ence on Timely Results in Operating Systems, TRIOS ’13, pages 1:1–1:17, New York, NY, USA, 2013. ACM.
6. S. Venkataraman, N. Tolia, P. Ranganathan, and R. Campbell. Consistent and durable data structures for non-volatile byte- addressable memory. In Proceedings of the 9th USENIX Conference on File and Storage Technologies, FAST ’11, San Jose, CA, USA, February 2011.
7. J. Yang, Q. Wei, C. Chen, C. Wang, K. L. Yong, and B. He. NV-Tree: Reducing Consistency Cost for NVM-based Single Level Systems. In 13th USENIX Conference on File and Storage Technologies, FAST ’15, pages 167–181, Santa Clara, CA, Feb. 2015. USENIX Association.
8. NVthreads: Practical Persistence for Multi-threaded Applications
