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