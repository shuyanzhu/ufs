/*************************************************************************
    > File Name: ufs_struct.c
    > Author: 朱会茗
    > Mail: 1294206499@qq.com
    > Created Time: 2018年12月03日 星期一 16时50分16秒
 ***********************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/ufs_wrap.h"
#include "../include/ufs_struct.h"

struct SuperBlk super;
FILE *ufsFp;
struct MInode mInodes[MINODES];
int maxUfd = 0;
char cachBlk[BLKSIZE];
char zeros[BLKSIZE] = { 0 };
// 初始化磁盘块
int Init(char *path)
{
	ufsFp = fopen(path, "wb+");
	setbuf(ufsFp, NULL);
	// 初始化磁盘大小
	if (fseek(ufsFp, UFSSIZE - 1, SEEK_SET) < 0) return FSERR;
	char c = 0;
	if (fwrite(&c, 1, 1, ufsFp) != 1) return FWERR;
	if (fseek(ufsFp, 0, SEEK_SET) < 0) return FSERR;

	// 初始化超级快
	memset(&super, 0, sizeof(super));
	super.magic = UFSMAGIC;
	super.diskSize = UFSSIZE-1;
	super.inodeNum = 1u << (24 - 6);
	super.blkNum = UFSSIZE / BLKSIZE - DATABGN;
	super.dirty = 0;

	// 写数据区磁盘块数组
	int i, j = 0;
	int fBlk[FREEBNUM];
	for (i = DATABGN; i < UFSSIZE / BLKSIZE; i = i + FREEBNUM) {
		for (int j = 0; j < FREEBNUM; j++)
			fBlk[j] = i + j + 1;
		// 最后一组空闲块只有254块，因为超级块用去两块
		if (i == UFSSIZE / BLKSIZE - FREEBNUM + 2)fBlk[FREEBNUM - 1] = fBlk[FREEBNUM - 2] = 0;
		if (fseek(ufsFp, i * BLKSIZE, SEEK_SET) < 0) return FSERR;
		if (fwrite(fBlk, sizeof(fBlk), 1, ufsFp) != 1) return FWERR;
	}
	if (fseek(ufsFp, (DATABGN + FREEBNUM - 1) * BLKSIZE, SEEK_SET) < 0)
		return FSERR;
	if (fread(super.freeBlk, sizeof(super.freeBlk), 1, ufsFp) != 1)
		return FRERR;
	super.freeBlk[FREEBNUM - 1] = DATABGN;
	super.nextB = FREEBNUM - 1;

	// 写索引节点列表区磁盘数组
	char zeros[4096] = { 0 };
	if (fseek(ufsFp, ITABLEBGN * BLKSIZE, SEEK_SET) < 0) return FSERR;
	for (i = 0; i < BLKSOFIN / 4; i++)
		if (fwrite(zeros, sizeof(zeros), 1, ufsFp) != 1) return FWERR;
	struct DInode rootI; // 磁盘索引节点的根节点
	rootI.type = 1;
	rootI.fSize = 0;
	rootI.lNum = 1;
	memset(&rootI.blkAddr, 0, sizeof(rootI.blkAddr));
	if (fseek(ufsFp, ITABLEBGN * BLKSIZE + sizeof(struct DInode), SEEK_SET) < 0)
		return FSERR;
	if (fwrite(&rootI, sizeof(struct DInode), 1, ufsFp) != 1) return FWERR;
	for (i = 0; i < FREEINUM; i++)
		super.freeInode[i] = i + 2;

	// 写超级块
	if (fseek(ufsFp, 0, SEEK_SET) < 0) return FSERR;
	if (fwrite(&super, sizeof(super), 1, ufsFp) != 1) return FSERR;

	return 0;
}

// 不考虑有文件空洞
int AllocBlk(){
	if (super.blkNum == 0)return -1;
	int ret = super.freeBlk[super.nextB];
	if (super.nextB == FREEBNUM -1) {
		Fseek(ufsFp, ret*BLKSIZE, SEEK_SET);
		Fread(super.freeBlk, sizeof(super.freeBlk), 1, ufsFp);
	}
	super.nextB = (super.nextB + 1) % FREEBNUM;
	super.blkNum--;
	return ret;
}
int FreeBlk(int blkNbr) {
	if (blkNbr == 0)return 1;
	if (super.nextB == 0) {
		Fseek(ufsFp, blkNbr * BLKSIZE, SEEK_SET);
		Fwrite(super.freeBlk, sizeof(super.freeBlk), 1, ufsFp);
	}
	if ((super.nextB = super.nextB - 1) < 0)super.nextB + 256;
	super.freeBlk[super.nextB] = blkNbr;
	super.blkNum++;
	return blkNbr;
	
}
int AllocI() {
	if (super.inodeNum == 0)return -1;
	int ret = super.freeInode[super.nextN];
	if (super.nextN == FREEINUM - 1) {
		int i = 0, nextDI = ret+1, pos = (ret + 1) * INODESIZE + ITABLESEEK;
		struct DInode dI;
		Fseek(ufsFp, pos, SEEK_SET);
		while (i != FREEINUM) {
			Fread(&dI, sizeof(dI), 1, ufsFp);
			if (dI.type == 0)
				super.freeInode[i++] = nextDI;
			nextDI++;
		}
	}
	super.nextN = (super.nextN + 1) % FREEINUM;
	super.inodeNum--;
	return ret;
}
int FreeI(int iNbr) {
	struct DInode dI;
	memset(&dI, 0, sizeof(dI));
	if (super.nextN == 0) {
		if (iNbr < super.freeInode[FREEINUM - 1])
			super.freeInode[FREEINUM - 1] = iNbr;
	}
	else 
		super.freeInode[--super.nextN] = iNbr;
	super.inodeNum++;
	Fseek(ufsFp, ITABLESEEK + INODESIZE * iNbr, SEEK_SET);
	Fwrite(&dI, sizeof(dI), 1, ufsFp);
	return iNbr;
}
int CreatFile(char *path) {
	int blkNbr0 = 0, blkNbr1 = 0, blkNbr2 = 0, blkNbr3 = 0; // 将要分配的三级磁盘块，以便出错时释放
	int iNbr, ufd; // 要分配的索引节点
	struct DInode *i, rootI, newI;
	struct Dir dirent;
	
	memset(&dirent, 0, sizeof(dirent));
	if (strlen(path) > 27)return BADFILENAME;
	if (strcpy(dirent.name, path) == NULL)return BADFILENAME;

	// 根据根节点有没有被打开确定指针i
	if ((ufd = FindOpenedI(1)) != -1) i = mInodes[ufd].Dp;
	else {	// 如果根节点没有打开而缓存
		i = &rootI;
		Fseek(ufsFp, ROOTISEEK, SEEK_SET);
		Fread(i, sizeof(rootI), 1, ufsFp);
	}

	// 分配索引节点
	if ((iNbr = AllocI()) < 0)return NOMOREINODE;
	// 分配磁盘块
	if (i->fSize%BLKSIZE == 0) {
		if (i->fSize < 10 * BLKSIZE) { // 暂时只写这种情况
			blkNbr0 = AllocBlk();
			if (blkNbr0 < 0)goto _no_more_blk;
			i->blkAddr[i->fSize / BLKSIZE] = blkNbr0;
		}
	}

	// 写回索引节点
	memset(&newI, 0, sizeof(newI));
	newI.type = FILETYPE; newI.fSize = 0; newI.lNum = 1;
	Fseek(ufsFp, ITABLESEEK + INODESIZE * iNbr, SEEK_SET);
	Fwrite(&newI, sizeof(newI), 1, ufsFp);

	// 写根索引节点，用来之后的
	Fseek(ufsFp, ROOTISEEK, SEEK_SET);
	i->fSize += sizeof(dirent);
	Fwrite(i, sizeof(struct DInode), 1, ufsFp);
	// 写目录项，文件大小已经增大，所以要降低偏移量
	dirent.iNbr = iNbr;
	Fseek(ufsFp, BMap(i->fSize / BLKSIZE, *i) * BLKSIZE + i->fSize%BLKSIZE-sizeof(dirent), SEEK_SET);
	Fwrite(&dirent, sizeof(dirent), 1, ufsFp);
	return iNbr;

_no_more_inode:
	return NOMOREINODE;
_no_more_blk:
	FreeI(iNbr);
	return NOMOREBLK;
}

// 返回文件描述符
int FindOpenedI(int iNbr) {
	int i = 0;
	// 查询是否打开过
	for (i = 0; (i < maxUfd + 1) && (i != MINODES); i++)
		if (mInodes[i].iNbr == iNbr) return i;
	return -1;
}
int FindNextMInode(int iNbr)
{
	int i = 0;
	// 查询是否打开过
	if ((i = FindOpenedI(iNbr)) >= 0)return i;

	// 寻找空位
	for (i = 0; (i < maxUfd + 1) && (i != MINODES); i++)
		if (mInodes[i].Dp == NULL) break;
	if (i == maxUfd)
		if (maxUfd == MINODES)
			return -1;
		else
			return maxUfd++;
	else
		return i;
}
int FindDirent(char *path, struct DInode *rootI, struct Dir dirs[RDDIRNUM], int *map) {
	if (rootI->fSize == 0)return -1;	 // 如果根目录为空	
	int bAddr = (rootI->fSize - 1) / BLKSIZE + 1;
	int i = 0;
	for (i = 0; i < bAddr; i++) {
		*map = BMap(i, *rootI);
		if (map <= 0)_quit("Bad BMap");
		FSR(dirs, sizeof(struct Dir)*RDDIRNUM, (*map) * BLKSIZE);
		int j = 0;
		for (int j = 0;
			(i * BLKSIZE + j * sizeof(struct Dir) < rootI->fSize) &&
			(j < RDDIRNUM);
			j++)
			if (strcmp(path, (char *)&dirs[j]) == 0) return j;
	} // 循环结束，未能找到和path相匹配的目录项
	return -1;
}
int NameI(int *iNum, char *path, int oflag)
{
	if (path[0] != '/') return -1;

	// 如果找的文件是根目录
	if (path[1] == 0) {
		*iNum = 1;
		return 1;
	}
	// 找的是普通文件，则首先拿到根目录
	struct DInode rootI;
	int i = 0;
	path = path + 1;
	if ((i = FindOpenedI(1)) >= 0)rootI = *(mInodes[i].Dp); // 根目录已经打开
	else { // 根目录未曾打开
		Fseek(ufsFp, ROOTISEEK, SEEK_SET);
		Fread(&rootI, sizeof(struct DInode), 1, ufsFp);
		Assert(rootI.type & 1);
	}

	struct Dir dirs[RDDIRNUM];
	memset(dirs, 0, sizeof(dirs));
	int map = 0; // 目录项所在的物理块号
	int j = FindDirent(path, &rootI, dirs, &map); // 目录项的块内偏移量
	if (j >= 0)return dirs[j].iNbr;	
	  // 创文件，分配新的索引节点
	if (oflag & CREAT) {
		*iNum = CreatFile(path);
		return 1;
	}

	// 非创文件
	return -1;
}

// 底下这几段代码写得贼恶心，如果考虑文件空洞，则会更恶心
int BMap(int pos, struct DInode i)
{
	int blk[BLKSIZE / 4];
	if ((i.fSize - 1) / BLKSIZE < pos)return -1;

	// 分别为直接块、一级间接块、二级间接块、三级间接块对应的情况
	if (pos < 10)
		return i.blkAddr[pos];
	else if (pos < 10 + 256) {
		if (i.blkAddr[10] == 0)return 0; //文件空洞

		Fseek(ufsFp, i.blkAddr[10] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[pos - 10];
	}
	else if (pos < 10 + 256 + 256 * 256) {
		if (i.blkAddr[11] == 0)return 0;

		Fseek(ufsFp, i.blkAddr[11] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);

		if (blk[(pos - 10 - 256) / 256] == 0)return 0;
		Fseek(ufsFp, blk[(pos - 10 - 256) / 256], SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[(pos - 10 - 256) % 256];
	}
	else {
		if (i.blkAddr[12] == 0)return 0;

		Fseek(ufsFp, i.blkAddr[12] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);

		if (blk[(pos - 10 - 256 - 256 * 256) / (256 * 256)] == 0)return 0;
		Fseek(ufsFp, blk[(pos - 10 - 256 - 256 * 256) / (256 * 256)], SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);

		if (blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) / 256] == 0)return 0;
		Fseek(
			ufsFp,
			blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) / 256],
			SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) % 256];
	}
}
int BRead(int pos, struct DInode *inode) {
	int iNbr = BMap(pos, *inode);
	if (iNbr == 0) {
		memset(cachBlk, 0, BLKSIZE);
		return iNbr;
	}
	else
		FSR(cachBlk, sizeof(cachBlk), iNbr * BLKSIZE);
	return iNbr;
}
int BAlloc(int pos, struct DInode *inode){

	// 直接块、一级间接块、二级间接块、三级间接块
	if (pos < 10) {
		int iNbr;
		if ((iNbr = AllocBlk()) < 0)return -1;
		inode->blkAddr[pos] = iNbr;
	}
	else if (pos < 10 + 256) {
		int blks1[256] = { 0 };
		if (inode->blkAddr[10] == 0) {
			int iNbr0;
			if((iNbr0 = AllocBlk()) < 0)return -1;
			inode->blkAddr[10] = iNbr0;
			FSW(blks1, sizeof(blks1), iNbr0 * BLKSIZE);
		}

		int iNbr1;
		if ((iNbr1 = AllocBlk()) < 0)return -1;
		blks1[pos - 10] = iNbr1;
		FSW(blks1, sizeof(blks1), inode->blkAddr[10] * BLKSIZE);
	}
	else if (pos < 10 + 256 + 256 * 256) { // 太麻烦了，暂时不写
	}
	else { // 太麻烦了，暂时不写
	}
	return 1;
}
int BFree(int pos, struct DInode *inode) {
	// 直接块、一级间接块、二级间接块、三级间接块
	if (pos < 10) {
		FreeBlk(inode->blkAddr[pos]);
		inode->blkAddr[pos] = 0;
	}
	else if (pos < 10 + 256) {
		int blks[BLKSIZE / 4] = { 0 };
		Fseek(ufsFp, inode->blkAddr[10] * BLKSIZE, SEEK_SET);
		Fread(blks, sizeof(blks), 1, ufsFp);
		int blkA = blks[pos - 10];
		FreeBlk(blkA);
		blks[pos - 10] = 0;
		Fseek(ufsFp, -BLKSIZE, SEEK_CUR);
		Fwrite(blks, sizeof(blks), 1, ufsFp);

		// 释放一级块
		if (pos == 10) { // 情况少见
			FreeBlk(inode->blkAddr[10]);
			inode->blkAddr[10] = 0;
		}
	}
	else if (pos < 10 + 256 + 256 * 256) {
		int blks1[BLKSIZE / 4] = { 0 }, blks2[BLKSIZE / 4] = { 0 };
		Fseek(ufsFp, inode->blkAddr[11] * BLKSIZE, SEEK_SET);
		Fread(blks1, sizeof(blks1), 1, ufsFp);
		Fseek(ufsFp, blks1[(pos - 10 - 256) / 256], SEEK_SET);
		Fread(blks2, sizeof(blks2), 1, ufsFp);

		FreeBlk(blks2[(pos - 10 - 256) % 256]);
		blks2[(pos - 10 - 256) % 256] = 0;
		Fseek(ufsFp, -BLKSIZE, SEEK_CUR);
		Fwrite(blks2, sizeof(blks2), 1, ufsFp);

		if (pos == 10 + 256) {
			FreeBlk(blks1[0]);
			FreeBlk(inode->blkAddr[11]);
			inode->blkAddr[11] = 0;
		}
		else if ((pos - 10 - 256) % 256 == 0) {
			FreeBlk(blks1[(pos - 10 - 256) / 256]);
			blks1[(pos - 10 - 256) / 256] = 0;
			Fseek(ufsFp, inode->blkAddr[11], SEEK_SET);
			Fwrite(blks1, sizeof(blks1), 1, ufsFp);
		}
	}
	else { // 太麻烦了，暂时不写
	}
	return 1;
}