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

int BMap(int pos, struct DInode i)
{
	unsigned int blk[BLKSIZE / 4];
	if ((i.fSize - 1) / BLKSIZE < pos)return -1;
	if (pos < 10)
		return i.blkAddr[pos];
	else if (pos < 10 + 256) {
		Fseek(ufsFp, i.blkAddr[10] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[pos - 10];
	}
	else if (pos < 10 + 256 + 256 * 256) {
		Fseek(ufsFp, i.blkAddr[11] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		Fseek(ufsFp, blk[(pos - 10 - 256) / 256], SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[(pos - 10 - 256) % 256];
	}
	else {
		Fseek(ufsFp, i.blkAddr[12] * BLKSIZE, SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		Fseek(ufsFp, blk[(pos - 10 - 256 - 256 * 256) / (256 * 256)], SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		Fseek(
			ufsFp,
			blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) / 256],
			SEEK_SET);
		Fread(blk, sizeof(blk), 1, ufsFp);
		return blk[(pos - 10 - 256 - 256 * 256) % (256 * 256) % 256];
	}
}
int NameI(unsigned int *iNum, char *path, int oflag)
{
    if (path[0] != '/') return -1;
    struct DInode rootI;
    Fseek(ufsFp, ROOTISEEK, SEEK_SET);
    Fread(&rootI, sizeof(struct DInode), 1, ufsFp);
    Assert(rootI.type & 1);
	if (path[1] == 0) {
		*iNum = 1;
		return 1;
	}
    else { // 打开的文件不是根目录
        path = path + 1;
		if (rootI.fSize == 0)goto _fsize_0;
        int bAddr = (rootI.fSize - 1) / BLKSIZE + 1;
        struct Dir dirs[RDDIRNUM];
        unsigned int i = 0;
        for (i = 0; i < bAddr; i++) {
            Fseek(ufsFp, BMap(i, rootI)*BLKSIZE, SEEK_SET);
            ///////////////////////////////
            Assert(sizeof(dirs) == BLKSIZE);
            ///////////////////////////////
            Fread(dirs, sizeof(dirs), 1, ufsFp);
            int j = 0;
            for (int j = 0;
                 (i * BLKSIZE + j * sizeof(struct Dir) < rootI.fSize) &&
                 (j < RDDIRNUM);
                 j++)
				if (strcmp(path, (char *)&dirs[j]) == 0) {
					*iNum = dirs[j].iNbr;
					return 1;
				}
        } // 循环结束，未能找到和path相匹配的目录项
		
		// 创文件，分配新的索引节点
	_fsize_0:
		if (oflag && CREAT) {
			*iNum = CreatFile(path);
			return 1;
		}
		
		// 非创文件
        return -1;
    }
    //////////////////
    printf("%u\n", rootI.lNum);
    return 1;
    ////////////////
}

int FindOpenedI(unsigned int iNbr) {
	int i = 0;
	// 查询是否打开过
	for (i = 0; (i < maxUfd + 1) && (i != MINODES); i++)
		if (mInodes[i].iNbr == iNbr) return i;
	return -1;
}
int FindNextMInode(unsigned int iNbr)
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