/*************************************************************************
    > File Name: ufs.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 14时15分25秒
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "../include/ufs_struct.h"
#include "../include/ufs.h"

#ifndef NOTEST

#endif

static void _quit(char *err)
{
    fprintf(stderr, "%s\n", err);
    exit(0);
}

static void _init(char *path)
{
    int i = Init(path);
    if (i < 0) _quit("_Init: 文件系统初始化失败");
    printf("磁盘初始化成功\n");
}

extern struct SuperBlk super;
int UfsInit(char *path)
{
    FILE *ufsFp = fopen(path, "r+");
    if (ufsFp == NULL) _quit("UfsInit: 文件不存在");
    setbuf(ufsFp, NULL);

    // 获得文件大小
    if (fseek(ufsFp, 0, SEEK_END) < 0) _quit("UfsInit: 确定文件大小出错");
    long ufsLength = ftell(ufsFp);
    if (fseek(ufsFp, 0, SEEK_SET) < 0) _quit("UfsInitL fseek failed");
    if (ufsLength < sizeof(struct SuperBlk)) {
        fclose(ufsFp);
        _init(path);
        return 0;
    }

    // 获取超级快
    if (fread(&super, sizeof(super), 1, ufsFp) != 1)
        _quit("UfsInit: fread failed");
    fclose(ufsFp);

    if (super.magic == UFSMAGIC)
        return 9;
    else
        _init(path);
    return 0;
}
