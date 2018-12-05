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
    }
    //////////////////
    printf("%u\n", rootI.lNum);
    return 1;
    ////////////////
}
