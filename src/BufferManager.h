/*
    BufferManger类改写于Unix环境下的BufferManager类，用于管理内存缓冲区。
    删除了一些与进程有关的内容，使之更加适配于文件系统的编写

*/

#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Buf.h"
#include "DeviceManager.h"

class BufferManager
{
public:
	/* static const member */
	static const int NBUF = 15;			/* 缓存控制块、缓冲区的数量 */
	static const int BUFFER_SIZE = 512;	/* 缓冲区大小。 以字节为单位 */

public:
	BufferManager();
	~BufferManager();
	
	void Initialize();					/* 缓存控制块队列的初始化。将缓存控制块中b_addr指向相应缓冲区首地址。*/
	
	Buf* GetBlk(int blkno);	/* 申请一块缓存，用于磁盘上编号为blkno的内存块*/
	void Brelse(Buf* bp);				/* 释放缓存控制块buf */

	Buf* Bread(int blkno);	/* 读一个磁盘块。dev为主、次设备号，blkno为目标磁盘块逻辑块号。 */
	// Buf* Breada(short adev, int blkno, int rablkno);	/* 读一个磁盘块，带有预读方式。
	// 													 * adev为主、次设备号。blkno为目标磁盘块逻辑块号，同步方式读blkno。
	// 													 * rablkno为预读磁盘块逻辑块号，异步方式读rablkno。 */
	void Bwrite(Buf* bp);				/* 写一个磁盘块 */
	void Bdwrite(Buf* bp);				/* 延迟写磁盘块 */
	// void Bawrite(Buf* bp);				/* 异步写磁盘块 */

	void ClrBuf(Buf* bp);				/* 清空缓冲区内容 */
	void Bflush();				/* 将dev指定设备队列中延迟写的缓存全部输出到磁盘 */
	
	Buf& GetBFreeList();				/* 获取自由缓存队列控制块Buf对象引用 */

private:
	void GetError(Buf* bp);				/* 获取I/O操作中发生的错误信息 */
	void NotAvail(Buf* bp);				/* 从自由队列中摘下指定的缓存控制块buf */
	// Buf* InCore(short adev, int blkno);	/* 检查指定字符块是否已在缓存中 */
	
private:
	Buf bFreeList;						/* 自由缓存队列控制块 */
	Buf m_Buf[NBUF];					/* 缓存控制块数组 */
	unsigned char Buffer[NBUF][BUFFER_SIZE];	/* 缓冲区数组 */
	
	DeviceManager* m_DeviceManager;		/* 指向设备管理模块全局对象,这里实际上只有磁盘 */
};

#endif