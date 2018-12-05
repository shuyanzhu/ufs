/*************************************************************************
    > File Name: ufs_struct.h
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 11时52分08秒
 ************************************************************************/

#ifndef __NDSL_UFS_STRUCT_H__
#define __NDSL_UFS_STRUCT_H__

// 错误宏
#define FRERR -1
#define FSERR -2
#define FWERR -3
#define FCERR -4
// 磁盘大小宏
#define BLKSIZE (1u << 10)
#define BLKSOFSU 2u
#define BLKSOFIN (1u << 14)
#define ITABLEBGN BLKSOFSU
#define DATABGN (BLKSOFSU + BLKSOFIN)
// 超级块宏
#define UFSSIZE (2u * 1024 * 1024 * 1024)
#define UFSMAGIC 19981019
#define FREEINUM 248
#define FREEBNUM 256
// 索引节点宏
#define INODESIZE (1u << 6)
#define BLKADDR 13 // 索引节点中直接块和间接快的数目
#define MINODES 1024 // 索引节点表大小

// 超级块数据结构
struct SuperBlk
{
    unsigned int magic;    // 魔数
    unsigned int diskSize; // 磁盘大小
    unsigned int inodeNum; // 剩余索引节点
    unsigned int blkNum;   // 剩余磁盘块
    unsigned int dirty;    // 脏位
    // 空闲索引节点表
    unsigned int nextN;
    unsigned int freeInode[FREEINUM];
    // 空闲快表:sp
    unsigned int curBlk;
    unsigned int nextB;
    unsigned int freeBlk[FREEBNUM];
};
// 磁盘索引节点
struct DInode
{
    int type;                      // 文件类型
    unsigned int fSize;            // 文件大小
    unsigned int lNum;             // 文件连接数目
    unsigned int blkAddr[BLKADDR]; // 文件块位置
};
// 内存索引节点
struct MInode
{
    struct DInode *Dp;
    unsigned int ref;
};

#endif
