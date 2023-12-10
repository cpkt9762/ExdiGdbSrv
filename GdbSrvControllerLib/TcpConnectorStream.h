//----------------------------------------------------------------------------
//
//  TcpConnectorStream.h
//
//  This header file implements two clases:
//  1.  The TcpConnectorStream class encapsulates the socket mechanisms to 
//      actively connect to a server. This class will produce TcpConnectorStream 
//      objects when a client wants to establish a socket connection with a server.
//      Also, it encapsulates the WinSock initialization and cleanup mechanism.
//
//  2.  The class TcpIpStream provides TCP/IP network I/O mechanisms.
//      Basically, this class provides methods to send and receive data over 
//      a TCP/IP connection as well as methods to configure the socket connection.
//      Also, It stores the connected socket descriptor and information about the 
//      server peer (the IP address and TCP port number).
//
// Copyright (c) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "ExceptionHelpers.h"
#include "TextHelpers.h"
#include <mutex>
#include <assert.h>
#include <intrin.h>
#pragma comment(lib, "Ws2_32.lib")

namespace GdbSrvControllerLib
{
	//  Verifies if the error identifies a connection lost socket event.
#define IS_CONNECTION_LOST(error)   ((error == WSAENETDOWN) || (error == WSAENOTCONN) || (error == WSAENETRESET) || \
                                         (error == WSAESHUTDOWN) || (error == WSAECONNABORTED) || (error == WSAETIMEDOUT) || \
                                         (error == WSAECONNRESET))

	//  The TcpIpStream class provides basic methods to configure, send, and receive data over a TCP/IP socket connection. 
	//  Each connection is completely encapsulated in each TcpIpStream object (the socket descriptor is kept private).
	//  The majority of the class methods are wrappers around the Bekerly Socket library functions.
	//  The class stores a pointer to the TextHandler object (it does not own this object) that allows tracing 
	//  all communication data sent and received over the socket connection.
	class TcpIpStream final
	{
	public:
		friend class TcpConnectorStream;

		~TcpIpStream()
		{
			Close();
		} 
		int Send(_In_ LPCSTR pBuffer, _In_ int length)
		{
			//printf("Send1 Send:%d %s\n", length, pBuffer);
			
			//printf("Send2 Send:%d %s\n", length, pBuffer);
			std::lock_guard<std::mutex> lock(m_mutex); 
			//
			assert(pBuffer != nullptr);

			//CallDisplayFunction(pBuffer, length, GdbSrvTextType::Command);

			int bytesDone = 0;
			PCHAR pTempBuffer = const_cast<PCHAR>(pBuffer);

			while (length > 0)
			{
				int sentBytes = send(m_socket, pTempBuffer, length, 0);
				if (sentBytes == SOCKET_ERROR || sentBytes == 0)
				{
					break;
				}
				pTempBuffer += sentBytes;
				length -= sentBytes;
				bytesDone += sentBytes;
			}
			//printf("Send3 Send:%d %s\n", length, pBuffer);
			return bytesDone;
		}
		//这个是发送中断信号,不能加锁
		int SendInterrupt(_In_ LPCSTR pBuffer, _In_ int length)
		{
			//printf("SendInterrupt Send:%d %s\n", length, pBuffer);  
			assert(pBuffer != nullptr);

			CallDisplayFunction(pBuffer, length, GdbSrvTextType::Command);

			int bytesDone = 0;
			PCHAR pTempBuffer = const_cast<PCHAR>(pBuffer);

			while (length > 0)
			{
				int sentBytes = send(m_socket, pTempBuffer, length, 0);
				if (sentBytes == SOCKET_ERROR || sentBytes == 0)
				{
					break;
				}
				pTempBuffer += sentBytes;
				length -= sentBytes;
				bytesDone += sentBytes;
			}
			//printf("SendInterrupt Send:%d %s\n", length, pBuffer);
			return bytesDone;
		}

	private:

		int ReceiveImpl(_Out_writes_bytes_(length) PCHAR pBuffer, _In_ int length)
		{

			int bytes_read = 0;
			assert(pBuffer != nullptr);
			memset(pBuffer, 0, length);
			auto size = std::size_t(length);
			bool iscontinue = false;
			//if (length<=256)
			{
				bytes_read=recv(m_socket, reinterpret_cast<char*>(pBuffer), (int)size, 0);
				//printf("Recv1 Receive:%d %d %s\n", bytes_read, size, pBuffer);
				if (bytes_read == size)
				{
					goto exit;
				}

				if ('#' == reinterpret_cast<const char*>(pBuffer)[bytes_read-3])
				{
					goto exit;
				}
				
			}
			if (bytes_read < static_cast<int>(size))
			{
				int remaining_bytes_read = 0;

				do
				{
					remaining_bytes_read = recv(m_socket, reinterpret_cast<char*>(pBuffer) + bytes_read, (int)size - bytes_read, 0);

					if (remaining_bytes_read < 0)
					{
						//printf("Recv bytes_read:%d remaining_bytes_read:%d %d %d\n", bytes_read, remaining_bytes_read, size, GetTickCount() - dwTime);
						bytes_read = remaining_bytes_read;
						goto exit;
					}
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						bytes_read = remaining_bytes_read;
						//printf("Recv bytes_read:%d remaining_bytes_read:%d %d %d\n", bytes_read, remaining_bytes_read, size, GetTickCount() - dwTime);
						goto exit;
					}
					if (remaining_bytes_read == 0)
					{
						bytes_read = remaining_bytes_read;
						//printf("Recv bytes_read:%d remaining_bytes_read:%d %d %d\n", bytes_read, remaining_bytes_read, size, GetTickCount() - dwTime);
						goto exit;
					}

					bytes_read += remaining_bytes_read;

					if ('#' == reinterpret_cast<const char*>(pBuffer)[bytes_read - 3])
					{
						goto exit;
					} 

				} while (bytes_read < static_cast<int>(size));

			}

		exit:

			if (bytes_read > 0)
			{
				//printf("Recv Receive:%d %d %s\n", bytes_read, size, pBuffer);
				 CallDisplayFunction(pBuffer, bytes_read, GdbSrvTextType::CommandOutput);
			}

			return bytes_read;
		}



