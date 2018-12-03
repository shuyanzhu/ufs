/*************************************************************************
    > File Name: ufs_struct.h
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 11时52分08秒
 ************************************************************************/

#ifndef __NDSL_UFS_STRUCT_H__
#define __NDSL_UFS_STRUCT_H__

// 错误宏
#define FWERR -1
#define FSERR -2
#define FRERR -3

// 磁盘大小宏
#define BLKSIZE (1u << 10)
#define BLKSOFSU 2
#define BLKSOFIN (1u << 14)
#define ITABLEBGN BLKSOFSU
#define DATABGN (BLKSOFSU + BLKSOFIN)

// 超级块宏
#define UFSSIZE (2 * 1024 * 1024 * 1024)
#define UFSMAGIC 19981019
#define FREEINUM 249
#define FREEBNUM 256

// 索引节点宏
#define INODESIZE (1u << 6)
#define BLKADDR 13 // 索引节点中直接块和间接快的数目

struct SuperBlk
{
    // 魔数，标示文件系统
    unsigned int magic;
    // 磁盘大小
    unsigned int diskSize;
    // 剩余空闲索引节点数目
    unsigned int inodeNum;
    // 剩余磁盘块数目
    unsigned int blkNum;
    // 超级快是否被修改
    unsigned int dirty;

    // 空闲索引节点表
    unsigned int freeInode[FREEINUM];

    // 空闲快表:sp
    unsigned int curBlk;
    unsigned int pNext;
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

int Init(char *path);
#endif
