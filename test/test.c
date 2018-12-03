/*************************************************************************
    > File Name: test.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 11时43分19秒
 ************************************************************************/

#include <stdio.h>

struct
{
    char p[10];
} i;
int main()
{
    printf("%ld\n", 2l * 1024 * 1024 * 1024);
    int zeros[1024];
    printf("unsigned size: %d\n", sizeof(2u));
    return 0;
}
