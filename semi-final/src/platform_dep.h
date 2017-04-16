#ifndef PLATFORM_DEP_H_INCLUDED
#define PLATFORM_DEP_H_INCLUDED

#ifdef WIN32
#	ifndef _WIN32_WINNT // Allow use of features specific to Windows XP or later.
#	define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#	endif
#	define _CRT_SECURE_NO_WARNINGS
#	include <winsock2.h>
#	undef min
#	undef max
#	include <stdexcept>
#	define socketerrno WSAGetLastError()
#else // WIN32
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>
#	include <unistd.h>
#	include <errno.h>
#	include <stdexcept>
#	define SOCKET int
#	define INVALID_SOCKET -1
#	define SOCKET_ERROR -1
#	define closesocket close
#	define socketerrno errno
#endif // WIN32

#if __cplusplus < 201103L

#include <sstream>
namespace std {
	template<class T>
	std::string to_string(T s) {
		std::stringstream ss;
		ss << s;
		return ss.str();
	}
}

#endif // __cplusplus < 201103L

namespace platform_dep {
	class enable_socket {
	public:
		enable_socket() {
#ifdef WIN32
			WSADATA WSAData;

			if(WSAStartup(0x101, &WSAData) != 0) {
				throw std::runtime_error("Error: Cannot able to use WINSOCK");
			}
#endif // WIN32
		}
		~enable_socket() {
#ifdef WIN32
			WSACleanup();
#endif // WIN32
		}
	};

	class tcp_socket {
		SOCKET handler;

	public:
		tcp_socket() {
			handler = socket(AF_INET, SOCK_STREAM, 0);
		}

		~tcp_socket() {
			invalidate();
		}

		bool valid() {
			return handler != INVALID_SOCKET;
		}

		SOCKET get_handler() {
			return handler;
		}

		void invalidate() {
			if(valid()) {
				closesocket(handler);
				handler = INVALID_SOCKET;
			}
		}
	};
}

#endif // PLATFORM_DEP_H_INCLUDED
