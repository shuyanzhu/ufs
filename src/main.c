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
    int fd1 = UfsOpen("/", 0);
    int fd2 = UfsOpen("/abc", 0);
    printf("%d %d\n", fd1, fd2);
    return 0;
}
