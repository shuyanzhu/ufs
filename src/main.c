/*************************************************************************
> File Name: main.c
> Author: 朱会茗
> Mail: 1294206499@qq.com
> Created Time: 2018年12月03日 星期一 22时06分19秒
************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include "../include/ufs.h"

int main()
{
    UfsInit("ufs");
    char s[100] = {0};

    int fd = UfsOpen("/cbc", UO_CREAT);
    // char buf[] = "eureka";
    // UfsWrite(fd, buf, sizeof(buf));

    char buf[7] = {0};
    UfsRead(fd, buf, sizeof(buf));
    printf("buf: %s\n", buf);

    UfsClose(-1);
    return 0;
}
