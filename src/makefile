# FileSystem
CC = g++
CFLAGS = -w -g

BIN = ..\\bin\\FileSystem.exe

$(BIN): main.o BufferManager.o DeviceManager.o File.o FileManager.o FileSystem.o INode.o OpenFileManager.o Utility.o Kernel.o Command.o
	$(CC) -o $@ $^
main.o: main.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
BufferManager.o: BufferManager.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
DeviceManager.o: DeviceManager.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
File.o: File.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
FileManager.o: FileManager.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
FileSystem.o: FileSystem.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
INode.o: INode.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
OpenFileManager.o: OpenFileManager.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
Utility.o: Utility.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
Kernel.o: Kernel.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
Command.o:Command.cpp 
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	-del *.o $(BIN)
	-del ..\\bin\\*.img