public:
	int  get_char(void)
	{
		uint8_t ch;
		int ret;
		 
		return ch;
	}
	int Receive2(_Out_writes_bytes_(length) PCHAR pBuffer, _In_ int length)
	{
		 //printf("Recv1 Receive:%d\n", length);
		std::lock_guard<std::mutex> lock(m_mutex);
		auto bret = ReceiveImpl(pBuffer, length);
		// printf("Recv2 Receive:%d %d %s\n", bret, length, pBuffer);
		 
		return bret;
	}
 
		void IncInputStreamCharCounter()
		{
			readInputStreamCharCounter--;

		}
		int Peek(_Out_writes_bytes_(length) PCHAR pBuffer, _In_ int length, _In_ int flags) 
		{
			printf("Recv1 %x Peek Peek:%d\n", m_socket, length);

			return NO_ERROR; 
		}

		int SetOptions(_In_ int level, _In_ int optionName, _In_reads_opt_(optionLength) const char* pOptionVal,
			_In_ int optionLength) const
		{
			return setsockopt(m_socket, level, optionName, pOptionVal, optionLength);
		}

		int SetWSAIoctl(_In_ unsigned int dwIoControlCode, _In_reads_(inputLength) void* pInputBuffer,
			_In_ unsigned int inputLength, _Out_writes_opt_(outLength) void* pOutBuffer,
			_Out_ unsigned int outLength, long unsigned int* pBytesReturned) const
		{
			return WSAIoctl(m_socket, dwIoControlCode, pInputBuffer, inputLength, pOutBuffer, outLength,
				pBytesReturned, nullptr, nullptr);
		}

		int GetOptions(_In_ int level, _In_ int optionName, _Out_writes_(*pOptionLength)char* pOptionVal,
			_Inout_ int* pOptionLength) const
		{
			return getsockopt(m_socket, level, optionName, pOptionVal, pOptionLength);
		}

		bool Connect()
		{
			bool connectDone = false;

			if (::connect(m_socket, reinterpret_cast<struct sockaddr*>(&m_address), sizeof(m_address)) != SOCKET_ERROR)
			{
				connectDone = true;
			}
			return connectDone;
		}

		bool Close() const
		{
			int result = closesocket(m_socket);
			if (result == SOCKET_ERROR)
			{
				return false;
			}
			return true;
		}

		int Select(_Inout_opt_ fd_set* pReadfds, _Inout_opt_  fd_set* pWritefds,
			_Inout_opt_  fd_set* pExceptfds, _In_opt_ const struct timeval* pTimeout) const
		{
			if (pReadfds != nullptr)
			{
				FD_ZERO(pReadfds);
				FD_SET(m_socket, pReadfds);
			}
			if (pWritefds != nullptr)
			{
				FD_ZERO(pWritefds);
				FD_SET(m_socket, pWritefds);
			}
			if (pExceptfds != nullptr)
			{
				FD_ZERO(pExceptfds);
				FD_SET(m_socket, pExceptfds);
			}

			return select(0, pReadfds, pWritefds, pExceptfds, pTimeout);
		}

		inline int IsFDSet(_In_ fd_set* pFds) const
		{
			assert(pFds != nullptr);

			return FD_ISSET(m_socket, pFds);
		}

		inline int Ioctlsocket(_In_ long cmd, _Inout_ u_long* pArg) const
		{
			assert(pArg != nullptr);

			return ioctlsocket(m_socket, cmd, pArg);
		}

		inline void SetCallBackDisplayFunc(_In_ const pSetDisplayCommData function,
			_In_ IGdbSrvTextHandler* const pTextHandler)
		{
			m_pDisplayFunction = function;
			m_pTextHandler = pTextHandler;
		}

		inline void CallDisplayFunction(_In_ LPCSTR pBuffer, _In_ size_t len, _In_ GdbSrvTextType textType)
		{
			if (pBuffer != nullptr && m_pDisplayFunction != nullptr && m_pTextHandler != nullptr)
			{
				m_pDisplayFunction(pBuffer, len, textType, m_pTextHandler, m_channel);
			}
		}

		inline void CallDisplayFunction(_In_ LPCSTR pBuffer, _In_ GdbSrvTextType textType)
		{
			CallDisplayFunction(pBuffer, strlen(pBuffer), textType);
		}

		std::string getPeerIP() const { return m_peerIP; }
		USHORT getPeerPort() const { return m_peerPort; }

	private:
		std::unique_ptr<char> pReadInputStream = nullptr;
		int readInputStreamCharCounter = 0;
		char* pReadInputStreamBuffer = nullptr;
		int maximumPacketLength = 0;
		SOCKET               m_socket;
		pSetDisplayCommData  m_pDisplayFunction;
		IGdbSrvTextHandler* m_pTextHandler;
		std::string          m_peerIP;
		USHORT               m_peerPort;
		struct sockaddr_in   m_address;
		unsigned             m_channel;
		std::mutex           m_mutex;
		TcpIpStream(_In_ SOCKET sd, _In_ struct sockaddr_in* pAddress, _In_ unsigned channel);
	};

	//  The TcpConnectorStream class provides the connection mechanism to actively establish a connection with a server. 
	//  Basically, it's a factory class for producing connector objects when a client wants to connect to the server.
	//  The sending data over the network behaviour is implemented in the TcpIpStream class.
	//  Also, it initializes the WinSock library by the process.
	class TcpConnectorStream final
	{
	public:
		TcpConnectorStream(_In_ const std::vector<std::wstring>& coreConnectionParameters) : m_isInitiated(false),
			m_isConnected(false)
		{
			WSADATA wsaData = { 0 };
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
			{
				unsigned channel = 0;
				for (auto const& connectionStr : coreConnectionParameters)
				{
					m_pTLinkLayerStreamClass.push_back(TcpInitialize(connectionStr, channel));
					channel++;
				}
				if (m_pTLinkLayerStreamClass.size() == coreConnectionParameters.size())
				{
					m_isInitiated = true;
				}
			}
		}

		~TcpConnectorStream()
		{
			Close();
			if (m_isInitiated)
			{
				WSACleanup();
			}
		}

		inline bool TcpOpenStreamCore(_In_ const std::wstring& connectionStr, _In_ unsigned core)
		{
			try
			{
				if (m_pTLinkLayerStreamClass[core] == nullptr)
				{
					return false;
				}
				m_pTLinkLayerStreamClass[core] = TcpInitialize(connectionStr, core);
				return true;
			}
			CATCH_AND_RETURN_BOOLEAN
		}

		inline bool TcpConnectCore(_In_ unsigned maxAttempts, _In_ unsigned core)
		{
			return TcpConnectStream(m_pTLinkLayerStreamClass[core].get(), maxAttempts);
		}

		inline bool TcpCloseCore(_In_ unsigned core)
		{
			return TcpCloseStream(m_pTLinkLayerStreamClass[core].get());
		}


		bool Connect(unsigned int retries)
		{
			m_isConnected = TcpConnect(retries);
			return m_isConnected;
		}

		bool Close()
		{
			return TcpClose();
		}

		TcpIpStream* GetLinkLayerStream() const { return m_pTLinkLayerStreamClass[0].get(); }
		TcpIpStream* GetLinkLayerStreamEntry(size_t coreNumber) const
		{
			if (m_pTLinkLayerStreamClass.size() > 1)
			{
				return m_pTLinkLayerStreamClass[coreNumber].get();
			}
			return GetLinkLayerStream();
		}
		int GetLastError() const { return WSAGetLastError(); }
		bool IsConnected() const { return m_isConnected; }
		bool IsConnectionLost(int error) const { return IS_CONNECTION_LOST(error); }
		size_t GetNumberOfConnections() const { return m_pTLinkLayerStreamClass.size(); }

	private:
		std::vector<std::unique_ptr<TcpIpStream>> m_pTLinkLayerStreamClass;
		bool m_isInitiated;
		bool m_isConnected;

		std::unique_ptr<TcpIpStream> TcpInitialize(_In_ const std::wstring& connectionStr, _In_ unsigned channel);
		bool TcpConnect(_In_ unsigned int retries);
		bool TcpClose();
		bool ParseConnectString(_In_ LPCTSTR pConnect, _Out_writes_(hostNameLength) PSTR pHostName, _In_ ULONG hostNameLength,
			_Out_ USHORT* pPortNumber);
		int ResolveHostName(_In_z_ const char* pHostname, _Inout_ struct in_addr* pAddr);
		bool TcpConnectStream(_In_ TcpIpStream* const pTcpStream, _In_ unsigned int maxAttempts);
		bool TcpCloseStream(_In_ TcpIpStream* const pTcpStream);
	};
}