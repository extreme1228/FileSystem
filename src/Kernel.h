/*
该Kernel文件改写自UNIX V6++源代码，删除了一些与
进程有关的函数定义，用来辅助文件系统的编写。
*/
#ifndef KERNEL_H
#define KERNEL_H

//只保留和文件系统相关的内容
#include "User.h"			//用户管理
#include "BufferManager.h"	//缓冲区管理
#include "DeviceManager.h"	//设备管理(这里主要是磁盘)
#include "FileManager.h"	//文件管理
#include "FileSystem.h"		//文件系统


/*
 * Kernel类用于封装所有内核相关的全局类实例对象，
 * 例如PageManager, ProcessManager等。
 * 
 * Kernel类在内存中为单体模式，保证内核中封装各内核
 * 模块的对象都只有一个副本。
 */
class Kernel
{
	/* Functions */
public:
	Kernel();
	~Kernel();
	static Kernel& Instance();	//获取Kernel单体类实例
	void Initialize();		/* 该函数完成初始化内核大部分数据结构的初始化 */
	

	/* Members */
	BufferManager& GetBufferManager();
	DeviceManager& GetDeviceManager();
	FileSystem& GetFileSystem();
	FileManager& GetFileManager();
	User& GetUser();

private:
	void InitBuffer();
	void InitFileSystem();
	void InitDevice();
	void InitFileManager();
	void InitUser();

private:
	static Kernel instance;		/* Kernel单体类实例 */

	BufferManager* m_BufferManager;
	DeviceManager* m_DeviceManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
    User * m_User;
};

#endif

