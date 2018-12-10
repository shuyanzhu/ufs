/*************************************************************************
    > File Name: ufs.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 14时15分25秒
 ************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/ufs_wrap.h"
#include "../include/ufs_struct.h"
#include "../include/ufs.h"
// 全局变量
extern struct SuperBlk super;
extern struct MInode mInodes[MINODES];
extern FILE *ufsFp;
extern int maxUfd;
extern char cachBlk[BLKSIZE];
int UfsInit(char *path)
{
    ufsFp = Fopen(path, "ab+");
    Fclose(ufsFp); // 如果文件不存在，创建文件
    ufsFp = Fopen(path, "rb+");
    setbuf(ufsFp, NULL);

    // 初始化内存索引节点
    memset(mInodes, 0, sizeof(mInodes));
    maxUfd = 0;

    // 获得文件大小, 文件系统最大为2G
    if (fseek(ufsFp, 0, SEEK_END) < 0) _quit("UfsInit: 确定文件大小出错");
    long ufsLength = ftell(ufsFp); // 获取pos有移植性问题，要求x64，long=int64
    if (ufsLength < sizeof(struct SuperBlk)) {
        fclose(ufsFp);
        if (Init(path) < 0) _quit("UfsInit: 初始化磁盘块失败");
        printf("文件系统初始化成功\n");
        return 0;
    }
    if (fseek(ufsFp, 0, SEEK_SET) < 0) _quit("UfsInitL fseek failed");

    // 初始化超级块(磁盘)
    if (fread(&super, sizeof(super), 1, ufsFp) != 1)
        _quit("UfsInit: fread failed");
    if (super.magic != UFSMAGIC) {
        fclose(ufsFp);
        printf("无效的磁盘\n新的磁盘生成中\n");
        if (Init(path) < 0) _quit("UfsInit: 初始化磁盘块失败");
        printf("文件系统初始化成功\n");
        return 0;
    }

    printf("文件系统初始化成功\n");
    return 0;
}

int UfsOpen(char *path, int oflag)
{
    int iNum = 0;
    if (NameI(&iNum, path, oflag) < 0) return NOTHATFL;
    int ufd = FindNextMInode(iNum);
    if (ufd < 0) return NOMOREFD;

	// 填充索引节点表
    struct DInode *Dp = malloc(sizeof(struct DInode));
    mInodes[ufd].Dp = Dp;
	mInodes[ufd].oflag = oflag && 4;
	mInodes[ufd].iNbr = iNum;
    Fseek(ufsFp, ITABLESEEK + iNum * INODESIZE, SEEK_SET);
    Fread(Dp, sizeof(struct DInode), 1, ufsFp);

	// 如果规定清文件
	if (oflag && TRUNC) {
		if (Dp->fSize == 0) return ufd;
		if (Dp->type == 1)return NEEDCLRDIR;
		int pos = 0;
		for (pos = 0; pos < (Dp->fSize - 1) / BLKSIZE; pos++)
			BFree(pos, Dp);
	}

    return ufd;
}

int UfsClose(int ufd) {
	if (ufd == -1) {
		Fseek(ufsFp, 0, SEEK_SET);
		Fwrite(&super, sizeof(super), 1, ufsFp);
	}
	else {

	}
	return 1;
}

int UfsRead(int ufd, char *buf, int len) {
	struct MInode *mI = &mInodes[ufd];

	if (mI->pos == mI->Dp->fSize)return 0; // EOF
	
	int n = 0;
	int ipos, bpos;
	while (n != len) { // 外层循环，读块
		int i = 0;
		bpos = mI->pos / BLKSIZE;
		ipos = mI->pos % BLKSIZE;
		Bread(bpos, mI->Dp);
		while (n != len) { // 内层循环，读块内偏移量
			if (mI->pos + i == mI->Dp->fSize)return n; // 读到文件尾
			i++;
			buf[n++] = cachBlk[ipos++];
			if (ipos == BLKSIZE)break; // 本块结束
		}
		mI->pos += i; // 新的文件逻辑偏移量
	}
	return n;
	
}
int UfsWrite(int ufd, char *buf, int len){
	struct MInode *mI = &mInodes[ufd];
	
	int n = 0;
	int ipos, bpos;
	while (n != len) {
		int i = 0, nBlk = 0;
		bpos = mI->pos / BLKSIZE;
		ipos = mI->pos % BLKSIZE;
		if (mI->Dp->fSize / BLKSIZE - 1 < bpos) BAlloc(bpos, mI->Dp);
		Bread(bpos, mI->Dp);
		while (n != len) {
			i++;
			cachBlk[ipos++] = buf[n++];
			if (ipos == BLKSIZE)break;
		}
		mI->pos += i;
		if (mI->pos > mI->Dp->fSize)mI->Dp->fSize = mI->pos;
		FSW(cachBlk, sizeof(cachBlk), BMap(bpos, *(mI->Dp)));
	}
	return n;
}

