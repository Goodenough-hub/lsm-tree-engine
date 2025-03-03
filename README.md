# lsm-tree-engine

## day1
实现跳表skiplist：
- put
- get
- remove

## day2
skiplist迭代器
对跳表封装，memtable
- 一个可读可写的skiplist
- 一个list，frozen_tables

## day3

memtable并发控制：两把锁
- 不加锁的函数
- 大批量数据处理的函数

block编码

## day4
block & sst编码
SSTbuilder用于构建SSTable

## day5

### 内存映射
将磁盘文件的数据映射到内存中，用户通过修改内存就能修改磁盘文件。

也可以用来进程间通信。

### 相关函数
```c++
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
/*
    - 功能：将文件或者设备的数据映射到内存中
    - 参数：
        - void *adde: NULL, 由内核指定
        - length：要映射的数据的长度，这个值不能为0.建议使用文件的长度
            获取文件的长度：stat leek
        - prot：对申请的内存映射区的操作权限
            - PROT_READ：可读
            - PROT_WRITE：可写
            - PROT_EXEC：可执行
            - PROT_NONE：不可读不可写不可执行
            要操作映射内存，必须要有读的权限
            PROT_READ 、 PROT_READ | PROT_WRITE
        - flags：映射文件的方式
            - MAP_SHARED：映射到内存中的数据，对映射到内存中的数据进行修改，映射到内存中的数据也会被修改.自动同步，进程间通信，必须要设置这个选项
            - MAP_PRIVATE：映射到内存中的数据，对映射到内存中的数据进行修改，映射到内存中的数据不会被修改.不同步，内存映射区的数据改变了，对原来的文件不会修改，会重新创建一个新的文件。 
        - fd：需要映射的那个文件的文件描述符
            - open 函数获取,open的是一个磁盘文件
            - 注意：文件的大小不能为0，open指定的权限不能与prot参数有冲突.必须要小于open的权限
            prot: Prot_READ               open:只读/读写
            prot: Prot_READ | Prot_WRITE  open:读写
        - offset：映射到内存中的偏移量，必须是文件系统系统的一个整数倍，一般使用0，表示不偏移
    - 返回值：成功返回映射的内存首地址，失败返回 (void *)-1  MAP_FAILED
*/

int munmap(void *addr, size_t length);
/*
    - 功能：取消内存映射
    - 参数：
        - void *addr：要释放的内存首地址
        - size_t length：释放内存的长度，和mmap函数中的length值相同
*/


int msync(void *addr, size_t length, int flags);
/*
    - 功能：同步内存映射区到文件。将内存映射区域（通过 mmap 创建）与磁盘文件同步，确保内存中的修改持久化到物理存储，或使缓存失效以重新加载磁盘数据。
    - 参数：
        - addr：内存映射区的首地址
        - length：内存映射区的长度
        - flags：操作选项。控制同步行为的标志位。
            - MS_ASYNC：异步写入（立即返回，不保证完成）
            - MS_SYNC：同步写入（阻塞直到完成）
            - MS_INVALIDATE：使缓存失效（后续访问重新读取磁盘）
*/
```

获取文件大小
```c++
int size = lseek(fd, 0, SEEK_END);

struct stat st;
fstat(fd_, &st) == -1
cout << st.st_size << endl;
```

> cpp中reserve与resize函数的异同
> - reserve：为容器预留空间，不会改变容器的大小，只是改变容器的容量，如果空间不足，会自动扩容，不会改变容器的大小。
> - resize：为容器重新分配空间，会改变容器的大小，如果空间不足，会自动扩容，会改变容器的大小。