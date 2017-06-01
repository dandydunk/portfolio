#include "stdafx.h"
#include "powebservice.h"
#include <vector>
#include <MacUtils.h>

std::string powebservice::Url = "";
int(*powebservice::Logger)(std::string text) = NULL;

powebservice::powebservice()
{
}

powebservice::~powebservice(){
}

int powebservice::GetVersion(std::string & version) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=version");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		version.clear();
		version.append(data);

		curl_easy_cleanup(curl);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::LastWhiteListUpdate(std::string & timestamp) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=whitelist_last_updated");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		timestamp.clear();
		timestamp.append(data);
		curl_easy_cleanup(curl);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetHTML2(std::string urlToQuery, std::string & html) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		
		curl_easy_setopt(curl, CURLOPT_URL, urlToQuery.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36");

		powebservice::Log("connecting to [" + urlToQuery + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		html.clear();
		html.append(data);
		curl_easy_cleanup(curl);

	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetHTML(std::string urlToQuery, std::string httpRequest, std::string & html) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=get_html&url=" + urlToQuery);
		std::string hr = curl_easy_escape(curl, httpRequest.c_str(), httpRequest.length());

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36");
		
		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		html.clear();
		html.append(data);
		curl_easy_cleanup(curl);

	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::LastBlackListUpdate(std::string & timestamp) {
	CURL *curl = curl_easy_init();
	if (curl) {
		char errBuf[CURL_ERROR_SIZE];
		std::string data;
		std::string url(powebservice::Url + "?action=blacklist_last_updated");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errBuf);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			powebservice::Log("curl_easy_perform error["+std::string(errBuf)+"]");
			return PO_WS_CURL_ERROR;
		}

		timestamp.clear();
		timestamp.append(data);

		curl_easy_cleanup(curl);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetCert(std::string host, std::string & cert) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=get_cert&host="+host);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		powebservice::Log("data[" + data + "]");
		cert.clear();
		cert.append(data);

		curl_easy_cleanup(curl);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetKey(std::string host, std::string & cert) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=get_key&host=" + host);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		powebservice::Log("data[" + data + "]");
		cert.clear();
		cert.append(data);

		curl_easy_cleanup(curl);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetBlackList(std::vector<UrlPattern> & matches) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=blacklist");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		curl_easy_cleanup(curl);

		data.append("\r\n");
		powebservice::Log("data[" + data + "]");

		size_t index = 0;
		size_t startExtract = 0;
		do {
			index = data.find("\r\n", index);
			if (index == std::string::npos) break;

			std::string line = data.substr(startExtract, index - startExtract);

			index += 2;
			startExtract = index;

			/* split line by spaces */
			size_t index2 = line.find(" ");
			if (index2 == std::string::npos) continue;

			UrlPattern wm;
			wm.pattern= line.substr(0, index2);
			wm.redirectUrl = line.substr(index2+1);

			matches.push_back(wm);
		} while (true);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

int powebservice::GetWhiteList(std::vector<UrlPattern> & matches) {
	CURL *curl = curl_easy_init();
	if (curl) {
		std::string data;
		std::string url(powebservice::Url + "?action=whitelist");
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, powebservice::GetCurlData);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&data);

		powebservice::Log("connecting to [" + url + "]");
		CURLcode curlCode = curl_easy_perform(curl);
		if (curlCode != CURLE_OK) {
			return PO_WS_CURL_ERROR;
		}

		curl_easy_cleanup(curl);

		data.append("\r\n");
		powebservice::Log("data[" + data + "]");

		size_t index = 0;
		size_t startExtract = 0;
		do {
			index = data.find("\r\n", index);
			if (index == std::string::npos) break;

			std::string line = data.substr(startExtract, index - startExtract);

			index += 2;
			startExtract = index;

			std::vector<std::string> lineMatches;
			MacUtils::StrSplit<std::string>(line, ' ', lineMatches);
			if (lineMatches.size() != 3) continue;

			UrlPattern wm;
			wm.host = lineMatches[0];
			wm.pattern = lineMatches[1];
			wm.redirectUrl = lineMatches[2];

			matches.push_back(wm);
		} while (true);
	}

	else {
		return PO_WS_CURL_ERROR;
	}

	return 0;
}

size_t powebservice::GetCurlData(char *buffer, size_t size, size_t nmemb, void *data) {
	std::string *cpData = (std::string*)data;
	size_t bufferSize = size * nmemb;
	cpData->append(buffer, bufferSize);
	//memcpy(&((*cpData)[0]), buffer, bufferSize);

	return bufferSize;
}