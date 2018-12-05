/*************************************************************************
    > File Name: ufs.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 14时15分25秒
 ************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/ufs_wrap.h"
#include "../include/ufs_struct.h"
#include "../include/ufs.h"
// 全局变量
extern struct SuperBlk super;
extern struct MInode mInodes[MINODES];
extern FILE *ufsFp;
extern int maxUfd;

// 初始化磁盘块
static int _init(char *path)
{
    ufsFp = fopen(path, "w+");
    setbuf(ufsFp, NULL);
    // 初始化磁盘大小
    if (fseek(ufsFp, UFSSIZE - 1, SEEK_SET) < 0) return FSERR;
    unsigned char c = 0;
    if (fwrite(&c, 1, 1, ufsFp) != 1) return FWERR;
    if (fseek(ufsFp, 0, SEEK_SET) < 0) return FSERR;

    // 初始化超级快
    memset(&super, 0, sizeof(super));
    super.magic = UFSMAGIC;
    super.diskSize = UFSSIZE;
    super.inodeNum = 1u << (24 - 6);
    super.blkNum = UFSSIZE / BLKSIZE - DATABGN;
    super.dirty = 0;

    // 写数据区磁盘块数组
    unsigned int i, j = 0;
    unsigned int fBlk[FREEBNUM];
    for (i = DATABGN + FREEBNUM - 1; i < UFSSIZE / BLKSIZE; i = i + FREEBNUM) {
        for (int j = 0; j < (FREEBNUM - 2); j++)
            fBlk[j] = i - (FREEBNUM - 1) + j;
        fBlk[FREEBNUM - 1] = i + FREEBNUM;
        if (fseek(ufsFp, i * BLKSIZE, SEEK_SET) < 0) return FSERR;
        if (fwrite(fBlk, sizeof(fBlk), 1, ufsFp) != 1) return FWERR;
    }
    super.blkNum -= FREEBNUM;
    if (fseek(ufsFp, (DATABGN + FREEBNUM - 1) * BLKSIZE, SEEK_SET) < 0)
        return FSERR;
    if (fread(super.freeBlk, sizeof(super.freeBlk), 1, ufsFp) != 1)
        return FRERR;
    super.curBlk = DATABGN + FREEBNUM - 1;
    super.nextB = 0;

    // 写索引节点列表区磁盘数组
    unsigned char zeros[4096] = {0};
    if (fseek(ufsFp, ITABLEBGN * BLKSIZE, SEEK_SET) < 0) return FSERR;
    for (i = 0; i < BLKSOFIN / 4; i++)
        if (fwrite(zeros, sizeof(zeros), 1, ufsFp) != 1) return FWERR;
    struct DInode rootI; // 磁盘索引节点的根节点
    rootI.type = 1;
    rootI.fSize = 0;
    rootI.lNum = 15;
    memset(&rootI.blkAddr[0], 0, BLKADDR);
    if (fseek(ufsFp, ITABLEBGN * BLKSIZE + sizeof(struct DInode), SEEK_SET) < 0)
        return FSERR;
    if (fwrite(&rootI, sizeof(struct DInode), 1, ufsFp) != 1) return FWERR;
    for (i = 0; i < FREEINUM; i++)
        super.freeInode[i] = i + 2;

    // 写超级块
    if (fseek(ufsFp, 0, SEEK_SET) < 0) return FSERR;
    if (fwrite(&super, sizeof(super), 1, ufsFp) != 1) return FSERR;

    return 0;
}

int UfsInit(char *path)
{
    ufsFp = Fopen(path, "a+");
    Fclose(ufsFp); // 如果文件不存在，创建文件
    ufsFp = Fopen(path, "r+");
    setbuf(ufsFp, NULL);

    // 初始化内存索引节点
    memset(mInodes, 0, sizeof(mInodes));
    maxUfd = 0;

    // 获得文件大小, 文件系统最大为2G
    if (fseek(ufsFp, 0, SEEK_END) < 0) _quit("UfsInit: 确定文件大小出错");
    long ufsLength = ftell(ufsFp); // 获取pos有移植性问题，要求x64，long=int64
    if (ufsLength < sizeof(struct SuperBlk)) {
        fclose(ufsFp);
        if (_init(path) < 0) _quit("UfsInit: 初始化磁盘块失败");
        printf("文件系统初始化成功\n");
        return 0;
    }
    if (fseek(ufsFp, 0, SEEK_SET) < 0) _quit("UfsInitL fseek failed");

    // 初始化超级块(磁盘)
    if (fread(&super, sizeof(super), 1, ufsFp) != 1)
        _quit("UfsInit: fread failed");
    if (super.magic != UFSMAGIC) {
        fclose(ufsFp);
        printf("无效的磁盘\n新的磁盘生成中\n");
        if (_init(path) < 0) _quit("UfsInit: 初始化磁盘块失败");
        printf("文件系统初始化成功\n");
        return 0;
    }

    printf("文件系统初始化成功\n");
    return 0;
}

int UfsOpen(char *path, int oflag)
{
    unsigned int iNum = 0;
    if (NameI(&iNum, path, oflag) < 0) return NOTHATFL;
    int ufd = FindNextMInode(iNum);
    if (ufd < 0) return NOMOREFD;

    struct DInode *Dp = malloc(sizeof(struct DInode));
    mInodes[ufd].Dp = Dp;
    Fseek(ufsFp, ITABLESEEK + iNum * INODESIZE, SEEK_SET);
    Fread(Dp, sizeof(struct DInode), 1, ufsFp);
    return ufd;
}

int UfsClose(int ufd) {}

