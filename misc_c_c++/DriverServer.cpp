#include "stdafx.h"
#include "DriverServer.h"
#include "powebservice.h"
#include <MacUtils.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <map>


Logger *DriverServer::MLogger;
std::string DriverServer::writeDir;
int DriverServer::extaDataIndex;
HANDLE DriverServer::mutexLogger;

DriverServer::DriverServer() {
	this->port = "";
	DriverServer::extaDataIndex = 0;
	this->stopServer = false;
	this->serverGoing = false;
	DriverServer::MLogger = NULL;
	this->countConnectedClients = 0;
	this->mutexBlackList = CreateMutex(NULL, FALSE, L"rihur efveijiueireurh");
	this->mutexWhiteList = CreateMutex(NULL, FALSE, L"rihuriuevvj0ijoieireurh");
}

void DriverServer::Log(std::string text) {
	if (DriverServer::MLogger) {
		
		WaitForSingleObject(DriverServer::mutexLogger, INFINITE);
		DriverServer::MLogger->Write(text);
		ReleaseMutex(DriverServer::mutexLogger);
	}
}

int DriverServer::Init() {
	DriverServer::Log("SERVER INIT");

	if (this->port.empty()) {
		return DS_ERROR_MISSING_PORT;
	}

	this->Stop();

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	int iResult = getaddrinfo(NULL, this->port.c_str(), &hints, &result);
	if (iResult != 0) {
		return DS_ERROR_ADDR;
	}

	//DriverServer::Log("making socket...");
	this->socketListen = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (this->socketListen == INVALID_SOCKET) {
		freeaddrinfo(result);
		return DS_ERROR_SOCKET;
	}

	//DriverServer::Log("binding...");
	iResult = bind(this->socketListen, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(this->socketListen);
		return DS_ERROR_SOCKET;
	}

	freeaddrinfo(result);

	//DriverServer::Log("listening...");
	iResult = listen(this->socketListen, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		closesocket(this->socketListen);
		return DS_ERROR_SOCKET;
	}

	DriverServer::extaDataIndex = SSL_get_ex_new_index(0, "dovecot", NULL, NULL, NULL);

	this->stopServer = FALSE;
	HANDLE thread = CreateThread(NULL, 0,
								DriverServer::ThreadProcServerListen,
								this, 0, NULL);

	if (!thread) {
		return DS_ERROR_THREAD;
	}

	return DS_ERROR_SUCCESS;
}

void DriverServer::UpdateBlackList(std::vector<UrlPattern> list) {
	WaitForSingleObject(this->mutexBlackList, INFINITE);

	this->blackList.clear();
	this->blackList.insert(this->blackList.end(), list.begin(), list.end());

	ReleaseMutex(this->mutexBlackList);
}

void DriverServer::UpdateCerts(std::vector<UrlPattern> list) {

}

void DriverServer::UpdateWhiteList(std::vector<UrlPattern> list) {
	WaitForSingleObject(this->mutexWhiteList, INFINITE);

	this->urlPatterns.clear();
	this->urlPatterns.insert(this->urlPatterns.end(), list.begin(), list.end());
	
	for (int i = 0; i < this->urlPatterns.size(); i++) {
		UrlPattern up = this->urlPatterns[i];
		std::string safeHost = up.host;
		std::replace(safeHost.begin(), safeHost.end(), '.', '_');

		std::string certPath = DriverServer::writeDir + "certs\\" + safeHost + ".crt";
		if (!PathFileExistsA(certPath.c_str())) {
			std::string certText;
			int curlResult = powebservice::GetCert(up.host, certText);
			if (!curlResult) {
				MacUtils::WriteToFileA(certPath, certText);
			}
		}

		std::string keyPath = DriverServer::writeDir + "certs\\" + safeHost + ".key";
		if (!PathFileExistsA(keyPath.c_str())) {
			std::string keyText;
			int curlResult = powebservice::GetKey(up.host, keyText);
			if (!curlResult) {
				MacUtils::WriteToFileA(keyPath, keyText);
			}
		}
	}

	ReleaseMutex(this->mutexWhiteList);
}

bool DriverServer::IsBlackListed(std::string url) {
	std::cmatch narrowMatch;
	bool r = FALSE;

	WaitForSingleObject(this->mutexBlackList, INFINITE);
	for (int i = 0; i < this->blackList.size(); i++) {
		UrlPattern urlPattern = this->blackList[i];
		std::regex rx(urlPattern.pattern);

		bool found = std::regex_search(url.c_str(), rx);
		if (found) {
			r = TRUE;
			break;
		}
	}
	ReleaseMutex(this->mutexBlackList);

	return r;
}


