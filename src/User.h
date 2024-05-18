/*
该文件改写自UNIX V6++源代码，删除了一些与进程无关的函数定义与实现，
并且尝试将User改为用户的定义，用来辅助文件系统的编写。
*/

#ifndef USER_H
#define USER_H

#include "FileManager.h"
#include "File.h"

/*
 * @comment 该类与Unixv6中 struct user结构对应，因此只改变
 * 类名，不修改成员结构名字，关于数据类型的对应关系如下:
 */
class User
{
public:
	
	/* u_error's Error Code */
	/* 1~32 来自linux 的内核代码中的/usr/include/asm/errno.h, 其余for V6++ */
	enum ErrorCode
	{
		NOERROR	= 0,	/* No error */
		EPERM	= 1,	/* Operation not permitted */
		ENOENT	= 2,	/* No such file or directory */
		ESRCH	= 3,	/* No such process */
		EINTR	= 4,	/* Interrupted system call */
		EIO		= 5,	/* I/O error */
		ENXIO	= 6,	/* No such device or address */
		E2BIG	= 7,	/* Arg list too long */
		ENOEXEC	= 8,	/* Exec format error */
		EBADF	= 9,	/* Bad file number */
		ECHILD	= 10,	/* No child processes */
		EAGAIN	= 11,	/* Try again */
		ENOMEM	= 12,	/* Out of memory */
		EACCES	= 13,	/* Permission denied */
		EFAULT  = 14,	/* Bad address */
		ENOTBLK	= 15,	/* Block device required */
		EBUSY 	= 16,	/* Device or resource busy */
		EEXIST	= 17,	/* File exists */
		EXDEV	= 18,	/* Cross-device link */
		ENODEV	= 19,	/* No such device */
		ENOTDIR	= 20,	/* Not a directory */
		EISDIR	= 21,	/* Is a directory */
		EINVAL	= 22,	/* Invalid argument */
		ENFILE	= 23,	/* File table overflow */
		EMFILE	= 24,	/* Too many open files */
		ENOTTY	= 25,	/* Not a typewriter(terminal) */
		ETXTBSY	= 26,	/* Text file busy */
		EFBIG	= 27,	/* File too large */
		ENOSPC	= 28,	/* No space left on device */
		ESPIPE	= 29,	/* Illegal seek */
		EROFS	= 30,	/* Read-only file system */
		EMLINK	= 31,	/* Too many links */
		EPIPE	= 32,	/* Broken pipe */
		ENOSYS	= 100,
		//EFAULT	= 106
	};

	
public:
	//系统调用返回值
    int system_ret;
	int u_arg[5];				/* 存放当前系统调用参数 */
	char* u_dirp;				/* 系统调用参数(一般用于Pathname)的指针 */


	/* 文件系统相关成员 */
	Inode* u_cdir;		/* 指向当前目录的Inode指针 */
	Inode* u_pdir;		/* 指向父目录的Inode指针 */

	DirectoryEntry u_dent;					/* 当前目录的目录项 */
	char u_dbuf[DirectoryEntry::DIRSIZ];	/* 当前路径分量 */
	char u_curdir[128];						/* 当前工作目录完整路径 */

	ErrorCode u_error;			/* 存放错误码 */
	int u_segflg;				/* 表明I/O针对用户或系统空间 */

	// /* 这里尝试改为用户的ID */
	short u_uid;		/* 有效用户ID */
	short u_gid;		/* 有效组ID */
	short u_ruid;		/* 真实用户ID */
	short u_rgid;		/* 真实组ID */
	
	/* 文件系统相关成员 */
	OpenFiles u_ofiles;		/* 进程打开文件描述符表对象 */

	/* 文件I/O操作 */
	IOParameter u_IOParam;	/* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */

	/* Member Functions */
// public:
// 	/* 根据系统调用参数uid设置有效用户ID，真实用户ID，进程用户ID(p_uid) */
// 	void Setuid();

// 	/* 获取用户ID，低16比特为真实用户ID(u_ruid)，高16比特为有效用户ID(u_uid) */
// 	void Getuid();

// 	/* 根据系统调用参数gid设置有效组ID，真实组ID */
// 	void Setgid();

// 	/* 获取组ID, 低16比特为真实组ID(u_rgid)，高16比特为有效组ID(u_gid) */
// 	void Getgid();

// 	/* 获取当前用户工作目录 */
// 	void Pwd();

// 	/* 检查当前用户是否是超级用户 */
// 	bool SUser();

// 	void Initialize();//初始化用户
};

#endif

