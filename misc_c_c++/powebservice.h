#pragma once
#include <string>
#include <curl/curl.h>
#include <vector>
#include "DriverServer.h"

#define PO_WS_CURL_ERROR 1


class powebservice {
public:
	powebservice();
	~powebservice();

public:
	static std::string Url;
	static int (*Logger)(std::string text);
	static void Log(std::string text) {
		if (powebservice::Logger != NULL) {
			powebservice::Logger(text);
		}
	}
public:
	static int GetVersion(std::string & version);
	static size_t GetCurlData(char *buffer, size_t size, size_t nmemb, void *userp);
	static int LastBlackListUpdate(std::string & timestamp);
	static int LastWhiteListUpdate(std::string & timestamp);
	static int GetWhiteList(std::vector<UrlPattern> & matches);
	static int GetBlackList(std::vector<UrlPattern> & matches);
	static int GetKey(std::string host, std::string & cert);
	static int GetCert(std::string host, std::string & cert);
	static int GetHTML(std::string urlToQuery, std::string httpRequest, std::string & html);
	static int GetHTML2(std::string urlToQuery, std::string & html);
};