bool DriverServer::ShouldRedirect(std::string url, std::string & redirectUrl) {
	std::cmatch narrowMatch;
	bool r = FALSE;

	if (this->IsBlackListed(url)) return r;

	WaitForSingleObject(this->mutexWhiteList, INFINITE);

	//DriverServer::Log("looping [" + MacUtils::NumberToString<size_t>(this->urlPatterns.size()) + "] patterns");
	DriverServer::Log("checking regex for[" + url + "]");
	for (int i = 0; i < this->urlPatterns.size(); i++) {
		UrlPattern urlPattern = this->urlPatterns[i];
		std::regex rx(urlPattern.pattern);

		bool found = std::regex_search(url.c_str(), rx);
		
		if (found) {
			//DriverServer::Log("matched!!");
			redirectUrl.clear();
			redirectUrl.append(urlPattern.redirectUrl);
			r = TRUE;
			break;
		}
	}

	ReleaseMutex(this->mutexWhiteList);

	return r;
}

std::map<std::string, std::string> DriverServer::GetHttpHeaders(std::string http) {
	std::map<std::string, std::string> mapHeaders;
	int httpIndex = 0;
	int index = 0;
	while (true) {
		if ((index = http.find("\r\n", index)) == std::string::npos) {
			break;
		}

		std::string line = http.substr(httpIndex, index - httpIndex);
		index += 2;
		httpIndex = index;

		/*  break up the line */
		int lineSpace = line.find(" ");
		if (lineSpace == std::string::npos) {
			/* corrupted header*/
			break;
		}

		std::string title = line.substr(0, lineSpace);
		std::string text = line.substr(lineSpace + 1);

		mapHeaders[title] = text;
	}

	return mapHeaders;
}

void DriverServer::CreateSSLObj(SSLConnection *sslConn, std::string host) {
	SSL_CTX *sslCtx = SSL_CTX_new(SSLv23_client_method());
	if (NULL == sslCtx) {
		DriverServer::Log("ssl_ctx_new failed");
		return;
	}

	SSL_CTX_set_verify(sslCtx, SSL_VERIFY_NONE, NULL);

	BIO* bio = BIO_new_ssl_connect(sslCtx);
	if (!bio) {
		DriverServer::Log("bio_new_ssl_connect failed");
		return;
	}

	SSL *ssl = NULL;
	BIO_get_ssl(bio, &ssl);

	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	SSL_set_cipher_list(ssl, "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS");
	SSL_set_tlsext_host_name(ssl, host.c_str());
	//BIO_set_nbio(bio, 1);

	std::string hostPort = host + ":443";
	BIO_set_conn_hostname(bio, hostPort.c_str());

	//DriverServer::Log("connecting... to ["+host+"]");
	do {
		int ret = BIO_do_connect(bio);
		unsigned long ssl_err = ERR_get_error();
		if (ret <= 0) {
			if (BIO_should_retry(bio)) {
				DriverServer::Log("retrying the connection to["+host+"]...");
				continue;
			}
			else {
				//DriverServer::Log("BIO_do_connect error [" + MacUtils::NumberToString<long>( ssl_err) + "]");
				if (!ssl_err) {
					continue;
				}
				else {
					DriverServer::Log("BIO_do_connect error hosts["+host+"] [" + std::string(ERR_reason_error_string(ssl_err)) + "]");
					return;
				}
			}
		}
		break;
	} while (1);

	//DriverServer::Log("doing the hand shake for host["+host+"]...");
	do {
		long lRet = BIO_do_handshake(bio);
		if (lRet <= 0) {
			if (BIO_should_retry(bio)) {
				DriverServer::Log("retrying the hand shake for host[" + host + "]...");
				continue;
			}
			else {
				DriverServer::Log("hand shake error for host[" + host + "]...");
				return;
			}
		}
		break;
	} while (1);

	sslConn->clientCtx = sslCtx;
	sslConn->clientSsl = ssl;
	sslConn->clientBio = bio;
}

void DriverServer::ssl_servername_callback(SSL *ssl, int *al, void *context) {
	const char *host;

	host = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	SSLConnection *sslConn = (SSLConnection*)SSL_get_ex_data(ssl, DriverServer::extaDataIndex);
	sslConn->host = host;
	std::string cpHost(host);
	std::replace(cpHost.begin(), cpHost.end(), '.', '_');
	//DriverServer::Log("server host name from cert[" + std::string(host) + "]");
	DriverServer::Log("HOST NAME[" + std::string(host) + "]SSL connection[" + MacUtils::NumberToString<UINT64>(sslConn->flowId)+"]");

	DriverServer::SetCtx(sslConn);
	std::string pemFilePath = DriverServer::writeDir + "certs\\" + std::string(cpHost) + ".key";
	int status = SSL_CTX_use_PrivateKey_file(sslConn->ctx, pemFilePath.c_str(), SSL_FILETYPE_PEM);
	if (1 != status) {
		//DriverServer::Log("SSL_CTX_use_PrivateKey_file failed for ["+pemFilePath+"]");
		int err = 0;// SSL_get_error(sslConn->ssl, status);
		while ((err = ERR_get_error()) != 0) {
			//DriverServer::Log("SSL_CTX_use_PrivateKey_file[" + std::string(ERR_error_string(err, NULL)) + "][" + MacUtils::NumberToString<int>(status)+"] error[" + MacUtils::NumberToString<int>(err) +"] failed");
		}
		return;
	}

	std::string keyFilePath = DriverServer::writeDir + "certs\\" + std::string(cpHost) + ".crt";
	status = SSL_CTX_use_certificate_file(sslConn->ctx, keyFilePath.c_str(), SSL_FILETYPE_PEM);
	if (1 != status) {
		//DriverServer::Log("SSL_CTX_use_certificate_file failed for ["+keyFilePath+"]");
		int err = 0;// SSL_get_error(sslConn->ssl, status);
		while ((err = ERR_get_error()) != 0) {
			//DriverServer::Log("SSL_CTX_use_certificate_file[" + std::string(ERR_error_string(err, NULL)) + "][" + MacUtils::NumberToString<int>(status)+"] error[" + MacUtils::NumberToString<int>(err) +"] failed");

		}
		return;
	}
	
	SSL_set_SSL_CTX(ssl, sslConn->ctx);

	sslConn->foundKey = true;
	//strHost.re
}

