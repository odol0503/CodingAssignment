#ifndef SERVER_H
#define SERVER_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <inttypes.h>

#include "common.hpp"

#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 1 

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

class Server
{
private:
	vector<PIPEINST> m_pipes;
	HANDLE m_hEvents[INSTANCES];
	unordered_map<string, shared_ptr<CustObj>> m_obj;

	VOID DisconnectAndReconnect(DWORD i)
	{
		// Disconnect the pipe instance. 
		if (!DisconnectNamedPipe(m_pipes[i].hPipeInst))
		{
			cout << "DisconnectNamedPipe failed with " << GetLastError() << "." << endl;
		}

		// Call a subroutine to connect to the new client. 
		m_pipes[i].fPendingIO = ConnectToNewClient(
			m_pipes[i].hPipeInst,
			&m_pipes[i].oOverlap);

		m_pipes[i].dwState = m_pipes[i].fPendingIO ?
			CONNECTING_STATE : // still connecting 
			READING_STATE;     // ready to read
	}

	BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
	{
		BOOL fConnected, fPendingIO = FALSE;

		// Start an overlapped connection for this pipe instance. 
		fConnected = ConnectNamedPipe(hPipe, lpo);

		// Overlapped ConnectNamedPipe should return zero. 
		if (fConnected)
		{
			cout << "ConnectNamedPipe failed with " << GetLastError() << "." << endl;
			return FALSE;
		}

		switch (GetLastError())
		{
			// The overlapped connection in progress. 
		case ERROR_IO_PENDING:
			fPendingIO = TRUE;
			break;

			// Client is already connected, so signal an event. 
		case ERROR_PIPE_CONNECTED:
			if (SetEvent(lpo->hEvent))
				break;

			// If an error occurs during the connect operation... 
		default:
			cout << "ConnectNamedPipe failed with " << GetLastError() << "." << endl;
			return FALSE;
		}

		return fPendingIO;
	}

	void GetAnswerToRequest(PIPEINST *pPipe)
	{
		json j = deserialize(pPipe->chRequest);
		int type = j.at("type");

		switch (type)
		{
		case INTEGER_TYPE:
			cout << "received integer=" << j.at("value") << endl << endl;
			break;

		case STRING_TYPE:
			cout << "received string=" << j.at("value") << endl << endl;
			break;

		case STRUCT_TYPE:
			cout << "received structure=" << j.at("value") << endl << endl;
			break;

		case CALLFUNC_TYPE:
		{
			string class_name = j.at("class_name");
			string func_name = j.at("func_name");
			string args = j.at("args");
			string ret_val;
			cout << "called " << class_name << "::" << func_name << endl;
			call_func(class_name, func_name, args, ret_val);
			if (ret_val.size())
			{
				memset(pPipe->chRequest, 0x00, BUFSIZE*sizeof(TCHAR));
				StringCchCopy(pPipe->chRequest, BUFSIZE * sizeof(TCHAR), ret_val.c_str());
			}
			break;
		}
		
		default:
			cout << "invalid type!";
			return;
		}

		memset(pPipe->chReply, 0x00, BUFSIZE*sizeof(TCHAR));
		StringCchCopy(pPipe->chReply, BUFSIZE * sizeof(TCHAR), pPipe->chRequest);
		pPipe->cbToWrite = (strlen(pPipe->chReply) + 1) * sizeof(TCHAR);
	}

	int call_func(string &class_name, string &func_name, string &args, string &ret_val)
	{
		if (m_obj.count(class_name) == 0) return -1;
		shared_ptr<CustObj> pObj = m_obj[class_name];
		shared_ptr<CustFunc> pFunc = pObj->get_func(func_name);
		if (!pFunc) return -1;
		pFunc->call(args, ret_val);
		return true;
	}

public:
	Server()
	{
		m_pipes.resize(INSTANCES);
	}

	int init()
	{
		// The initial loop creates several instances of a named pipe 
		// along with an event object for each instance.  An 
		// overlapped ConnectNamedPipe operation is started for 
		// each instance. 
		for (int i = 0; i < INSTANCES; i++)
		{
			// Create an event object for this instance. 
			m_hEvents[i] = CreateEvent(
				NULL,    // default security attribute 
				TRUE,    // manual-reset event 
				TRUE,    // initial state = signaled 
				NULL);   // unnamed event object 

			if (m_hEvents[i] == NULL)
			{
				cout << "CreateEvent failed with " << GetLastError() << "." << endl;
				return -1;
			}

			//cout << "CreateEvent: m_hEvents[" << i << "]=" << showbase << hex << m_hEvents[i] << endl;

			m_pipes[i].oOverlap.hEvent = m_hEvents[i];

			m_pipes[i].hPipeInst = CreateNamedPipe(
				PIPE_NAME,				// pipe name 
				PIPE_ACCESS_DUPLEX |     // read/write access 
				FILE_FLAG_OVERLAPPED,    // overlapped mode 
				PIPE_TYPE_MESSAGE |      // message-type pipe 
				PIPE_READMODE_MESSAGE |  // message-read mode 
				PIPE_WAIT,               // blocking mode 
				INSTANCES,               // number of instances 
				BUFSIZE * sizeof(TCHAR),   // output buffer size 
				BUFSIZE * sizeof(TCHAR),   // input buffer size 
				PIPE_TIMEOUT,            // client time-out 
				NULL);                   // default security attributes 

			if (m_pipes[i].hPipeInst == INVALID_HANDLE_VALUE)
			{
				cout << "CreateNamedPipe failed with" << GetLastError() << endl;
				return -1;
			}

			//cout << "CreateNamedPipe: m_pipes[" << i << "].hPipeInst=" << hex << m_pipes[i].hPipeInst << endl;

			// Call the subroutine to connect to the new client
			m_pipes[i].fPendingIO = ConnectToNewClient(
				m_pipes[i].hPipeInst,
				&m_pipes[i].oOverlap);

			m_pipes[i].dwState = m_pipes[i].fPendingIO ?
				CONNECTING_STATE : // still connecting 
				READING_STATE;     // ready to read 

			//cout << "ConnectToNewClient: m_pipes[" << i << "].fPendingIO=" << dec << m_pipes[i].fPendingIO << endl;
		}

		cout << "Server starting..." << endl;
		return 0;
	}

