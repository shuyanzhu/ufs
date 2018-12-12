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
#define NOMOREBLK -5
#define NOMOREINODE -6
#define BADFILENAME -7
// 磁盘大小宏
#define BLKSIZE (1 << 10)
#define BLKSOFSU 2
#define BLKSOFIN (1 << 14)
#define ITABLEBGN BLKSOFSU
#define DATABGN (BLKSOFSU + BLKSOFIN)
// 超级块宏
#define UFSSIZE (1 * 1024 * 1024 * 1024)
#define UFSMAGIC 199810109
#define FREEINUM 249
#define FREEBNUM 256
// 索引节点宏
#define INODESIZE (1 << 6)
#define BLKADDR 13   // 索引节点中直接块和间接快的数目
#define MINODES 1024 // 索引节点表大小
#define ITABLESEEK (ITABLEBGN * BLKSIZE)
#define ROOTISEEK (ITABLESEEK + INODESIZE)
// 文件创建宏
#define TRUNC (1 << 3)
#define CREAT (1 << 4)
#define DIRTYPE 1
#define FILETYPE 2

//
#define RDDIRNUM (BLKSIZE / sizeof(struct Dir))
//
typedef long long int64;

// 超级块数据结构
struct SuperBlk
{
    int magic;    // 魔数
    int diskSize; // 磁盘大小
    int inodeNum; // 剩余索引节点
    int blkNum;   // 剩余磁盘块
    int dirty;    // 脏位
    // 空闲索引节点表
    int nextN;
    int freeInode[FREEINUM];
    // 空闲快表:sp
    int nextB;
    int freeBlk[FREEBNUM];
};
// 磁盘索引节点
struct DInode
{
    int type;                      // 文件类型
    int fSize;            // 文件大小
    int lNum;             // 文件连接数目
    int blkAddr[BLKADDR]; // 文件块位置
};
// 目录结构, 断言长度为32
struct Dir
{
    char name[28];     // 路径名分量至多为27
    int iNbr; // 磁盘索引节点号
};

// 内存索引节点
struct MInode
{
    struct DInode *Dp;
    int oflag;  // 打开方式
    int iNbr;   // 磁盘索引节点号
    int pos;    // 文件偏移量
    int pading; // 以备扩展
};
//初始化文件系统
int Init(char *path);
// 创建文件、解除文件，分配/释放磁盘块和索引节点等
int CreatFile(char *path);
int AllocBlk();
int FreeBlk(int blkNbr);
int AllocI();
int FreeI(int iNbr);

// 在已经打开的文件中找
int FindOpenedI(int iNbr);
// 获得文件描述符
int FindNextMInode(int iNbr);

// 查找目录项
int FindDirent(char *path, struct DInode *rootI, struct Dir dirs[RDDIRNUM], int *i);
// 将文件名转为索引节点号
int NameI(int *iNum, char *path, int oflag);
// 将逻辑块号偏移量转换为磁盘块号
int BMap(int pos, struct DInode i);
int BRead(int pos, struct DInode *inode);
int BAlloc(int pos, struct DInode *inode);
int BFree(int pos, struct DInode *inode);


#endif