void DriverServer::SetCtx(SSLConnection* sslConn) {
	//DriverServer::Log("making new SSL object...");
	sslConn->ctx = SSL_CTX_new(SSLv23_server_method());
	if (!sslConn->ctx) {
		DriverServer::Log("SSL_CTX_new failed");
		return;
	}


	//DriverServer::Log("---setting CTX...");
	if (1 != SSL_CTX_set_cipher_list(sslConn->ctx, "ALL:")) {
		DriverServer::Log("SSL_CTX_set_cipher_list failed");
		return;
	}

	DH *dh_2048 = NULL;
	FILE *paramfile = NULL;
	std::string s(DriverServer::writeDir + "dh_param_2048.pem");
	fopen_s(&paramfile, s.c_str(), "r");
	if (paramfile) {
		dh_2048 = PEM_read_DHparams(paramfile, NULL, NULL, NULL);
		fclose(paramfile);
	}
	else {
		DriverServer::Log("fopen_s dh file failed");
		return;
	}

	if (dh_2048 == NULL) {
		DriverServer::Log("dh_2048 was null");
		return;
	}
	if (SSL_CTX_set_tmp_dh(sslConn->ctx, dh_2048) != 1) {
		DriverServer::Log("SSL_CTX_set_tmp_dh failed");
		return;
	}

	SSL_CTX_set_options(sslConn->ctx, SSL_OP_SINGLE_DH_USE);
	//SSL_CTX_set_tlsext_servername_callback(sslConn->ctx, DriverServer::ssl_servername_callback);

	//DriverServer::Log("making the SID...");
	SSL_CTX_set_session_id_context(sslConn->ctx, (unsigned char*)sslConn->sid, strlen(sslConn->sid));
}

void DriverServer::CleanUpSSLConnection(SSLConnection **psslConn) {
	SSLConnection *sslConn = *psslConn;

	if (sslConn->ssl) {
		SSL_free(sslConn->ssl);
		sslConn->ssl = NULL;
	}

	if (sslConn->ctx) {
		SSL_CTX_free(sslConn->ctx);
		sslConn->ctx = NULL;
	}

	
	if (sslConn->clientSsl) {
		SSL_free(sslConn->clientSsl);
		sslConn->clientSsl = NULL;
	}

	if (sslConn->clientCtx) {
		SSL_CTX_free(sslConn->clientCtx);
		sslConn->clientCtx = NULL;
	}

	/*
	if (sslConn->clientBio) {
		BIO_free_all(sslConn->clientBio);
		sslConn->clientBio = NULL;
	}*/

	this->mapSSLConnections[sslConn->flowId] = NULL;
	delete sslConn;
}

int DriverServer::IsHttpHeaderComplete(std::vector<char> & buffer) {
	for (size_t i = 0; i < buffer.size(); i++) {
		if ((i + 3) >= buffer.size()) {
			return -1;
		}

		if (buffer[i] == '\r' && buffer[i + 1] == '\n' && buffer[i + 2] == '\r' && buffer[i + 3] == '\n') {
			return i + 3;
		}
	}

	return -1;
}

