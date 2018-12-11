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
    printf("%d\n", -1 / 1024);
    UfsInit("ufs");
    char s[100] = {0};
    int fd[100];
    int fd1 = UfsOpen("/", 0);
    int fd2 = UfsOpen("/abc", UO_CREAT);
    int fd3 = UfsOpen("/bbc", UO_CREAT);
    printf("%d %d %d\n", fd1, fd2, fd3);
	
	char buf[11] = { 0 };
	//UfsWrite(fd2, buf, sizeof(buf));
	//UfsClose(fd2);
	//fd2 = UfsOpen("/abc", UO_CREAT);
	UfsRead(fd2, buf, sizeof(buf));

	UfsClose(-1);
    return 0;
}