	int run()
	{
		DWORD dwWait, cbRet, dwErr;;
		BOOL fSuccess;

		while (1)
		{
			// Wait for the event object to be signaled, indicating 
			// completion of an overlapped read, write, or 
			// connect operation. 
			dwWait = WaitForMultipleObjects(
				INSTANCES,    // number of event objects 
				m_hEvents,      // array of event objects 
				FALSE,        // does not wait for all 
				INFINITE);    // waits indefinitely 

			// dwWait shows which pipe completed the operation. 
			int i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
			if (i < 0 || i >(INSTANCES - 1))
			{
				cout << "Index out of range." << endl;
				return -1;
			}

			//cout << "WaitForMultipleObjects index=" << i << endl;

			// Get the result if the operation was pending. 
			if (m_pipes[i].fPendingIO)
			{
				fSuccess = GetOverlappedResult(
					m_pipes[i].hPipeInst, // handle to pipe 
					&m_pipes[i].oOverlap, // OVERLAPPED structure 
					&cbRet,            // bytes transferred 
					FALSE);            // do not wait 

				switch (m_pipes[i].dwState)
				{
					// Pending connect operation 
				case CONNECTING_STATE:
					if (!fSuccess)
					{
						cout << "Error " << GetLastError() << "." << endl;
						return -1;
					}
					m_pipes[i].dwState = READING_STATE;
					break;

					// Pending read operation 
				case READING_STATE:
					if (!fSuccess || cbRet == 0)
					{
						DisconnectAndReconnect(i);
						continue;
					}
					m_pipes[i].cbRead = cbRet;
					m_pipes[i].dwState = WRITING_STATE;
					break;

					// Pending write operation 
				case WRITING_STATE:
					if (!fSuccess || cbRet != m_pipes[i].cbToWrite)
					{
						DisconnectAndReconnect(i);
						continue;
					}
					m_pipes[i].dwState = READING_STATE;
					break;

				default:
					cout << "Invalid pipe state" << endl;
					return -1;
				}
			}

			// The pipe state determines which operation to do next. 
			switch (m_pipes[i].dwState)
			{
				// READING_STATE: 
				// The pipe instance is connected to the client 
				// and is ready to read a request from the client. 
			case READING_STATE:
				memset(m_pipes[i].chRequest, 0x00, BUFSIZE * sizeof(TCHAR));
				fSuccess = ReadFile(
					m_pipes[i].hPipeInst,
					m_pipes[i].chRequest,
					BUFSIZE * sizeof(TCHAR),
					&m_pipes[i].cbRead,
					&m_pipes[i].oOverlap);

				//cout << "READING_STATE: fSuccess=" << fSuccess << ", "
				//	<< "m_pipes[" << i << "].cbRead=" << m_pipes[i].cbRead << endl;

				// The read operation completed successfully.
				if (fSuccess && m_pipes[i].cbRead != 0)
				{
					m_pipes[i].fPendingIO = FALSE;
					m_pipes[i].dwState = WRITING_STATE;
					continue;
				}

				// The read operation is still pending. 
				dwErr = GetLastError();
				if (!fSuccess && (dwErr == ERROR_IO_PENDING))
				{
					m_pipes[i].fPendingIO = TRUE;
					continue;
				}

				// An error occurred; disconnect from the client. 
				DisconnectAndReconnect(i);
				break;

				// WRITING_STATE: 
				// The request was successfully read from the client. 
				// Get the reply data and write it to the client. 
			case WRITING_STATE:
				GetAnswerToRequest(&m_pipes[i]);

				fSuccess = WriteFile(
					m_pipes[i].hPipeInst,
					m_pipes[i].chReply,
					m_pipes[i].cbToWrite,
					&cbRet,
					&m_pipes[i].oOverlap);

				// The write operation completed successfully. 
				if (fSuccess && cbRet == m_pipes[i].cbToWrite)
				{
					m_pipes[i].fPendingIO = FALSE;
					m_pipes[i].dwState = READING_STATE;
					continue;
				}

				// The write operation is still pending. 
				dwErr = GetLastError();
				if (!fSuccess && (dwErr == ERROR_IO_PENDING))
				{
					m_pipes[i].fPendingIO = TRUE;
					continue;
				}

				// An error occurred; disconnect from the client. 
				DisconnectAndReconnect(i);
				break;

			default:
				cout << "Invalid pipe state." << endl;
				return -1;
			}
		}

		return 0;
	}

	int register_obj(shared_ptr<CustObj> obj)
	{
		if (m_obj.count(obj->get_name())) return -1;

		m_obj[obj->get_name()] = obj;
		return 0;
	}
};

#endif