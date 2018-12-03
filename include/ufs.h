/*************************************************************************
    > File Name: ufs.h
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 10时58分58秒
 ************************************************************************/

#ifndef __NDSL_UFS_H__
#define __NDSL_UFS_H__

#define UO_RD 1
#define UO_WR (1u << 1)
/*
typedef struct
{
}*/

// 初始化文件系统
int UfsInit(char *path);
int UfsOpen(char *path, int oflag);
int UfsRead(int ufd, char *buf, int len);
int UfsWrite(int ufd, char *buf, int len);
int UfsUnlink(char *path);
// DIR *DirOpen(char *pathname);
// struct dirent *DirRead(DIR *dp);

#endif
