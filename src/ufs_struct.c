/*************************************************************************
    > File Name: ufs_struct.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 16时50分16秒
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/ufs_struct.h"

static void _quit(char *err)
{
    fprintf(stderr, "%s\n", err);
    exit(0);
}

struct SuperBlk super;
int Init(char *path)
{
    FILE *fp = fopen(path, "w+");
    setbuf(fp, NULL);

    if (fseek(fp, UFSSIZE - 1, SEEK_SET) < 0) return FSERR;
    unsigned char i = 0;
    if (fwrite(&i, 1, 1, fp) != 1) return FWERR;
    if (fseek(fp, 0, SEEK_SET) < 0) return FSERR;

    // 初始化超级快
    super.magic = UFSMAGIC;
    super.diskSize = UFSSIZE;
    super.inodeNum = 1u << (24 - 6);
    super.blkNum = UFSSIZE / BLKSIZE - DATABGN;
    super.dirty = 0;

    // 初始化索引节点列表
    unsigned char zeros[4096] = {0};
    if (fseek(fp, ITABLEBGN * BLKSIZE, SEEK_SET) < 0) return FSERR;
    int i = 0;
    for (i = 0; i < BLKSOFIN / 4; i++)
        if (fwrite(zeros, sizeof(zeros), 1, fp) != 1) return FWERR;

    struct DInode rooti; // 磁盘索引节点的根节点
    rooti.type = 1;
    rooti.fSize = 0;
    rooti.iNum = 1;
    memset(&rooti.blkAddr[0], 0, BLKADDR);
    if (fseek(fp, ITABLEBGN * BLKSIZE, SEEK_SET) < 0) return FSERR;
    if (fwrite(&rooti, sizeof(struct DInode), 1, fp) != 1)
        return FWERR

               // 初始化磁盘块
               int i = DATABGN,
                   j = 0;
    unsigned int fBlk[FREEBNUM];
    for (i = DATABGN + FREEBNUM - 1; i < UFSSIZE / BLKSIZE; i = i + FREEBNUM) {
        for (int j = 0; j < (FREEBNUM - 2); j++)
            fBlk[j] = i - (FREEBNUM - 1) + j;
        fBlk[FREEBNUM - 1] = i + FREEBNUM;
        if (fseek(fp, i * BLKSIZE, SEEK_SET) < 0) return FSERR;
        if (fwrite(fBlk, sizeof(fBlk), 1, fp) != 1) return FWERR;
    }

    // 写回超级块，INIT成功返回
    super.blkNum -= FREEBNUM;
    if (fseek(fp, (DATABGN + FREEBNUM - 1) * BLKSIZE, SEEK_SET) < 0)
        return FSERR;
    if (fread(super.freeBlk, sizeof(super.freeBlk), 1, fp) != 1) return FRERR;
    if (fseek(fp, 0, SEEK_SET) < 0) return FSERR;
    if (fwrite(&super, sizeof(super), 1, fp) != 1) return FSERR;

    return 0;
}
