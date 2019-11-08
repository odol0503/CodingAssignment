#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>

#include "common.hpp"

class Client {
private:
	HANDLE m_hPipe;
	TCHAR m_buf[BUFSIZE];

public:
	int init()
	{
		while (1)
		{
			m_hPipe = CreateFile(
				PIPE_NAME,   // pipe name 
				GENERIC_READ |  // read and write access 
				GENERIC_WRITE,
				0,              // no sharing 
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_FLAG_OVERLAPPED, 
				NULL);          // no template file 

			// Break if the pipe handle is valid. 
			if (m_hPipe != INVALID_HANDLE_VALUE)
				break;

			// Exit if an error other than ERROR_PIPE_BUSY occurs. 
			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				cout << "Could not open pipe. GLE=" << GetLastError() << endl;
				return -1;
			}

			// All pipe instances are busy, so wait for 20 seconds. 
			if (!WaitNamedPipe(PIPE_NAME, 20000))
			{
				cout << "Could not open pipe: 20 second wait timed out." << endl;
				return -1;
			}
		}

		return 0;
	}

	int close()
	{
		CloseHandle(m_hPipe);
		return 0;
	}

	int sendMsgAsync(json &input, json &output)
	{
		OVERLAPPED oWrite, oRead;
		string msg = serialize(input);

		memset(&oWrite, 0, sizeof(oWrite));
		memset(&oRead, 0, sizeof(oRead));

		oWrite.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		oRead.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

		if (oWrite.hEvent == NULL || oRead.hEvent == NULL)
		{
			std::cout << "CreateEvent error: " << GetLastError() << std::endl;
			return -1;
		}

		BOOL fSuccess = WriteFile(m_hPipe, (LPCVOID)msg.c_str(), msg.length(), NULL, &oWrite);

		DWORD error = GetLastError();
		if (!fSuccess && error == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(oWrite.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				DWORD nBytes = 0;
				GetOverlappedResult(m_hPipe, &oWrite, &nBytes, NULL);
			}
			else
			{
				CancelIo(m_hPipe);
				return -1;
			}
		}

		memset(m_buf, 0x00, BUFSIZE * sizeof(TCHAR));
		fSuccess = ReadFile(m_hPipe, (void*)m_buf, BUFSIZE*sizeof(TCHAR), 0, &oRead);
		error = GetLastError();

		if (!fSuccess && error == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(oRead.hEvent, INFINITE) == WAIT_OBJECT_0)
			{
				DWORD nBytes = 0;
				GetOverlappedResult(m_hPipe, &oRead, &nBytes, true);
			}
			else
			{
				CancelIo(m_hPipe);
				return -1;
			}
		}
		else if (fSuccess)
		{
			getOutput(m_buf, output);
		}

		return 0;
	}

	int sendMsgSync(json &input, json &output)
	{
		DWORD nBytes;
		string msg = serialize(input);
		bool fSuccess = WriteFile(m_hPipe, (void*)msg.c_str(), msg.size(), &nBytes, NULL);

		memset(m_buf, 0x00, BUFSIZE * sizeof(TCHAR));
		fSuccess = ReadFile(m_hPipe, (LPVOID)m_buf, BUFSIZE*sizeof(TCHAR), 0, NULL);
		if (!fSuccess) {
			return -1;
		}

		getOutput(m_buf, output);

		return 0;
	}

	void getOutput(string input, json &output)
	{
		output = deserialize(input);
	}
};

#endif