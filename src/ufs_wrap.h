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

inline vomZd w_quit(char *buf)
{
    fprintf(stderr, "%s\n", buf);
    exit(-1);
}

inline int Fseek(FILE *fp, long offset, int whence)
{
    return fseek(fp, offset, whence) ? w_quit("Fseek failed") : 0;
}

inline long Ftell(FILE *fp)
{
    long n = ftell(fp);
    return (n < 0) ? w_quit("Ftell failed") : n;
}

inline FILE *Fopen(char *path, char *mode)
{
    FILE *n;
    return (n = fopen(path, mode)) ? n : w_quit("Fopen failed");
}

inline size_t Fread(void *ptr, size_t size, size_t obno, FILE *fp)
{
    size_t n = fread(ptr, size, obno, fp);
    return (n == obno) ? n : w_quit("Fread failed");
}

inline size_t Fwrite(void *ptr, size_t size, size_t obno, FILE *fp)
{
    size_t n = fwrite(ptr, size, obno, fp);
    return (n == obno) ? n : w_quit("Fwrite failed");
}

inline int Fclose(FILE *fp)
{
    return (EOF == fclose(fp)) ? w_quit("Fclose failed") : 0;
}

inline void Assert(int b) { b ? 0 : w_quit("Assert quit"); }
#endif
