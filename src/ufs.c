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
extern char zeros[BLKSIZE];

static inline void _free_file(struct DInode *dI)
{
    int pos = 0;
    for (pos = 0; pos < (dI->fSize - 1) / BLKSIZE; pos++)
        BFree(pos, dI);
    dI->fSize = 0;
    return;
}

static void _ufs_close() {
	UfsClose(-1);
	return;
}

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
		atexit(_ufs_close); // 注册
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
		atexit(_ufs_close); // 注册
        return 0;
    }

    printf("文件系统初始化成功\n");
	atexit(_ufs_close); // 注册
    return 0;
}

int UfsOpen(char *path, int oflag)
{
	if (strlen(path) > 28)return -1;
	if (oflag & 3 == 3)return BADOFLAG;

    int iNum = 0;
    if (NameI(&iNum, path, oflag) < 0) return NOTHATFL;
    int ufd = FindNextMInode(iNum);
    if (ufd < 0) return NOMOREFD;

    // 填充索引节点表
    struct DInode *Dp = malloc(sizeof(struct DInode));
    mInodes[ufd].Dp = Dp;
    mInodes[ufd].oflag = oflag & (UO_APPEND | UO_RDWR |UO_WR); // 和7位与
    mInodes[ufd].iNbr = iNum;
    Fseek(ufsFp, ITABLESEEK + iNum * INODESIZE, SEEK_SET);
    Fread(Dp, sizeof(struct DInode), 1, ufsFp);

    // 如果规定清文件
    if (oflag & UO_TRUNC) {
        if (Dp->fSize == 0) return ufd;
        if (Dp->type == 1) return NEEDCLRDIR;
        _free_file(Dp);
    }

    return ufd;
}
int UfsClose(int ufd)
{ // 成功返回0，失败返回-1
    if (ufd < -1 || ufd > MINODES - 1) return -1;
    if (ufd == -1) { // 关闭所有打开的索引节点，写回超级块
        int i = 0;
        for (i = 0; i < maxUfd; i++) {
            struct MInode *mI = &mInodes[i];
            if (mI->Dp == NULL) continue;
            if (mI->Dp->lNum == 0) { // 联结数为0，释放磁盘块
                _free_file(mI->Dp);
                FreeI(mI->iNbr);
                free(mI->Dp);
                memset(mI, 0, sizeof(struct MInode));
                continue;
            }
            FSW(mI->Dp,
                sizeof(struct DInode),
                ITABLESEEK + INODESIZE * mI->iNbr); // 写回索引节点
            free(mI->Dp);
            memset(mI, 0, sizeof(struct MInode));
        }
        Fseek(ufsFp, 0, SEEK_SET);
        Fwrite(&super, sizeof(super), 1, ufsFp);
		fprintf(stderr, "Ufs exit\n");
    } else {
        struct MInode *mI = &mInodes[ufd];
        if (mI->Dp == NULL) return -1;
        if (mI->Dp->lNum == 0) { // 联结数为0，释放磁盘块
            _free_file(mI->Dp);
            FreeI(mI->iNbr);
            free(mI->Dp);
            memset(mI, 0, sizeof(struct MInode));
            return 0;
        }
        FSW(mI->Dp,
            sizeof(struct DInode),
            ITABLESEEK + INODESIZE * mI->iNbr); // 写回索引节点
        free(mI->Dp);
        memset(mI, 0, sizeof(struct MInode));
    }
    return 0;
}