void DriverServer::PacketCheckDestSSL(SOCKET socket, UINT64 flowId, char *data, SIZE_T dataSize) {
	SSLConnection *sslConn;
	int status = 0;

	if (this->mapSSLConnections.find(flowId) == this->mapSSLConnections.end() || !this->mapSSLConnections[flowId]) {
		DriverServer::Log("NEW SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		sslConn = new SSLConnection;
		ZeroMemory(sslConn, sizeof(SSLConnection));
		this->mapSSLConnections[flowId] = sslConn;

		sslConn->foundKey = false;
		sslConn->flowId = flowId;
		sslConn->byPass = false;
		sslConn->clientBio = NULL;
		sslConn->clientCtx = NULL;
		sslConn->clientSsl = NULL;

		++sslConn->status;
	}
	else {
		DriverServer::Log("FOUND SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		sslConn = this->mapSSLConnections[flowId];
		++sslConn->status;
	}

	if (sslConn->byPass) {
		DriverServer::Log("BYPASS SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		this->SendSSLData(socket, 0, NULL, 0);
		return;
	}

	if (!sslConn->ssl) {
		DriverServer::Log("MAKING SSL OBJ connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		const SSL_METHOD *sslMethod = SSLv23_server_method();
		if (!sslMethod) {
			this->CleanUpSSLConnection(&sslConn);
			DriverServer::Log("TLS_server_method failed");
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}

		sslConn->ctx = SSL_CTX_new(sslMethod);
		if (!sslConn->ctx) {
			this->CleanUpSSLConnection(&sslConn);
			DriverServer::Log("SSL_CTX_new failed");
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}

		SSL_CTX_set_tlsext_servername_callback(sslConn->ctx, DriverServer::ssl_servername_callback);

		sslConn->ssl = SSL_new(sslConn->ctx);
		if (!sslConn->ssl) {
			this->CleanUpSSLConnection(&sslConn);
			DriverServer::Log("SSL_new failed");
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}
		SSL_set_accept_state(sslConn->ssl);
		SSL_set_verify(sslConn->ssl, SSL_VERIFY_NONE, NULL);

		//DriverServer::Log("making the SID...");
		std::string flowIdStr = MacUtils::NumberToString<UINT64>(flowId);
		sslConn->sid = new char[flowIdStr.length() + 1];
		memcpy_s(sslConn->sid, flowIdStr.length() + 1, flowIdStr.c_str(), flowIdStr.length());
		sslConn->sid[flowIdStr.length()] = '\0';
		SSL_set_session_id_context(sslConn->ssl, (unsigned char*)sslConn->sid, strlen(sslConn->sid));
		SSL_set_ex_data(sslConn->ssl, DriverServer::extaDataIndex, (void*)sslConn);

		//DriverServer::Log("Bio_New read");
		sslConn->read = BIO_new(BIO_s_mem());
		if (!sslConn->read) {
			this->CleanUpSSLConnection(&sslConn);
			DriverServer::Log("BIO_new read failed");
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}


		//BIO_set_nbio(sslConn->read, 1);

		//DriverServer::Log("Bio_New write");
		sslConn->write = BIO_new(BIO_s_mem());
		if (!sslConn->read) {
			this->CleanUpSSLConnection(&sslConn);
			DriverServer::Log("BIO_new write failed");
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}
		//BIO_set_nbio(sslConn->write, 1);
		SSL_set_bio(sslConn->ssl, sslConn->read, sslConn->write);
		//SSL_do_handshake(sslConn->ssl);

		
	}

	status = BIO_write(sslConn->read, data, dataSize);
	if (status <= 0) {
		DriverServer::Log("BIO_write failed");
		this->SendSSLData(socket, 0, NULL, 0);
		return;
	}

	if (!SSL_is_init_finished(sslConn->ssl)) {
		DriverServer::Log("HAND SHAKE NOT FINISHED---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		status = SSL_do_handshake(sslConn->ssl);
		if (status <= 0) {
			int err = 0;// SSL_get_error(sslConn->ssl, status);
			while ((err = ERR_get_error()) != 0) {
				//DriverServer::Log("SSL_do_handshake[" + std::string(ERR_error_string(err, NULL)) + "][" + MacUtils::NumberToString<int>(status)+"] error[" + MacUtils::NumberToString<int>(err) +"] failed");
			}

			//this->SendSSLData(socket, 0, NULL, 0);
			//return;
		}

		if (!sslConn->foundKey) {
			//DriverServer::Log("couldn't find a key...");
			DriverServer::Log("NO KEY---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
			sslConn->byPass = true;
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}

		char buffer[9999];

		//DriverServer::Log("Bio_read...");
		status = BIO_read(sslConn->write, buffer, 9999);
		if (status <= 0) {
			DriverServer::Log("BIO_read[" + MacUtils::NumberToString<int>(status)+"] failed");
			sslConn->byPass = true;
			this->SendSSLData(socket, 0, NULL, 0);
			return;
		}
		else {
			//DriverServer::Log("Bio_read read [" + MacUtils::NumberToString<int>(status)+"]...");
			this->SendSSLData(socket, 1, buffer, status);
		}
	}

	else {
		DriverServer::Log("HAND SHAKE FINISHED---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		int pendingBytes = 1024;
		do {
			char *buffer = new char[pendingBytes];
			status = SSL_read(sslConn->ssl, buffer, pendingBytes);
			if (status <= 0) {
				DriverServer::Log("SSL_read failed");
				this->CleanUpSSLConnection(&sslConn);
				this->SendSSLData(socket, -1, NULL, 0);
				delete buffer;
				return;
			}

			int httpsBufferSize = sslConn->httpsBuffer.size();
			sslConn->httpsBuffer.resize(httpsBufferSize + status);
			memcpy(&(sslConn->httpsBuffer[httpsBufferSize]), buffer, status);

			delete buffer;

			pendingBytes = SSL_pending(sslConn->ssl);
			if (pendingBytes <= 0) {
				break;
			}
		} while (1);

		if (!sslConn->foundHeader) {
			DriverServer::Log("FIND HEADER---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
			int endOfHeaderPos = this->IsHttpHeaderComplete(sslConn->httpsBuffer);
			if (-1 == endOfHeaderPos) {
				DriverServer::Log("FIND HEADER---MORE DATA NEEDED---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
				this->SendSSLData(socket, 3, NULL, 0);
				return;
			}

			else {
				sslConn->foundHeader = true;
				std::string httpBuf;
				httpBuf.resize(endOfHeaderPos);
				memcpy(&httpBuf[0], &(sslConn->httpsBuffer[0]), endOfHeaderPos);
				DriverServer::Log("HEADER---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"] ["+httpBuf+"]");
				std::map<std::string, std::string> mapHeaders = this->GetHttpHeaders(httpBuf);

				std::string lineGet;
				if (mapHeaders.find("GET") != mapHeaders.end()) {
					lineGet = mapHeaders["GET"];
					sslConn->isGet = true;
				}

				else if (mapHeaders.find("POST") != mapHeaders.end()) {
					lineGet = mapHeaders["POST"];
					sslConn->isGet = false;
				}

				else {
					this->CleanUpSSLConnection(&sslConn);
					this->SendSSLData(socket, -1, NULL, 0);
					return;
				}

				int lineGetSpace = lineGet.find(" ");
				if (lineGetSpace == std::string::npos) {
					this->CleanUpSSLConnection(&sslConn);
					this->SendSSLData(socket, -1, NULL, 0);
					return;
				}

				std::string requestPath = lineGet.substr(0, lineGetSpace);

				if (mapHeaders.find("Host:") == mapHeaders.end()) {
					this->CleanUpSSLConnection(&sslConn);
					this->SendSSLData(socket, -1, NULL, 0);
					return;
				}

				if (mapHeaders.find("Content Length:") != mapHeaders.end()) {
					sslConn->contentLen = atoi(mapHeaders["Content Length:"].c_str());
				}

				std::string requestUrl = "https://" + mapHeaders["Host:"] + requestPath;
				std::string redirectUrl;
				std::string responseBuffer;
				DriverServer::Log("URL["+requestUrl+"]---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
				if (this->ShouldRedirect(requestUrl, redirectUrl)) {
					responseBuffer = "HTTP/1.1 301 Moved Permanently\r\nConnection: close\r\nLocation: ";
					responseBuffer.append(redirectUrl);
					responseBuffer.append("\r\n\r\n");
					//DriverServer::Log(responseBuffer);

					DriverServer::Log("REDIRECT URL[" + redirectUrl + "]---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
					this->SendHttpsData(socket, sslConn, (char*)responseBuffer.c_str(), responseBuffer.size(), 4);
					this->CleanUpSSLConnection(&sslConn);
					return;
				}

				this->CreateSSLObj(sslConn, mapHeaders["Host:"]);
				if (!sslConn->clientBio) {
					DriverServer::Log("ERROR creating SSL object...---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
					this->CleanUpSSLConnection(&sslConn);
					this->SendSSLData(socket, -1, NULL, 0);
					return;
				}

				BIO_write(sslConn->clientBio, &(sslConn->httpsBuffer[0]), sslConn->httpsBuffer.size());
				sslConn->httpsBufferLen += sslConn->httpsBuffer.size() - endOfHeaderPos;
				sslConn->httpsBuffer.clear();
			}
		}

		else {
			BIO_write(sslConn->clientBio, &(sslConn->httpsBuffer[0]), sslConn->httpsBuffer.size());
			sslConn->httpsBufferLen += sslConn->httpsBuffer.size();
			sslConn->httpsBuffer.clear();
		}
		
		if (sslConn->httpsBufferLen < sslConn->contentLen) {
			DriverServer::Log("sslConn->httpsBufferLen < sslConn->contentLen---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
			this->SendSSLData(socket, 3, NULL, 0);
			return;
		}

		DriverServer::Log("ReadClientBIO---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		this->ReadClientBIO(sslConn, socket);

		DriverServer::Log("CleanUpSSLConnection---SSL connection[" + MacUtils::NumberToString<UINT64>(flowId)+"]");
		this->CleanUpSSLConnection(&sslConn);
	}
}

void DriverServer::ReadClientBIO(SSLConnection *sslConn, SOCKET socket) {
	std::string headers;
	do {
		char buf[1];
		int bytesRead = BIO_read(sslConn->clientBio, buf, 1);
		if (bytesRead <= 0) {
			DriverServer::Log("BIO_read Error reading from the client...");
			this->CleanUpSSLConnection(&sslConn);
			this->SendSSLData(socket, -1, NULL, 0);
			return;
		}

		headers.push_back(buf[0]);
		if (buf[0] == '\n') {
			size_t s = headers.size();
			if (s >= 4) {
				if (headers[s - 4] == '\r' && headers[s - 3] == '\n' && headers[s - 2] == '\r' && headers[s - 1] == '\n') {
					break;
				}
			}
		}
	} while (1);

	//DriverServer::Log("client headers["+headers+"]");

	std::map<std::string, std::string> mapHeaders = GetHttpHeaders(headers);
	if (mapHeaders.find("Content-Length:") != mapHeaders.end()) {
		int len = atoi(mapHeaders["Content-Length:"].c_str());
		if (!len) {
			this->SendHttpsData(socket, sslConn, (char*)headers.c_str(), headers.size(), 5);
			return;
		}
		else {
			this->SendHttpsData(socket, sslConn, (char*)headers.c_str(), headers.size(), 2);
		}
		int totalBytesRead = 0;
		
		do {
			int bufLen = len - totalBytesRead;
			char *buf = new char[bufLen];
			int bytesRead = BIO_read(sslConn->clientBio, buf, bufLen);
			if (bytesRead <= 0) {
				delete buf;
				break;
			}

			totalBytesRead += bytesRead;
			if (totalBytesRead >= len) {
				if (!this->SendHttpsData(socket, sslConn, buf, bytesRead, 5)) {
					delete buf;
					return;
				}
				delete buf;
				break;
			}
			else {
				if (!this->SendHttpsData(socket, sslConn, buf, bytesRead, 2)) {
					delete buf;
					return;
				}
			}

			delete buf;
		} while (true);
	}

	else if (mapHeaders.find("Transfer-Encoding:") != mapHeaders.end()) {
		int startIndex = 0;
		std::string strLen;

		this->SendHttpsData(socket, sslConn, (char*)headers.c_str(), headers.size(), 2);
		
		do {
			char buf[1];
			int bytesRead = BIO_read(sslConn->clientBio, buf, 1);
			if (bytesRead <= 0) {
				break;
			}

			if (buf[0] == '\r') {
				char * p;
				long chunkLen = strtol(strLen.c_str(), &p, 16);
				if (*p != 0) {
					break;
				}
				//DriverServer::Log("got chunk size[" + MacUtils::NumberToString<long>(chunkLen)+"]");

				strLen.push_back(buf[0]);
				
				std::vector<char> vBuf;
				if (!chunkLen) {
					strLen.append("\n\r\n");
					if (!this->SendHttpsData(socket, sslConn, (char*)strLen.c_str(), strLen.size(), 5)) {
						return;
					}

					return;
				}
				else {
					vBuf.resize(strLen.size());
					memcpy(&vBuf[0], strLen.c_str(), strLen.size());
				}
				strLen.clear();

				int buf2LenTotal = 1 + chunkLen + 2;
				int totalBytesRead = 0;
				bool readError = false;
				do {
					int buf2Len = buf2LenTotal - totalBytesRead;
					char *buf2 = new char[buf2Len];
					int bytesRead = BIO_read(sslConn->clientBio, buf2, buf2Len);
					if (bytesRead <= 0) {
						delete buf2;
						readError = true;
						break;
					}

					size_t vBufSize = vBuf.size();
					vBuf.resize(vBufSize + bytesRead);
					memcpy(&vBuf[vBufSize], buf2, bytesRead);
					
					delete buf2;
					totalBytesRead += bytesRead;
					if (totalBytesRead >= buf2LenTotal) break;
				} while (1);

				if (readError) {
					break;
				}

				if (!this->SendHttpsData(socket, sslConn, &vBuf[0], vBuf.size(), 2)) {
					return;
				}
			}
			else {
				strLen.push_back(buf[0]);
			}
		} while (1);
	}

	else {
		this->SendHttpsData(socket, sslConn, (char*)headers.c_str(), headers.size(), 5);
	}
}

bool DriverServer::SendHttpsData(SOCKET socket, SSLConnection *sslConn, char *buffer, int bufferLen, int packetType) {
	//DriverServer::Log("writing response data...");
	int status = SSL_write(sslConn->ssl, buffer, bufferLen);
	if (status <= 0) {
		DriverServer::Log("SSL_write response data failed");
		this->CleanUpSSLConnection(&sslConn);
		this->SendSSLData(socket, -1, NULL, 0);
		return false;
	}

	int pendingDataCount = BIO_pending(sslConn->write);
	//DriverServer::Log("BIO_pending encoded data[" + MacUtils::NumberToString<int>(pendingDataCount)+"]");
	if (pendingDataCount <= 0) {
		this->CleanUpSSLConnection(&sslConn);
		this->SendSSLData(socket, -1, NULL, 0);
		return false;
	}

	int totalBytesRead = 0;
	do {
		int encodedBufferLen = pendingDataCount - totalBytesRead;
		char *encodedBuffer = new char[encodedBufferLen];
		status = BIO_read(sslConn->write, encodedBuffer, encodedBufferLen);
		if (status <= 0) {
			delete encodedBuffer;
			DriverServer::Log("BIO_read encoded data[" + MacUtils::NumberToString<int>(status)+"] failed");
			this->CleanUpSSLConnection(&sslConn);
			this->SendSSLData(socket, -1, NULL, 0);
			return false;
		}
		
		//DriverServer::Log("Bio_read read --- sending [" + MacUtils::NumberToString<int>(status)+"]...");
		
		this->SendSSLData(socket, packetType, encodedBuffer, status);

		delete encodedBuffer;
		totalBytesRead += status;
		if (totalBytesRead >= pendingDataCount) break;
	} while (1);

	return true;
}

void DriverServer::SendSSLData(SOCKET socket, int redirect, char *data, SIZE_T dataSize) {
	char *packet;
	int packetSize = 0;
	int packetIndex = 0;
	int packetId = htonl(PACKET_ID_CHECK_DEST);

	
	packetSize = sizeof(packetId) + sizeof(redirect);
	if (dataSize) {
		packetSize += sizeof(dataSize) + dataSize;
	}

	packet = new char[packetSize];

	/* packet id */
	//DriverServer::Log("SendSSLData...packeting the packet id...");
	memcpy(&packet[packetIndex], &packetId, sizeof(packetId));
	packetIndex += sizeof(packetId);

	/* should redirect*/
	//DriverServer::Log("SendSSLData...packeting the should redirect["+MacUtils::NumberToString<int>(redirect)+"] result...");
	int nRedirect = htonl(redirect);
	memcpy(&packet[packetIndex], &nRedirect, sizeof(nRedirect));
	packetIndex += sizeof(nRedirect);

	if (dataSize) {
		/* data size*/
		//DriverServer::Log("SendSSLData...packeting the data size["+MacUtils::NumberToString<SIZE_T>(dataSize)+"]...");
		SIZE_T nDataSize = htonl(dataSize);
		memcpy(&packet[packetIndex], &nDataSize, sizeof(nDataSize));
		packetIndex += sizeof(dataSize);

		/* data */
		//DriverServer::Log("SendSSLData...packeting the data...");
		memcpy(&packet[packetIndex], data, dataSize);
		packetIndex += dataSize;
	}

	/* send to client*/
	int bytesSent = 0;
	int totalBytesSent = 0;
	do {
		//DriverServer::Log("sending...");
		bytesSent = send(socket, &packet[totalBytesSent], packetSize - totalBytesSent, 0);
		if (SOCKET_ERROR == bytesSent) {
			DriverServer::Log("socket error sending SSL packet");
			break;
		}
		totalBytesSent += bytesSent;
	} while (totalBytesSent < packetSize);

	//DriverServer::Log("SENT [" + MacUtils::NumberToString<SIZE_T>(totalBytesSent)+"] bytes...");
	delete packet;
}

void DriverServer::PacketCheckDest(SOCKET socket, std::string url) {
	std::string redirectUrl;
	int result = 0;
	
	if (url.length()) {
		result = this->ShouldRedirect(url, redirectUrl) ? htonl(1) : htonl(0);
	}

	int packetSize = 0;
	int packetId = htonl(PACKET_ID_CHECK_DEST);
	int redirectUrlLength = htonl(redirectUrl.length());

	packetSize = sizeof(packetId) + sizeof(result);

	if (result) {
		packetSize += sizeof(redirectUrlLength);
		packetSize += (int)redirectUrl.length();
	}

	char* packet = new char[packetSize];

	unsigned int packetIndex = 0;
	memcpy(&packet[packetIndex], &packetId, sizeof(packetId));
	packetIndex += sizeof(packetId);
	
	memcpy(&packet[packetIndex], &result, sizeof(result));
	if (result) {
		packetIndex += sizeof(result);
		memcpy(&packet[packetIndex], &redirectUrlLength, sizeof(redirectUrlLength));

		packetIndex += sizeof(redirectUrlLength);
		memcpy(&packet[packetIndex], redirectUrl.c_str(), redirectUrl.length());

		std::string l = "redirecting to [" + redirectUrl + "]";
		DriverServer::Log("redirecting to [" + redirectUrl + "]");
	}

	int bytesSent = 0;
	int totalBytesSent = 0;
	do {
		bytesSent = send(socket, &packet[totalBytesSent], packetSize - totalBytesSent, 0);
		if (SOCKET_ERROR == bytesSent) {
			DriverServer::Log("socket error sending");
			break;
		}
		if (!bytesSent) {
			break;
		}
		totalBytesSent += bytesSent;
	} while (totalBytesSent < packetSize);

	delete packet;
}

void DriverServer::Stop() {
	this->stopServer = TRUE;

	closesocket(this->socketListen);

	while (this->serverGoing) {
		Sleep(100);
	}

	while (this->countConnectedClients > 0) {
		Sleep(100);
	}

	
	//CRYPTO_cleanup_all_ex_data();
}

DWORD WINAPI DriverServer::ThreadProcServerListen(_In_ LPVOID lpParameter)  {
	DriverServer *driverServer = (DriverServer*)lpParameter;

	driverServer->serverGoing = true;
	while (!driverServer->stopServer) {
		driverServer->Log("waiting for client to connect...");
		SOCKET clientSocket = accept(driverServer->socketListen, NULL, NULL);
		if (clientSocket != INVALID_SOCKET) {
			driverServer->Log("client connected...");

			ClientThreadParams *p = new ClientThreadParams;
			p->socket = clientSocket;
			p->driverServer = driverServer;
			HANDLE thread = CreateThread(NULL, 0, DriverServer::ThreadProcServerClient, (LPVOID)p, 0, NULL);
		}
	}

	driverServer->Log("driver server listen exiting...");
	driverServer->serverGoing = false;
	return 0;
}

DWORD WINAPI DriverServer::ThreadProcServerClient(_In_ LPVOID lpParameter)  {
	ClientThreadParams *params = (ClientThreadParams*)lpParameter;

	InterlockedIncrement(&(params->driverServer->countConnectedClients));

	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();

	do {
		int packetId = 0;
		//DriverServer::Log("waiting for packet ID...");
		int bytesRecv = recv(params->socket, (char*)&packetId, sizeof(packetId), 0);
		if (!bytesRecv) {
			break;
		}

		if (SOCKET_ERROR == bytesRecv) {
			params->driverServer->Log("error recv packet id");
			break;
		}

		packetId = ntohl(packetId);
		//DriverServer::Log("packet id[" + MacUtils::NumberToString<int>(packetId) +"]");
		bool exitLoop = false;
		switch (packetId) {
			case PACKET_ID_SSL: {
				//DriverServer::Log("PACKET_ID_SSL---geting ip address...");
				
				/* flowId*/
				UINT64 flowId = 0;
				bytesRecv = recv(params->socket, (char*)&flowId, sizeof(flowId), 0);
				if (SOCKET_ERROR == bytesRecv) {
					params->driverServer->Log("error recv flow id for SSL packet");
					exitLoop = true;
					break;
				}

				else if (!bytesRecv) {
					exitLoop = true;
					break;
				}

				flowId = ntohll(flowId);
				//DriverServer::Log("port[" + MacUtils::NumberToString<UINT16>(port)+"]...getting process id...");


				/* process id */
				UINT64 processId = 0;
				bytesRecv = recv(params->socket, (char*)&processId, sizeof(processId), 0);
				if (SOCKET_ERROR == bytesRecv) {
					DriverServer::Log("error recv process id for SSL packet");
					exitLoop = true;
					break;
				}

				else if (!bytesRecv) {
					exitLoop = true;
					break;
				}

				processId = ntohll(processId);
				//DriverServer::Log("processId[" + MacUtils::NumberToString<UINT64>(processId)+"]...getting data length...");

				/* data length */
				SIZE_T dataLength = 0;
				bytesRecv = recv(params->socket, (char*)&dataLength, sizeof(dataLength), 0);
				if (SOCKET_ERROR == bytesRecv) {
					DriverServer::Log("error recv data length for SSL packet");
					exitLoop = true;
					break;
				}

				else if (!bytesRecv) {
					exitLoop = true;
					break;
				}

				dataLength = ntohl(dataLength);
				//DriverServer::Log("data length[" + MacUtils::NumberToString<SIZE_T>(dataLength)+"]...getting data...");

				/* data */
				char *data = new char[dataLength];
				bytesRecv = recv(params->socket, data, dataLength, 0);
				if (SOCKET_ERROR == bytesRecv) {
					DriverServer::Log("error recv data for SSL packet");
					exitLoop = true;
					break;
				}

				else if (!bytesRecv) {
					exitLoop = true;
					break;
				}

				if (processId == GetCurrentProcessId()) {
					//params->driverServer->Log("this process requesting internet...");
					params->driverServer->SendSSLData(params->socket, 0, NULL, 0);
				}

				else {
					//params->driverServer->Log("PacketCheckDestSSL");
					params->driverServer->PacketCheckDestSSL(params->socket, flowId, data, dataLength);
				}

				delete[] data;
			}
			break;
			
			case PACKET_ID_CHECK_DEST: {
				params->driverServer->Log("getting the url length...");
				int lenUrl = 0;
				bytesRecv = recv(params->socket, (char*)&lenUrl, sizeof(lenUrl), 0);
				if (SOCKET_ERROR == bytesRecv) {
					params->driverServer->Log("error recv packet url");
					exitLoop = true;
					break;
				}
				else if (!bytesRecv) {
					exitLoop = true;
					break;
				}

				lenUrl = ntohl(lenUrl);

				char *url = new char[lenUrl + 1];
				params->driverServer->Log("getting the url...");
				bytesRecv = recv(params->socket, url, lenUrl, 0);
				if (SOCKET_ERROR == bytesRecv) {
					params->driverServer->Log("error recv packet url");
					exitLoop = true;
					break;
				}

				else if (bytesRecv > 0) {
					url[lenUrl] = '\0';

					params->driverServer->Log(std::string("checking HTTP url[") + url + std::string("]..."));
					params->driverServer->PacketCheckDest(params->socket, url);
				}
				else {
					exitLoop = true;
				}

				delete url;
			}
			break;
		}

		if (exitLoop) break;
	} while (params->driverServer->serverGoing);

	closesocket(params->socket);

	ERR_remove_state(0);
	ENGINE_cleanup();
	CONF_modules_unload(1);
	ERR_free_strings();
	EVP_cleanup();
	//sk_SSL_COMP_free(SSL_COMP_get_compression_methods());

	InterlockedDecrement(&(params->driverServer->countConnectedClients));

	delete params;
	return 0;
}


DriverServer::~DriverServer()
{
}
