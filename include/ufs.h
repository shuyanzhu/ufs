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
#define UO_RDWR (1 << 1)
#define UO_APPEND (1 << 2)
#define UO_TRUNC (1 << 3)
#define UO_CREAT (1 << 4)

#define NOMOREFD -7
#define NOTHATFL -2
#define NEEDCLRDIR -3
#define BADUFD -4
#define NOMOREBLKS -5
#define NOTROOT -6
#define WRONLY -8
#define RDONLY -9
#define BADOFLAG -10
#define PATHTOOLONG -11

// 目录结构
struct Dirent
{
    char name[29];
};
// 初始化文件系统
int UfsInit(char *path);
int UfsOpen(char *path, int oflag);
int UfsClose(int ufd);
int UfsRead(int ufd, void *buff, int len);
int UfsWrite(int ufd, void *buff, int len);
int UfsUnlink(char *path);
int DirOpen();
struct Dirent *DirRead(int ufd);

#endif
