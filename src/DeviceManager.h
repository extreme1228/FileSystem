/*
    该类改写自UNix V6++源代码，因为本次文件系统中仅仅涉及
    一个磁盘文件的读写，所以在该文件处做了较大幅度的删减
*/

#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

//本次文件系统中不涉及块设备和字符设备的操作，所以这两个头文件也不需要
// #include "BlockDevice.h"
// #include "CharDevice.h"
#include<stdio.h>
#include "Buf.h"

class DeviceManager
{
private:
	static const char* DISK_FILE_NAME;	/* 磁盘文件名 */
	static const int BLOCK_SIZE = 512;

	FILE * m_DiskFile;		/* 磁盘文件指针 */
public:
	DeviceManager();
	~DeviceManager();

	void Initialize();

	void FormatDisk();		/* 格式化磁盘 */

	void IO_read(Buf* bp);	/* 从磁盘读取数据 */

	void IO_write(Buf* bp);	/* 向磁盘写入数据 */
};

#endif
