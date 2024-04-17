#include "Kernel.h"
#include<iostream>
Kernel Kernel::instance;    // Kernel单体类实例


/*
 * 设备管理、高速缓存管理全局manager
 */
BufferManager g_BufferManager;
DeviceManager g_DeviceManager;

/*
 * 文件系统相关全局manager
 */
FileSystem g_FileSystem;
FileManager g_FileManager;

//全局用户
User g_User;

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

Kernel& Kernel::Instance()
{
	return Kernel::instance;
}



void Kernel::InitBuffer()
{
	this->m_BufferManager = &g_BufferManager;
	this->GetBufferManager().Initialize();
}

void Kernel::InitDevice()
{
    this->m_DeviceManager = &g_DeviceManager;
    this->GetDeviceManager().Initialize();
}
void Kernel::InitFileSystem()
{
	this->m_FileSystem = &g_FileSystem;
	this->GetFileSystem().Initialize();
}

void Kernel::InitFileManager()
{
    this->m_FileManager = &g_FileManager;
    this->GetFileManager().Initialize();
}
void Kernel::InitUser()
{
    this->m_User = &g_User;
}

void Kernel::Initialize()
{
	// g_BufferManager = BufferManager();
	// g_DeviceManager = DeviceManager();
	// g_FileSystem = FileSystem();
	// g_FileManager = FileManager();
	// g_User = User();
    InitDevice();
    InitUser();
	InitFileManager();
	InitBuffer();
	InitFileSystem(); 
}


BufferManager& Kernel::GetBufferManager()
{
	return *(this->m_BufferManager);
}

DeviceManager& Kernel::GetDeviceManager()
{
	return *(this->m_DeviceManager);
}

FileSystem& Kernel::GetFileSystem()
{
	return *(this->m_FileSystem);
}

FileManager& Kernel::GetFileManager()
{
	return *(this->m_FileManager);
}

User& Kernel::GetUser()
{
	return *(this->m_User);
}
