#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <map>
#include <Logger.h>
#include <regex>
#include <openssl\ssl.h>
#ifndef HEADER_DH_H
#include <openssl/dh.h>
#endif

#define DS_ERROR_SUCCESS 0
#define DS_ERROR_MISSING_PORT 1
#define DS_ERROR_SOCKET 2
#define DS_ERROR_ADDR 3
#define DS_ERROR_THREAD 4



#define PACKET_ID_CHECK_DEST 1
#define PACKET_ID_SSL 2

#pragma comment (lib, "Ws2_32.lib")


struct UrlPattern {
	std::string host;
	std::string pattern;
	std::string redirectUrl;
};

struct SSLConnection {
	SSL* ssl;
	SSL_CTX* ctx;
	char *sid;
	BIO* read;
	BIO* write;
	std::string host;
	bool foundKey;
	UINT64 flowId;
	int status;
	bool redirect;
	bool byPass;
	SSL_CTX *clientCtx;
	SSL *clientSsl;
	BIO *clientBio;
	std::vector<char> httpsBuffer;
	size_t httpsBufferLen;
	bool foundHeader;
	bool isGet;
	int contentLen;
};

class DriverServer {
public:
	std::string port;
	SOCKET socketListen;
	bool serverGoing;
	bool stopServer;
	static Logger *MLogger;
	static int extaDataIndex;
	static std::string writeDir;
	HANDLE mutexWhiteList;
	static HANDLE mutexLogger;
	std::vector<UrlPattern> urlPatterns;
	HANDLE mutexBlackList;
	std::vector<UrlPattern> blackList;
	ULONG countConnectedClients;
	std::map<UINT64, SSLConnection*> mapSSLConnections;
public:
	DriverServer();
	~DriverServer();
	void ReadClientBIO(SSLConnection *sslConn, SOCKET socket);
	bool SendHttpsData(SOCKET socket, SSLConnection *sslConn, char *buffer, int bufferLen, int packetType);
	void CreateSSLObj(SSLConnection *sslConn, std::string host);
	int IsHttpHeaderComplete(std::vector<char> & buffer);
	std::map<std::string, std::string> GetHttpHeaders(std::string http);
	int Init();
	static void Log(std::string text);
	bool ShouldRedirect(std::string url, std::string & redirectUrl);
	void Stop();
	bool IsBlackListed(std::string url);
	void UpdateBlackList(std::vector<UrlPattern> list);
	void UpdateWhiteList(std::vector<UrlPattern> list);
	void PacketCheckDestSSL(SOCKET socket, UINT64 flowId, char *data, SIZE_T dataSize);
	void SendSSLData(SOCKET socket, int redirect, char *data, SIZE_T dataSize);
	SSLConnection* MakeServer();
	static void ssl_servername_callback(SSL *ssl, int *al, void *context);
	static void SetCtx(SSLConnection* sslConn);
	void UpdateCerts(std::vector<UrlPattern> list);
	void CleanUpSSLConnection(SSLConnection **sslConn);
public:
	static DWORD WINAPI ThreadProcServerListen(_In_ LPVOID lpParameter);
	static DWORD WINAPI ThreadProcServerClient(_In_ LPVOID lpParameter);

public:
	void PacketCheckDest(SOCKET socket, std::string url);
};

struct ClientThreadParams {
	SOCKET socket;
	DriverServer *driverServer;
};

