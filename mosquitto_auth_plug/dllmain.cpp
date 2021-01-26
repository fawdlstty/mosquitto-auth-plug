#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "Wldap32.lib")

#pragma comment (lib, "mosquitto.lib")
#pragma comment (lib, "mosquittopp.lib")



BOOL APIENTRY DllMain (HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}



extern "C" {
#define EOS '\0'
#define FNM_NOMATCH 1
#define FNM_PATHNAME 2
#define FNM_PERIOD 4
#define FNM_NOESCAPE 1

	char* strsep1 (char** str, const char* delims) {
		char* token;


		if (!*str) {
			/* No more tokens */
			return NULL;
		}


		token = *str;
		while (**str != '\0') {
			if (strchr (delims, **str)) {
				**str = '\0';
				(*str)++;
				return token;
			}
			(*str)++;
		}


		/* There is no other token */
		*str = NULL;


		return token;
	}

	static const char* rangematch1 (const char* pattern, int test, int flags) {
		int negate, ok;
		char c, c2;

		/*
		 * A bracket expression starting with an unquoted circumflex
		 * character produces unspecified results (IEEE 1003.2-1992,
		 * 3.13.2).  This implementation treats it like '!', for
		 * consistency with the regular expression syntax.
		 * J.T. Conklin (conklin@ngai.kaleida.com)
		 */
		if (negate = (*pattern == '!' || *pattern == '^'))
			++pattern;

		for (ok = 0; (c = *pattern++) != ']';) {
			if (c == '\\' && !(flags & FNM_NOESCAPE))
				c = *pattern++;
			if (c == EOS)
				return (NULL);
			if (*pattern == '-'
				&& (c2 = *(pattern + 1)) != EOS && c2 != ']') {
				pattern += 2;
				if (c2 == '\\' && !(flags & FNM_NOESCAPE))
					c2 = *pattern++;
				if (c2 == EOS)
					return (NULL);
				if (c <= test && test <= c2)
					ok = 1;
			} else if (c == test)
				ok = 1;
		}
		return (ok == negate ? NULL : pattern);
	}

	int fnmatch1 (const char* pattern, const char* string, int flags) {
		const char* stringstart;
		char c, test;

		for (stringstart = string;;)
			switch (c = *pattern++) {
			case EOS:
				return (*string == EOS ? 0 : FNM_NOMATCH);
			case '?':
				if (*string == EOS)
					return (FNM_NOMATCH);
				if (*string == '/' && (flags & FNM_PATHNAME))
					return (FNM_NOMATCH);
				if (*string == '.' && (flags & FNM_PERIOD) &&
					(string == stringstart ||
						((flags & FNM_PATHNAME) && *(string - 1) == '/')))
					return (FNM_NOMATCH);
				++string;
				break;
			case '*':
				c = *pattern;
				/* Collapse multiple stars. */
				while (c == '*')
					c = *++pattern;

				if (*string == '.' && (flags & FNM_PERIOD) &&
					(string == stringstart ||
						((flags & FNM_PATHNAME) && *(string - 1) == '/')))
					return (FNM_NOMATCH);

				/* Optimize for pattern with * at end or before /. */
				if (c == EOS)
					if (flags & FNM_PATHNAME)
						return (strchr (string, '/') == NULL ?
							0 : FNM_NOMATCH);
					else
						return (0);
				else if (c == '/' && flags & FNM_PATHNAME) {
					if ((string = strchr (string, '/')) == NULL)
						return (FNM_NOMATCH);
					break;
				}

				/* General case, use recursion. */
				while ((test = *string) != EOS) {
					if (!fnmatch1 (pattern, string, flags & ~FNM_PERIOD))
						return (0);
					if (test == '/' && flags & FNM_PATHNAME)
						break;
					++string;
				}
				return (FNM_NOMATCH);
			case '[':
				if (*string == EOS)
					return (FNM_NOMATCH);
				if (*string == '/' && flags & FNM_PATHNAME)
					return (FNM_NOMATCH);
				if ((pattern =
					rangematch1 (pattern, *string, flags)) == NULL)
					return (FNM_NOMATCH);
				++string;
				break;
			case '\\':
				if (!(flags & FNM_NOESCAPE)) {
					if ((c = *pattern++) == EOS) {
						c = '\\';
						--pattern;
					}
				}
				/* FALLTHROUGH */
			default:
				if (c != *string++)
					return (FNM_NOMATCH);
				break;
			}
		/* NOTREACHED */
	}

	//#include "D:/GitHub/mosquitto/lib/mosquitto_internal.h"

	typedef const char* (*mosquitto_client_id_t) (const struct mosquitto *client);
	typedef void* (*mosquitto_client_certificate_t) (const struct mosquitto *client);
	typedef const char* (*mosquitto_client_username_t) (const struct mosquitto *client);

	static mosquitto_client_id_t s_mosquitto_client_id = nullptr;
	static mosquitto_client_certificate_t s_mosquitto_client_certificate = nullptr;
	static mosquitto_client_username_t s_mosquitto_client_username = nullptr;

	void _init_mosquitto_module () {
		HMODULE _hModule = ::GetModuleHandle (nullptr);
		s_mosquitto_client_id = (mosquitto_client_id_t) ::GetProcAddress (_hModule, "mosquitto_client_id");
		s_mosquitto_client_certificate = (mosquitto_client_certificate_t) ::GetProcAddress (_hModule, "mosquitto_client_certificate");
		s_mosquitto_client_username = (mosquitto_client_username_t) ::GetProcAddress (_hModule, "mosquitto_client_username");
	}

	const char* mosquitto_client_id (const struct mosquitto* client) {
		if (s_mosquitto_client_id == nullptr)
			_init_mosquitto_module ();
		if (s_mosquitto_client_id)
			return s_mosquitto_client_id (client);
		else
			return nullptr;
	}

	void* mosquitto_client_certificate (const struct mosquitto* client) {
		if (s_mosquitto_client_certificate == nullptr)
			_init_mosquitto_module ();
		if (s_mosquitto_client_certificate)
			return s_mosquitto_client_certificate (client);
		else
			return nullptr;
	}

	const char* mosquitto_client_username (const struct mosquitto* client) {
		if (s_mosquitto_client_username == nullptr)
			_init_mosquitto_module ();
		if (s_mosquitto_client_username)
			return s_mosquitto_client_username (client);
		else
			return nullptr;
	}
}
