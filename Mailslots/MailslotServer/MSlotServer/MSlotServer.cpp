// MSlotServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <thread>
#include <mutex>
HANDLE g_hSlot;
HANDLE g_hChildSlots[2];
HANDLE g_hMutex;
LPCTSTR SlotName = TEXT("\\\\.\\mailslot\\mymailslot");
LPCTSTR childNames[2] = { TEXT("\\\\.\\mailslot\\client1"),
						TEXT("\\\\.\\mailslot\\client2") };
const int ArrSize = 15;
int arr[ArrSize];
int finalArr[ArrSize];
const int amountOfClients = 2;
std::vector<std::vector<int>> bucket;
std::mutex mtx;

void fillBucket(HANDLE childSlot,int j)
{
	DWORD cbRead, cbWritten;
	int buckSize = bucket[j].size();
	int* buf = new int[buckSize];
	for (int i = 0; i < buckSize; i++)
	{
		buf[i] = bucket[j][i];
	}
	WriteFile(childSlot, &buckSize, sizeof(int), &cbWritten, NULL);
	WriteFile(childSlot, buf, buckSize * sizeof(int), &cbWritten, NULL);
	ReadFile(g_hSlot, buf, buckSize * sizeof(int), &cbRead, NULL);
	std::cout << "Client No" << j << " returned: ";
	for (int i = 0; i < buckSize; i++)
	{
		std::cout << buf[i] << " ";
		bucket[j][i] = buf[i];
	}
	delete[]buf;
	std::cout << std::endl;
}
void generateBucket()
{
	for (int i = 0; i < ArrSize; i++)
	{
		arr[i] = rand() % (amountOfClients * 25);
	}
	for (int i = 0; i < amountOfClients; i++)
	{
		bucket.push_back(std::vector<int>());
	}
	for (int i = 0; i < ArrSize; i++)
	{
		bucket[arr[i] / 25].push_back(arr[i]);
	}
}
void createClientSlot(int i)
{
		g_hChildSlots[i] = CreateFile(childNames[i], GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		while (g_hChildSlots[i] == INVALID_HANDLE_VALUE)
		{
			g_hChildSlots[i] = CreateFile(childNames[i], GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			Sleep(5);
		}
		mtx.lock();
		std::cout << "Client #" << i << " has been connected\n";
		mtx.unlock();
}

void createStuff()
{
	g_hSlot = CreateMailslot(SlotName, 0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);
	g_hMutex = CreateMutex(NULL, FALSE, L"Mutex");
	if (g_hSlot == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailSlot failed with %d\n", GetLastError());
	}
	std::cout << "Welcome to the MailSlot server!\nWaiting for Clients to connect..." << std::endl;
	std::thread t[2];
	for (int i = 0; i < 2; i++)
	{
		t[i] = std::thread(createClientSlot, i);
	}
	for (int i = 0; i < 2; i++)
	{
		t[i].join();
	}
	std::cout << "Press any key to start sorting" << std::endl;
	std::cin.get();
	generateBucket();
}
void sortArr()
{
	int index = 0;
	for (int i = 0; i < 2; i++)
		for (unsigned int j = 0; j < bucket[i].size(); j++)
			finalArr[index++] = bucket[i][j];
}
void arrPrint()
{
	std::cout << "The resulting Array is: ";
	for (int i = 0; i < ArrSize; i++)
	{
		std::cout << finalArr[i] << " ";
	}
	std::cout << std::endl;

}
void readFill()
{
	DWORD cbRead;
	int currChild;
	for (int i = 0; i < 2; i++)
	{
		ReadFile(g_hSlot, &currChild, sizeof(int), &cbRead, NULL);
		fillBucket(g_hChildSlots[currChild], currChild);
	}
	sortArr();
}
void CloseHandles()
{
	CloseHandle(g_hSlot);
	CloseHandle(g_hMutex);
	for (int i = 0; i < 2; i++)
	{
		CloseHandle(g_hChildSlots[i]);
	}
}
int main()
{
	srand(time(NULL));
	createStuff();
	readFill();
	arrPrint();
	CloseHandles();
	system("pause");
	return 0;
}
