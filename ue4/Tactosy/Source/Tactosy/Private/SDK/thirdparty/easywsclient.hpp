#ifndef EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD
#define EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD

// This code comes from:
// https://github.com/dhbaird/easywsclient
//
// To get the latest version:
// wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.hpp
// wget https://raw.github.com/dhbaird/easywsclient/master/easywsclient.cpp


#ifdef _WIN32
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment( lib, "ws2_32" )
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <io.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef snprintf
#define snprintf _snprintf_s
#endif
#if _MSC_VER >=1600
// vs2010 or later
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#define socketerrno WSAGetLastError()
#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#else
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#ifndef _SOCKET_T_DEFINED
typedef int socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR   (-1)
#endif
#define closesocket(s) ::close(s)
#include <errno.h>
#define socketerrno errno
#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#endif

#include <string>
#include <vector>
//#include <vld.h>

namespace easywsclient {

    struct CallbackImp
    {
        virtual void operator()(const std::string& message) = 0;
    };
    struct BytesCallbackImp
    {
        virtual void operator()(const std::vector<uint8_t>& message) = 0;
    };

	struct CharCallbackImp
	{
		virtual void operator()(const char* message) = 0;
	};

    socket_t hostname_connect(const std::string& hostname, int port) {
        struct addrinfo hints;
        struct addrinfo *result;
        struct addrinfo *p;
        int ret;
        socket_t sockfd = INVALID_SOCKET;
        char sport[16];
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        snprintf(sport, 16, "%d", port);
        if ((ret = getaddrinfo(hostname.c_str(), sport, &hints, &result)) != 0)
        {
            fprintf(stderr, "getaddrinfo: failed\n");
            return 1;
        }
        for (p = result; p != nullptr; p = p->ai_next)
        {
            sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (sockfd == INVALID_SOCKET) { continue; }
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) != SOCKET_ERROR) {
                break;
            }
            closesocket(sockfd);
            sockfd = INVALID_SOCKET;
        }
        freeaddrinfo(result);



        return sockfd;
    }


    class WebSocket {
    public:
        typedef WebSocket * pointer;
        typedef enum readyStateValues { CLOSING, CLOSED, CONNECTING, OPEN } readyStateValues;

        // Factories:
        static pointer create(const std::string &host, int port, const std::string &path);
        // Interfaces:
        virtual ~WebSocket() { }
        virtual void poll(int timeout = 0) = 0; // timeout in milliseconds
        virtual void send(const std::string& message) = 0;
        virtual void sendBinary(const std::string& message) = 0;
        virtual void sendBinary(const std::vector<uint8_t>& message) = 0;
        virtual void sendPing() = 0;
        virtual void close() = 0;
        virtual readyStateValues getReadyState() const = 0;

        template<class Callable>
        void dispatch(Callable callable)
            // For callbacks that accept a string argument.
        { // N.B. this is compatible with both C++11 lambdas, functors and C function pointers
            struct _Callback : public CallbackImp {
                Callable& callable;
                _Callback(Callable& callable) : callable(callable) { }
                void operator()(const std::string& message) { callable(message); }
            };
            _Callback callback(callable);
            _dispatch(callback);
        }

		template<class Callable>
		void dispatchChar(Callable callable)
			// For callbacks that accept a Char argument.
		{ // N.B. this is compatible with both C++11 lambdas, functors and C function pointers
			struct _Callback : public CharCallbackImp {
				Callable& callable;
				_Callback(Callable& callable) : callable(callable) { }
				void operator()(const char* message) { callable(message); }
			};
			_Callback callback(callable);
			_dispatchChar(callback);
		}

        template<class Callable>
        void dispatchBinary(Callable callable)
            // For callbacks that accept a std::vector<uint8_t> argument.
        { // N.B. this is compatible with both C++11 lambdas, functors and C function pointers
            struct _Callback : public BytesCallbackImp {
                Callable& callable;
                _Callback(Callable& callable) : callable(callable) { }
                void operator()(const std::vector<uint8_t>& message) { callable(message); }
            };
            _Callback callback(callable);
            _dispatchBinary(callback);
        }

    protected:
        virtual void _dispatch(CallbackImp& callable) = 0;
        virtual void _dispatchBinary(BytesCallbackImp& callable) = 0;
		virtual void _dispatchChar(CharCallbackImp& callable) = 0;
    };

    class _RealWebSocket : public WebSocket
    {
    public:
        // http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
        //
        //  0                   1                   2                   3
        //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-------+-+-------------+-------------------------------+
        // |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
        // |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
        // |N|V|V|V|       |S|             |   (if payload len==126/127)   |
        // | |1|2|3|       |K|             |                               |
        // +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
        // |     Extended payload length continued, if payload len == 127  |
        // + - - - - - - - - - - - - - - - +-------------------------------+
        // |                               |Masking-key, if MASK set to 1  |
        // +-------------------------------+-------------------------------+
        // | Masking-key (continued)       |          Payload Data         |
        // +-------------------------------- - - - - - - - - - - - - - - - +
        // :                     Payload Data continued ...                :
        // + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        // |                     Payload Data continued ...                |
        // +---------------------------------------------------------------+
        struct wsheader_type {
            unsigned header_size;
            bool fin;
            bool mask;
            enum opcode_type {
                CONTINUATION = 0x0,
                TEXT_FRAME = 0x1,
                BINARY_FRAME = 0x2,
                CLOSE = 8,
                PING = 9,
                PONG = 0xa,
            } opcode;
            int N0;
            uint64_t N;
            uint8_t masking_key[4];
        };

//		std::mutex mtx;

        std::vector<uint8_t> rxbuf;
        std::vector<uint8_t> txbuf;
        std::vector<uint8_t> receivedData;

        socket_t sockfd;
        readyStateValues readyState;
        bool useMask;

        _RealWebSocket(socket_t sockfd) : sockfd(sockfd), readyState(OPEN), useMask(useMask) {
        }

        readyStateValues getReadyState() const {
            return readyState;
        }

        void poll(int timeout = 0) { // timeout in milliseconds
            if (readyState == CLOSED) {
                if (timeout > 0) {
                    timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
                    select(0, nullptr, nullptr, nullptr, &tv);
                }
                return;
            }
            if (timeout != 0) {
                fd_set rfds;
                fd_set wfds;
                timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
                FD_ZERO(&rfds);
                FD_ZERO(&wfds);
                FD_SET(sockfd, &rfds);
                if (txbuf.size()) { FD_SET(sockfd, &wfds); }
                select(sockfd + 1, &rfds, &wfds, 0, timeout > 0 ? &tv : 0);
            }
            while (true) {
                // FD_ISSET(0, &rfds) will be true
                int N = rxbuf.size();
                ssize_t ret;
                rxbuf.resize(N + 1500);
                ret = recv(sockfd, (char*)&rxbuf[0] + N, 1500, 0);
                if (false) {}
                else if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
                    rxbuf.resize(N);
                    break;
                }
                else if (ret <= 0) {
                    rxbuf.resize(N);
                    closesocket(sockfd);
                    readyState = CLOSED;
                    fputs(ret < 0 ? "recv Connection error!\n" : "recv Connection closed!\n", stderr);
                    break;
                }
                else {
                    rxbuf.resize(N + ret);
                }
            }
            while (txbuf.size()) {
                int ret = ::send(sockfd, (char*)&txbuf[0], txbuf.size(), 0);
                if (false) {} // ??
                else if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
                    break;
                }
                else if (ret <= 0) {
                    closesocket(sockfd);
                    readyState = CLOSED;

                    fputs(ret < 0 ? "tx Connection error!\n" : "tx Connection closed!\n", stderr);
                    break;
                }
                else {
                    txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
					std::vector<uint8_t>().swap(txbuf);
                }
            }
            if (!txbuf.size() && readyState == CLOSING) {
                closesocket(sockfd);
                readyState = CLOSED;
            }
        }

        // Callable must have signature: void(const std::string & message).
        // Should work with C functions, C++ functors, and C++11 std::function and
        // lambda:
        //template<class Callable>
        //void dispatch(Callable callable)
        virtual void _dispatch(CallbackImp & callable) {
            struct CallbackAdapter : public BytesCallbackImp
                // Adapt void(const std::string<uint8_t>&) to void(const std::string&)
            {
                CallbackImp& callable;
                CallbackAdapter(CallbackImp& callable) : callable(callable) { }
                void operator()(const std::vector<uint8_t>& message) {
                    std::string stringMessage(message.begin(), message.end());
					//printf("%s\n",stringMessage.c_str());
                    callable(stringMessage);
                }
            };
            CallbackAdapter bytesCallback(callable);
            _dispatchBinary(bytesCallback);
        }

		virtual void _dispatchChar(CharCallbackImp & callable) {
			struct CallbackAdapter : public BytesCallbackImp
				// Adapt void(const std::string<uint8_t>&) to void(const std::string&)
			{
				CharCallbackImp& callable;
				CallbackAdapter(CharCallbackImp& callable) : callable(callable) { }
				void operator()(const std::vector<uint8_t>& message) {
					std::string stringMessage(message.begin(), message.end());
					//printf("%s\n",stringMessage.c_str());
					callable(stringMessage.c_str());
				}
			};
			CallbackAdapter bytesCallback(callable);
			_dispatchBinary(bytesCallback);
		}

        virtual void _dispatchBinary(BytesCallbackImp & callable) {
            // TODO: consider acquiring a lock on rxbuf...
			//mtx.lock();
            while (true) {
                wsheader_type ws;
                
				if (rxbuf.size() < 2)
                {
//					mtx.unlock();
                    return; /* Need at least 2 */
                }

                const uint8_t * data = (uint8_t *)&rxbuf[0]; // peek, but don't consume
                ws.fin = (data[0] & 0x80) == 0x80;
                ws.opcode = (wsheader_type::opcode_type) (data[0] & 0x0f);
                ws.mask = (data[1] & 0x80) == 0x80;
                ws.N0 = (data[1] & 0x7f);
                ws.header_size = 2 + (ws.N0 == 126 ? 2 : 0) + (ws.N0 == 127 ? 8 : 0) + (ws.mask ? 4 : 0);
                if (rxbuf.size() < ws.header_size)
                {
//					mtx.unlock();
                    return; /* Need: ws.header_size - rxbuf.size() */
                }
                int i = 0;
                if (ws.N0 < 126) {
                    ws.N = ws.N0;
                    i = 2;
                }
                else if (ws.N0 == 126) {
                    ws.N = 0;
                    ws.N |= ((uint64_t)data[2]) << 8;
                    ws.N |= ((uint64_t)data[3]) << 0;
                    i = 4;
                }
                else if (ws.N0 == 127) {
                    ws.N = 0;
                    ws.N |= ((uint64_t)data[2]) << 56;
                    ws.N |= ((uint64_t)data[3]) << 48;
                    ws.N |= ((uint64_t)data[4]) << 40;
                    ws.N |= ((uint64_t)data[5]) << 32;
                    ws.N |= ((uint64_t)data[6]) << 24;
                    ws.N |= ((uint64_t)data[7]) << 16;
                    ws.N |= ((uint64_t)data[8]) << 8;
                    ws.N |= ((uint64_t)data[9]) << 0;
                    i = 10;
                }
                if (ws.mask) {
                    ws.masking_key[0] = ((uint8_t)data[i + 0]) << 0;
                    ws.masking_key[1] = ((uint8_t)data[i + 1]) << 0;
                    ws.masking_key[2] = ((uint8_t)data[i + 2]) << 0;
                    ws.masking_key[3] = ((uint8_t)data[i + 3]) << 0;
                }
                else {
                    ws.masking_key[0] = 0;
                    ws.masking_key[1] = 0;
                    ws.masking_key[2] = 0;
                    ws.masking_key[3] = 0;
                }

                if (rxbuf.size() < ws.header_size + ws.N)
                {
//					mtx.unlock();
                    return; /* Need: ws.header_size+ws.N - rxbuf.size() */
                }

                // We got a whole message, now do something with it:
                if (false) {}
                else if (
                    ws.opcode == wsheader_type::TEXT_FRAME
                    || ws.opcode == wsheader_type::BINARY_FRAME
                    || ws.opcode == wsheader_type::CONTINUATION
                    ) {
                    if (ws.mask)
                    {
                        for (size_t idx = 0; idx != ws.N; ++idx)
                        {
                            rxbuf[idx + ws.header_size] ^= ws.masking_key[idx & 0x3];
                        }
                    }
                    receivedData.insert(receivedData.end(), rxbuf.begin() + ws.header_size, rxbuf.begin() + ws.header_size + (size_t)ws.N);// just feed
                    if (ws.fin) {
                        callable((const std::vector<uint8_t>) receivedData);
                        receivedData.erase(receivedData.begin(), receivedData.end());
                        std::vector<uint8_t>().swap(receivedData);// free memory
                    }
                }
                else if (ws.opcode == wsheader_type::PING) {
                    if (ws.mask) {
                        for (size_t idx = 0; idx != ws.N; ++idx)
                        {
                            rxbuf[idx + ws.header_size] ^= ws.masking_key[idx & 0x3];
                        }
                    }
                    std::string dataStr(rxbuf.begin() + ws.header_size, rxbuf.begin() + ws.header_size + (size_t)ws.N);
                    sendData(wsheader_type::PONG, dataStr.size(), dataStr.begin(), dataStr.end());
                }
                else if (ws.opcode == wsheader_type::PONG) {}
                else if (ws.opcode == wsheader_type::CLOSE) { close(); }
                else { fprintf(stderr, "ERROR: Got unexpected WebSocket message.\n"); close(); }

                rxbuf.erase(rxbuf.begin(), rxbuf.begin() + ws.header_size + (size_t)ws.N);
            }
//			mtx.unlock();
        }

        void sendPing() {
            std::string empty;
            sendData(wsheader_type::PING, empty.size(), empty.begin(), empty.end());
        }

        void send(const std::string& message) {
            sendData(wsheader_type::TEXT_FRAME, message.size(), message.begin(), message.end());
        }

        void sendBinary(const std::string& message) {
            sendData(wsheader_type::BINARY_FRAME, message.size(), message.begin(), message.end());
        }

        void sendBinary(const std::vector<uint8_t>& message) {
            sendData(wsheader_type::BINARY_FRAME, message.size(), message.begin(), message.end());
        }

        template<class Iterator>
        void sendData(wsheader_type::opcode_type type, uint64_t message_size, Iterator message_begin, Iterator message_end) {
            // TODO:
            // Masking key should (must) be derived from a high quality random
            // number generator, to mitigate attacks on non-WebSocket friendly
            // middleware:
            const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
            // TODO: consider acquiring a lock on txbuf...
            if (readyState == CLOSING || readyState == CLOSED)
            {
                return;
            }
            std::vector<uint8_t> header;
            header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (useMask ? 4 : 0), 0);
            header[0] = 0x80 | type;
            if (false) {}
            else if (message_size < 126) {
                header[1] = (message_size & 0xff) | (useMask ? 0x80 : 0);
                if (useMask) {
                    header[2] = masking_key[0];
                    header[3] = masking_key[1];
                    header[4] = masking_key[2];
                    header[5] = masking_key[3];
                }
            }
            else if (message_size < 65536) {
                header[1] = 126 | (useMask ? 0x80 : 0);
                header[2] = (message_size >> 8) & 0xff;
                header[3] = (message_size >> 0) & 0xff;
                if (useMask) {
                    header[4] = masking_key[0];
                    header[5] = masking_key[1];
                    header[6] = masking_key[2];
                    header[7] = masking_key[3];
                }
            }
            else { // TODO: run coverage testing here
                header[1] = 127 | (useMask ? 0x80 : 0);
                header[2] = (message_size >> 56) & 0xff;
                header[3] = (message_size >> 48) & 0xff;
                header[4] = (message_size >> 40) & 0xff;
                header[5] = (message_size >> 32) & 0xff;
                header[6] = (message_size >> 24) & 0xff;
                header[7] = (message_size >> 16) & 0xff;
                header[8] = (message_size >> 8) & 0xff;
                header[9] = (message_size >> 0) & 0xff;
                if (useMask) {
                    header[10] = masking_key[0];
                    header[11] = masking_key[1];
                    header[12] = masking_key[2];
                    header[13] = masking_key[3];
                }
            }
            // N.B. - txbuf will keep growing until it can be transmitted over the socket:
            txbuf.insert(txbuf.end(), header.begin(), header.end());
            txbuf.insert(txbuf.end(), message_begin, message_end);
            if (useMask) {
                for (size_t i = 0; i != message_size; ++i)
                {
                    *(txbuf.end() - message_size + i) ^= masking_key[i & 0x3];
                }
            }
        }

        void close() {
            if (readyState == CLOSING || readyState == CLOSED)
            {
                return;
            }
            readyState = CLOSING;
            uint8_t closeFrame[6] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 }; // last 4 bytes are a masking key
            std::vector<uint8_t> header(closeFrame, closeFrame + 6);
            txbuf.insert(txbuf.end(), header.begin(), header.end());
        }

    };


    class _DummyWebSocket : public WebSocket
    {
    public:
        void poll(int timeout) { }
        void send(const std::string& message) { }
        void sendBinary(const std::string& message) { }
        void sendBinary(const std::vector<uint8_t>& message) { }
        void sendPing() { }
        void close() { }
        readyStateValues getReadyState() const { return CLOSED; }
        void _dispatch(CallbackImp & callable) { }
        void _dispatchBinary(BytesCallbackImp& callable) { }
		void _dispatchChar(CharCallbackImp & callable) { }
    };

    WebSocket::pointer  WebSocket::create(const std::string &hosts, int port, const std::string &_path) {

        const char *host= hosts.c_str();
        const char *path = _path.c_str();
        std::string origin = "";

        socket_t sockfd = hostname_connect(host, port);
        if (sockfd == INVALID_SOCKET) {
            fprintf(stderr, "Unable to connect to %s:%d\n", host, port);
            return nullptr;
        }
        {
            // XXX: this should be done non-blocking,
            char line[256];
            int i;
            snprintf(line, 256, "GET /%s HTTP/1.1\r\n", path); ::send(sockfd, line, strlen(line), 0);
            if (port == 80) {
                snprintf(line, 256, "Host: %s\r\n", host); ::send(sockfd, line, strlen(line), 0);
            }
            else {
                snprintf(line, 256, "Host: %s:%d\r\n", host, port); ::send(sockfd, line, strlen(line), 0);
            }
            snprintf(line, 256, "Upgrade: websocket\r\n"); ::send(sockfd, line, strlen(line), 0);
            snprintf(line, 256, "Connection: Upgrade\r\n"); ::send(sockfd, line, strlen(line), 0);
            if (!origin.empty()) {
                snprintf(line, 256, "Origin: %s\r\n", origin.c_str()); ::send(sockfd, line, strlen(line), 0);
            }
            snprintf(line, 256, "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"); ::send(sockfd, line, strlen(line), 0);
            snprintf(line, 256, "Sec-WebSocket-Version: 13\r\n"); ::send(sockfd, line, strlen(line), 0);
            snprintf(line, 256, "\r\n"); ::send(sockfd, line, strlen(line), 0);
            for (i = 0; i < 2 || (i < 255 && line[i - 2] != '\r' && line[i - 1] != '\n'); ++i)
            {
                if (recv(sockfd, line + i, 1, 0) == 0)
                {
                    return nullptr;
                }
            }
            line[i] = 0;
            if (i == 255)
            {
                fprintf(stderr, "ERROR: Got invalid status line connecting to: %s\n", "ws://127.0.0.1:15881/feedbackBytes");
                return nullptr;
            }
//            int status;
//            if (sscanf(line, "HTTP/1.1 %d", &status) != 1 || status != 101)
//            {
//                fprintf(stderr, "ERROR: Got bad status connecting to %s: %s", "ws://127.0.0.1:15881/feedbackBytes", line);
//                return nullptr;
//            }
            // TODO: verify response headers,
            while (true) {
                for (i = 0; i < 2 || (i < 255 && line[i - 2] != '\r' && line[i - 1] != '\n'); ++i) {
                    if (recv(sockfd, line + i, 1, 0) == 0)
                    {
                        return nullptr;
                    }
                }
                if (line[0] == '\r' && line[1] == '\n')
                {
                    break;
                }
            }
        }
        int flag = 1;
        setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // Disable Nagle's algorithm
#ifdef _WIN32
        u_long on = 1;
        ioctlsocket(sockfd, FIONBIO, &on);
#else
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif
        fprintf(stderr, "Connected to: ws://%s:%d/%s\n", host, port, path);
        return pointer(new _RealWebSocket(sockfd));
    }
} // namespace easywsclient

#endif /* EASYWSCLIENT_HPP_20120819_MIOFVASDTNUASZDQPLFD */
