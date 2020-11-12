// FileMappingClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <algorithm>
HANDLE g_hFileMap;
HANDLE g_hLeadingMutex;
HANDLE g_hEvent;
HANDLE g_hEvent2;
void createStuff()
{
	std::cout << "Welcome to the File Mapping client!" << std::endl;
	std::cout << "Trying to connect to the File Mapping server..." << std::endl;
	while (g_hFileMap == NULL)
	{
		g_hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"Local\\FileMap");
		Sleep(2);
	}
	while (g_hEvent == NULL)
	{
		g_hEvent = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"Event");
		Sleep(2);
	}
	while (g_hEvent2 == NULL)
	{
		g_hEvent2 = OpenEvent(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, L"Event2");
		Sleep(2);
	}
	while (g_hLeadingMutex == NULL)
	{
		g_hLeadingMutex = OpenMutex(SYNCHRONIZE, FALSE, L"LeadingMutex");
		Sleep(2);
	}
	std::cout << "Connected successfully!" << std::endl;
}
void printArr(int* arr, int size)
{
	for (int i = 0; i < size; i++)
	{
		std::cout << arr[i] << " ";
	}
	std::cout << std::endl;
}
void readInfo()
{
	WaitForSingleObject(g_hLeadingMutex, INFINITE);

	//first read
	WaitForSingleObject(g_hEvent, INFINITE);
	int* temp = (int*)(MapViewOfFile(g_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int)));
	int size = *temp;
	SetEvent(g_hEvent2);

	//second read
	WaitForSingleObject(g_hEvent, INFINITE);
	temp = (int*)(MapViewOfFile(g_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, size * sizeof(int)));

	int* arr = new int[size];
	for (int i = 0; i < size; i++)
	{
		arr[i] = temp[i];
	}

	printArr(arr, size);

	std::sort(arr, arr + size);
	//Write

	CopyMemory(temp, arr, size * sizeof(int));
	SetEvent(g_hEvent2);

	printArr(arr, size);
	delete[] arr;
	ReleaseMutex(g_hLeadingMutex);
}
int main()
{
	createStuff();
	readInfo();
}