int UfsRead(int ufd, void *buff, int len)
{
	if (ufd < 0 || ufd > maxUfd)return BADUFD;
    char *buf = (char *) buff;
    struct MInode *mI = &mInodes[ufd];

	if (mI->oflag & 3 == UO_WR)return WRONLY;

    if (mI->Dp == NULL) return BADUFD;      // 没有打开的文件描述符
    if (mI->pos == mI->Dp->fSize) return 0; // EOF

    int n = 0;
    int ipos, bpos;
    while (n != len) { // 外层循环，读块
        int i = 0;
        bpos = mI->pos / BLKSIZE;
        ipos = mI->pos % BLKSIZE;
        BRead(bpos, mI->Dp);
        while (n != len) { // 内层循环，读块内偏移量
            if (mI->pos + i == mI->Dp->fSize) return n; // 读到文件尾
            i++;
            buf[n++] = cachBlk[ipos++];
            if (ipos == BLKSIZE) break; // 本块结束
        }
        mI->pos += i; // 新的文件逻辑偏移量
    }
    return n;
}
int UfsWrite(int ufd, void *buff, int len)
{
	if (ufd < 0 || ufd > maxUfd)return BADUFD;
    char *buf = (char *) buff;
    struct MInode *mI = &mInodes[ufd];

	if (mI->oflag & 3 == UO_RD)return RDONLY;

    if (mI->Dp == NULL) return BADUFD; // 没有打开的文件描述符
	int append = mI->pos;
	if (mI->oflag & UO_APPEND)mI->pos = mI->Dp->fSize;

    int n = 0;
    int ipos, bpos;
    while (n != len) {
        int i = 0, nBlk = 0;
        bpos = mI->pos / BLKSIZE;
        ipos = mI->pos % BLKSIZE;
        if ((mI->Dp->fSize-1)/ BLKSIZE  < bpos)
            if (BAlloc(bpos, mI->Dp) < 0) return NOMOREBLKS;
        BRead(bpos, mI->Dp);
        while (n != len) {
            i++;
            cachBlk[ipos++] = buf[n++];
            if (ipos == BLKSIZE) break;
        }
        mI->pos += i;
        if (mI->pos > mI->Dp->fSize) mI->Dp->fSize = mI->pos;
        int map = BMap(bpos, *(mI->Dp));
        if (map == 0)
            if ((map = BAlloc(bpos, mI->Dp)) < 0) return NOMOREBLKS;
        FSW(cachBlk, sizeof(cachBlk), map * BLKSIZE);
    }
	if (mI->oflag & UO_APPEND)mI->pos = append; // 如果为追加模式，则恢复原来的文件偏移量
    return n;
}

int UfsUnlink(char *path)
{
    if (path[0] != '/') return -1;
    if (path[1] == 0) return -1;
    path = path + 1;

    int iNbr, ufd;

    // 释放目录项
    struct DInode *rDI;
    if (ufd = FindOpenedI(1) < 0) {
        rDI = (struct DInode *) malloc(sizeof(struct DInode));
        FSR(rDI, sizeof(struct DInode), ROOTISEEK);
    } else
        rDI = mInodes[ufd].Dp;
    int map, j;
    struct Dir dirs[RDDIRNUM];
    memset(dirs, 0, sizeof(dirs));
    if ((j = FindDirent(path, rDI, dirs, &map)) < 0) return 0;
    iNbr = dirs[j].iNbr;
    // 最后一个目录项
    struct Dir edir;
    int rootEnd =
        BMap(rDI->fSize / BLKSIZE, *rDI) * BLKSIZE + rDI->fSize % BLKSIZE;
    FSR(&edir, sizeof(edir), rootEnd - sizeof(edir));
    // 把最后一个目录项放到被释放的目录项中
    dirs[j] = edir;
    FSW(dirs, sizeof(dirs), map * BLKSIZE);
    // 如果当前目录项释放后可以释放某一块
    if (rDI->fSize % BLKSIZE == sizeof(struct Dir))
        BFree(rDI->fSize / BLKSIZE, rDI);
    rDI->fSize -= sizeof(struct Dir);

    // 对该文件的索引节点进行操作
    struct DInode *dI;
    if ((ufd = FindOpenedI(iNbr)) < 0) { // 如果文件未曾打开

        dI = (struct DInode *) malloc(sizeof(struct DInode));
        FSR(dI, sizeof(struct DInode), ITABLESEEK + iNbr * INODESIZE);
        if (--dI->lNum) return 0;
        _free_file(dI);
        FreeI(iNbr);
        free(dI);
        dI = NULL;
    } else { // 简单将已经打开的索引节点降低联结数
        dI = mInodes[ufd].Dp;
        dI->lNum--;
    }
    return 0;
}

int DirOpen() { return UfsOpen("/", 0); }
struct Dirent *DirRead(int ufd)
{
	if (ufd < 0 || ufd > maxUfd)return BADUFD;
	if (mInodes[ufd].Dp->type != DIRTYPE)return NOTROOT;

    struct Dirent *dirent = (struct Dirent *) malloc(sizeof(struct Dirent));
    struct Dir dir;


    if (0 == UfsRead(ufd, &dir, sizeof(struct Dir))) return NULL;
    memcpy(dirent->name, dir.name, sizeof(dirent->name));
    return dirent;
}
