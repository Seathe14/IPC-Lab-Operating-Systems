#include <iostream>
#include <windows.h>
#include <vector>
#include <tchar.h>
#include <ctime>
#define BUFSIZE 512
DWORD _stdcall ArraySort(LPVOID);

//CONSTS
const int ArrSize = 15;
const int amountOfPipes = 5;
//VARS
int arr[ArrSize];
int finalArr[ArrSize];
int count = 0;

//HANDLES
HANDLE g_hPipes[amountOfPipes];
HANDLE g_hEvent;

//VECTORS
std::vector<std::vector<int>> bucket;


int main()
{
    BOOL fConnected = FALSE;
    DWORD dwThreadId = 0;
    HANDLE hPipe, hThread;
    char ch;
    g_hEvent = CreateEvent(NULL, FALSE, FALSE, L"Event");
    srand(time(NULL));
    for (int i = 0; i < ArrSize; i++)
    {
        arr[i] = rand() % (amountOfPipes*10);
    }
    for (int i = 0; i < amountOfPipes; i++)
    {
        bucket.push_back(std::vector<int>());
    }
    for (int i = 0; i < ArrSize; i++)
    {
        bucket[arr[i] / 10].push_back(arr[i]);
    }
    LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mypipe");
    std::cout << "Welcome to the Pipe Server!" << std::endl;
    for (int i = 0; i < amountOfPipes; i++)
    {
		_tprintf(TEXT("Pipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
		hPipe = CreateNamedPipe(lpszPipename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, amountOfPipes, BUFSIZE, BUFSIZE, 0, NULL);
		if (hPipe == INVALID_HANDLE_VALUE)
		{
			std::cerr << "Failed to create named pipe" << std::endl;
			//exit(1);
		}
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{
			printf("Client connected, creating a processing thread.\n");
			hThread = CreateThread(NULL, 0, ArraySort, (LPVOID)hPipe, 0, &dwThreadId);
			if (hThread == NULL)
				std::cerr << "Failed to create thread" << std::endl;
			else
				CloseHandle(hThread);
		}
		else
			CloseHandle(hPipe);
		Sleep(1);
    }
    WaitForSingleObject(g_hEvent,INFINITE);
	while (count == amountOfPipes)
	{
		std::cout << "You've reached maximum amount of g_hPipes, type 'E' to exit" << std::endl;
		std::cin >> ch;
        if (ch == 'E')
            break;
	}
    int index = 0;
    for (int i = 0; i < amountOfPipes; i++)
        for (int j = 0; j < bucket[i].size(); j++)
            finalArr[index++] = bucket[i][j];
    std::cout << "The resulting Array is: ";
    for (int i = 0; i < ArrSize; i++)
    {
        std::cout << finalArr[i] << " ";
    }
    std::cout << std::endl;
    system("pause");
	return 0;
}


DWORD _stdcall ArraySort(LPVOID lpParam)
{
    g_hPipes[count] = (HANDLE)lpParam;
    count++;
    if (count == amountOfPipes)
        SetEvent(g_hEvent);
	BOOL fSuccess = FALSE;
    DWORD cbBytesRead, cbReplyBytes, cbWritten;
	HANDLE hPipe = NULL;
    int thisCount = 0;
    hPipe = (HANDLE)lpParam;
    
	WriteFile(hPipe, &count, sizeof(int), &cbWritten, NULL);

	ReadFile(hPipe, &thisCount, sizeof(int), &cbBytesRead, NULL);

    int buckSize = bucket[thisCount - 1].size();

    int* buf = new int[buckSize];
    for (int i = 0; i < buckSize; i++)
    {
        buf[i] = bucket[thisCount - 1][i];
    }
    WriteFile(hPipe, &buckSize, sizeof(int), &cbWritten, NULL);
    while (true)
    {
        Sleep(5);
        fSuccess = WriteFile(hPipe, buf, buckSize * sizeof(int), &cbWritten, NULL);
        if (!fSuccess) break;
        fSuccess = ReadFile(hPipe, buf, buckSize * sizeof(int), &cbBytesRead, NULL);
		if (!fSuccess || cbBytesRead == 0)
			break;
    }
   // ReadFile(hPipe, &thisCount, sizeof(int), &cbBytesRead, NULL);
    std::cout << "Pipe No" << thisCount - 1 << " returned: ";
	for (int i = 0; i < buckSize; i++)
	{
		std::cout << buf[i] << " ";
		bucket[thisCount - 1][i] = buf[i];
	}
    std::cout << std::endl;
    delete[] buf;
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return 0;
}
