/*************************************************************************
    > File Name: ufs.h
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 10时58分58秒
 ************************************************************************/

#ifndef __NDSL_UFS_H__
#define __NDSL_UFS_H__

#define UO_RD 0
#define UO_WR 1
#define UO_RDWR (1u << 1)
#define UO_APPEND (1u << 2)
#define UO_TRUNC (1u << 3)
#define UO_CREAT (1u << 4)
#define NOMOREFD -1
#define NOTHATFL -2

/*
typedef struct
{
}*/

// 初始化文件系统
int UfsInit(char *path);
int UfsOpen(char *path, int oflag);
int UfsClose(int ufd);
int UfsRead(int ufd, char *buf, int len);
int UfsWrite(int ufd, char *buf, int len);
int UfsUnlink(char *path);
// DIR *DirOpen(char *pathname);
// struct dirent *DirRead(DIR *dp);

#endif
