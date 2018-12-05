/*************************************************************************
    > File Name: ufs_struct.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 16时50分16秒
 ***********************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/ufs_wrap.h"
#include "../include/ufs_struct.h"

struct SuperBlk super;
FILE *ufsFp;
struct MInode mInodes[MINODES];
int maxUfd = 0;

int FindNextMInode(unsigned int iNbr)
{
    int i = 0;
    // 查询是否打开过
    for (i = 0; (i < maxUfd + 1) && (i != MINODES); i++)
        if (mInodes[i].iNbr == iNbr) return i;

    // 寻找空位
    for (i = 0; (i < maxUfd + 1) && (i != MINODES); i++)
        if (mInodes[i].Dp == NULL) break;
    if (i == maxUfd)
        if (maxUfd == MINODES)
            return -1;
        else
            return maxUfd++;
    else
        return i;
}

int NameI(unsigned int *iNum, char *path, int oflag)
{
    if (path[0] != '/') return -1;
    struct DInode rootI;
    Fseek(ufsFp, ROOTISEEK, SEEK_SET);
    Fread(&rootI, sizeof(struct DInode), 1, ufsFp);
    Assert(rootI.type & 1);
    if (path[1] == 0)
        return 1;
    else {
        path = path + 1;
        if (rootI.fSize == 0) return -1;
        unsigned bAddr = (rootI.fSize - 1) / BLKSIZE + 1;
        struct Dir dirs[RDDIRNUM];
        unsigned int i = 0;
        for (i = 0; i < bAddr; i++) {
            Fseek(ufsFp, _bmap(i, rootI), SEEK_SET);
            ///////////////////////////////
            Assert(sizeof(dirs) == BLKSIZE);
            ///////////////////////////////
            Fread(dirs, sizeof(dirs), 1, ufsFp);
            int j = 0;
            for (int j = 0;
                 (i * BLKSIZE + j * sizeof(struct Dir) < rootI.fSize) &&
                 (j < RDDIRNUM);
                 j++)
                if (strcmp(path, (char *) &dirs[j]) == 0) return dirs[j].iNbr;
        } // 循环结束，未能找到和path相匹配的目录项
        return -1;
    }
    //////////////////
    printf("%u\n", rootI.lNum);
    return 1;
    ////////////////
}
long _bmap(int64 pos, struct DInode i)
{
    unsigned int blk[BLKSIZE / 4];
    if (pos < 10)
        return i.blkAddr[pos];
    else if (pos < 10 + 256) {
        Fseek(ufsFp, i.blkAddr[10] * BLKSIZE, SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        return blk[pos - 10];
    } else if (pos < 10 + 256 + 256 * 256) {
        Fseek(ufsFp, i.blkAddr[11] * BLKSIZE, SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        Fseek(ufsFp, blk[(pos - 10 - 256) / 256], SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        return blk[(pos - 10 - 256) % 256];
    } else {
        Fseek(ufsFp, i.blkAddr[12] * BLKSIZE, SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        Fseek(ufsFp, blk[(pos - 10 - 256 - 256 * 256) / (256 * 256)], SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        Fseek(
            ufsFp,
            blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) / 256],
            SEEK_SET);
        Fread(blk, sizeof(blk), 1, ufsFp);
        return blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) % 256];
    }
}
