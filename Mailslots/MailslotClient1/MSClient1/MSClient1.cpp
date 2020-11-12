// MSActualClient1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <algorithm>
LPCTSTR SlotName = TEXT("\\\\.\\mailslot\\mymailslot");
LPCTSTR SlotName1 = TEXT("\\\\.\\mailslot\\client1");
HANDLE g_hMutex;
HANDLE g_hSlot;
HANDLE g_hThisSlot;
void init()
{
	g_hThisSlot = CreateMailslot(SlotName1, 0, MAILSLOT_WAIT_FOREVER, NULL);
	g_hSlot = CreateFile(SlotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	std::cout << "Trying to connect to the server..." << std::endl;
	while (g_hSlot == INVALID_HANDLE_VALUE)
	{
		g_hSlot = CreateFile(SlotName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		Sleep(5);
	}
	std::cout << "Connected to the server..." << std::endl;
	g_hMutex = OpenMutex(SYNCHRONIZE, FALSE, L"Mutex");
}
void printArr(int *arr, int size)
{
	for (int i = 0; i < size; i++)
	{
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}
void readWrite()
{
	DWORD cbWritten, cbRead;
	WaitForSingleObject(g_hMutex, INFINITE);
	int thisNum = 0;
	WriteFile(g_hSlot, &thisNum, sizeof(int), &cbWritten, NULL);
	int sizeOfArray;
	ReadFile(g_hThisSlot, &sizeOfArray, sizeof(int), &cbRead, NULL);
	int* Array = new int[sizeOfArray];
	ReadFile(g_hThisSlot, Array, sizeOfArray * sizeof(int), &cbRead, NULL);
	std::sort(Array, Array + sizeOfArray);
	WriteFile(g_hSlot, Array, sizeOfArray * sizeof(int), &cbWritten, NULL);
	printArr(Array, sizeOfArray);
	ReleaseMutex(g_hMutex);

	delete[] Array;
}
void closeHandles()
{
	CloseHandle(g_hSlot);
	CloseHandle(g_hThisSlot);
	CloseHandle(g_hMutex);
}
int main()
{
	init();
	readWrite();
	closeHandles();
	Sleep(5000);

	//system("pause");
}
