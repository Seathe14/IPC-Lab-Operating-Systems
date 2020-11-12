// FileMappingOS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <ctime>
//CONSTS
const int numOfClients = 4;
const int ArrSize = 15;
//VARS
int arr[ArrSize];
int finalArr[ArrSize];
//HANDLES
HANDLE g_hFileMap;
HANDLE g_hLeadingMutex;
HANDLE g_hEvent;
HANDLE g_hEvent2;

//VECTOR
std::vector<std::vector<int>> bucket;

void fillBucket(int j)
{
	DWORD Result;
	int buckSize = bucket[j].size();
	int* buf = new int[buckSize];
	LPVOID fileMap = NULL;

	for (int i = 0; i < buckSize; i++)
	{
		buf[i] = bucket[j][i];
	}
	
	int* temp = NULL;
	fileMap = MapViewOfFile(g_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 512);
	// first write
	CopyMemory(fileMap, &buckSize, sizeof(int));
	SetEvent(g_hEvent);

	WaitForSingleObject(g_hEvent2,INFINITE);

	//second write
	CopyMemory(fileMap, buf, (sizeof(int) * ArrSize));
	SetEvent(g_hEvent);


	//read
	WaitForSingleObject(g_hEvent2, INFINITE);
	temp = (int*)MapViewOfFile(g_hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * buckSize);

	for (int i = 0; i < buckSize; i++)
	{
		bucket[j][i] = temp[i];
	}
	std::cout << "Client No" << j << " returned: ";
	for (int i = 0; i < buckSize; i++)
	{
		std::cout << bucket[j][i] << " ";
	}
	std::cout << std::endl;
	delete[]buf;
	UnmapViewOfFile(fileMap);
	UnmapViewOfFile(temp);
	std::cout << std::endl;
}
void generateBucket()
{
	for (int i = 0; i < ArrSize; i++)
	{
		arr[i] = rand() % (numOfClients * 25);
	}

	for (int i = 0; i < numOfClients; i++)
	{
		bucket.push_back(std::vector<int>());
	}

	for (int i = 0; i < ArrSize; i++)
	{
		bucket[arr[i] / 25].push_back(arr[i]);
	}
}
void createStuff()
{
	srand(time(NULL));
	g_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 512, L"Local\\FileMap");
	g_hLeadingMutex = CreateMutex(NULL, FALSE, L"LeadingMutex");
	g_hEvent = CreateEvent(NULL, FALSE, FALSE, L"Event");
	g_hEvent2 = CreateEvent(NULL, FALSE, FALSE, L"Event2");
	generateBucket();
	std::cout << "Welcome to the File Mapping server!\nPress any key, to start" << std::endl;
	std::cin.get();
}
void sortArr()
{
	int index = 0;

	for (int i = 0; i < numOfClients; i++)
		for (int j = 0; j < bucket[i].size(); j++)
			finalArr[index++] = bucket[i][j];
}
void arrPrint()
{
	for (int i = 0; i < ArrSize; i++)
	{
		std::cout << finalArr[i] << " ";
	}
	std::cout << std::endl;

}
void deleteStuff()
{
	CloseHandle(g_hFileMap);
	CloseHandle(g_hLeadingMutex);
	CloseHandle(g_hEvent);
	CloseHandle(g_hEvent2);
}
int main()
{
	createStuff();
	for (int i = 0; i < numOfClients; i++)
	{
		fillBucket(i);
	}
	sortArr();
	arrPrint();
	deleteStuff();
	system("pause");
}
