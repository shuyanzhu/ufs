/*************************************************************************
> File Name: main.c
> Author: 朱会茗
> Mail: 1294206499@qq.com
> Created Time: 2018年12月03日 星期一 22时06分19秒
************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#include "../include/ufs.h"

int main()
{
    UfsInit("ufs");
    char s[100] = {0};
    printf("%d\n", -1 / 1024);

    int fd = UfsOpen("/abbc", UO_CREAT);
    int fd1 = UfsOpen("/mingming", UO_CREAT);

    // char buf[] = "1234576970757665324345657568968987566";
    // UfsWrite(fd, buf, sizeof(buf));

    char buff[100] = {0};
    if (0 == UfsRead(fd, buff, sizeof(buff)))
        printf("EOF\n");
    else
        fwrite(buff, 100, 1, stderr);

    int rfd = DirOpen();
    struct Dirent *dirent = DirRead(rfd);
    while (dirent != NULL) {
        printf("%s\n", dirent->name);
        free(dirent);
        dirent = DirRead(rfd);
    }
    UfsClose(-1);
    return 0;
}
