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
    FILE *fp = fopen("data", "w+");
    printf("%ld\n", 2l * 1024 * 1024 * 1024);
    int zeros[1024] = {0};
    unsigned int i = 0;
    for (i = 0; i < (3000000); i++)
        fwrite(zeros, sizeof(zeros), 1, fp);
    printf("unsigned size: %d\n", sizeof(2u));
    return 0;
}
