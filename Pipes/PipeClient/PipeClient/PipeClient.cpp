// OSClientActual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <vector>
#include <algorithm> 
#define BUFSIZE 512


//int Array[5];
int _tmain(int argc, TCHAR *argv[])
{
    HANDLE hPipe;
    LPTSTR lpszWrite = TEXT("Default message from client");
    TCHAR chReadBuf[BUFSIZE];
    BOOL fSuccess = FALSE;
    DWORD cbRead, cbToWrite, cbWritten, dwMode;
    int thisCount = 0;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mypipe");
    if (argc > 1)
        lpszWrite = argv[1];
    while (1)
    {
        hPipe = CreateFile(lpszPipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE)
            break;
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return -1;
		}
        if (!WaitNamedPipe(lpszPipename, 15000))
        {
            printf("Could not open pipe: 15 sec timeout");
            return -1;
        }
    }
    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
    if (!fSuccess)
    {
        _tprintf(TEXT("SetNamePipeHandleState failed %d\n", GetLastError));
        return -1;
    }
        int sizeOfArray;
        Sleep(5);
        ReadFile(hPipe,&thisCount,sizeof(int), &cbRead, NULL);

		WriteFile(hPipe, &thisCount, sizeof(int), &cbWritten, NULL);


        ReadFile(hPipe, &sizeOfArray, sizeof(int), &cbRead, NULL);

        int *Array = new int[sizeOfArray];
        fSuccess = ReadFile(hPipe, Array, sizeOfArray * sizeof(int), &cbRead, NULL);
    if (!fSuccess && (GetLastError() != ERROR_MORE_DATA))
    {
        printf("TransactNamedPipe failed.\n");
        return 0;
    }
    std::sort(Array, Array + sizeOfArray);
    std::cin.get();
    while (1)
    {
        fSuccess = WriteFile(hPipe, Array, sizeOfArray * sizeof(int), &cbWritten, NULL);
        if (fSuccess)
            break;
        else _tprintf(TEXT("%s\n", Array));
    }

    delete[] Array;
	CloseHandle(hPipe);
    return 0;
}
