/*************************************************************************
    > File Name: ufs_wrap.h
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月04日 星期二 17时11分12秒
 ************************************************************************/

#ifndef __NDSL_UFS_WRAP_H__
#define __NDSL_UFS_WRAP_H__
#include <stdlib.h>
#include <stdio.h>

static inline void _quit(char *buf)
{
    fprintf(stderr, "%s\n", buf);
    exit(-1);
}

static inline int Fseek(FILE *fp, long offset, int whence)
{
    if (fseek(fp, offset, whence) < 0) _quit("Fseek failed");
    return 0;
}

static inline long Ftell(FILE *fp)
{
    long n;
    if ((n = ftell(fp)) < 0) _quit("Ftell failed");
    return n;
}

static inline FILE *Fopen(char *path, char *mode)
{
    FILE *n;
    if ((n = fopen(path, mode)) == NULL) _quit("Fopen failed");
    return n;
}

static inline size_t Fread(void *ptr, size_t size, size_t obno, FILE *fp)
{
    size_t n;
    if ((n = fread(ptr, size, obno, fp)) != obno) _quit("Fread failed");
    return n;
}

static inline size_t Fwrite(void *ptr, size_t size, size_t obno, FILE *fp)
{
    size_t n;
    if ((n = fwrite(ptr, size, obno, fp)) != obno) _quit("Fwrite failed");
    return n;
}

static inline void FSR(void *ptr, size_t size, long offset) {
	extern FILE *ufsFp;
	Fseek(ufsFp, offset, SEEK_SET);
	Fread(ptr, size, 1, ufsFp);
}
static inline void FSW(void *ptr, size_t size, long offset) {
	extern FILE *ufsFp;
	Fseek(ufsFp, offset, SEEK_SET);
	Fwrite(ptr, size, 1, ufsFp);
}

static inline int Fclose(FILE *fp)
{
    if (EOF == fclose(fp)) _quit("Fclose failed");
    return 0;
}

static inline void Assert(int b)
{
    if (b == 0) _quit("Assert failed");
}
#endif
