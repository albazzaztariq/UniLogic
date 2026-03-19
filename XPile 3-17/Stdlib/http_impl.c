// Stdlib/http_impl.c — HTTP client FFI for UniLogic
// Windows: WinHTTP API (no external deps)
// Unix: POSIX sockets (no external deps)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

typedef struct {
    char* data;
    int   len;
    int   cap;
} HttpBuf;

static void buf_init(HttpBuf* b) {
    b->cap = 4096;
    b->data = (char*)malloc(b->cap);
    b->len = 0;
    b->data[0] = '\0';
}

static void buf_append(HttpBuf* b, const char* chunk, int n) {
    while (b->len + n + 1 > b->cap) {
        b->cap *= 2;
        b->data = (char*)realloc(b->data, b->cap);
    }
    memcpy(b->data + b->len, chunk, n);
    b->len += n;
    b->data[b->len] = '\0';
}

static char* winhttp_request(const char* url, const char* method, const char* body) {
    // Parse URL
    wchar_t wurl[2048];
    MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, 2048);

    URL_COMPONENTS uc;
    memset(&uc, 0, sizeof(uc));
    uc.dwStructSize = sizeof(uc);
    wchar_t host[256], path[1024];
    uc.lpszHostName = host; uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path; uc.dwUrlPathLength = 1024;
    if (!WinHttpCrackUrl(wurl, 0, 0, &uc)) return strdup("");

    HINTERNET session = WinHttpOpen(L"UniLogic/0.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return strdup("");

    HINTERNET conn = WinHttpConnect(session, host, uc.nPort, 0);
    if (!conn) { WinHttpCloseHandle(session); return strdup(""); }

    wchar_t wmethod[16];
    MultiByteToWideChar(CP_UTF8, 0, method, -1, wmethod, 16);
    DWORD flags = (uc.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET req = WinHttpOpenRequest(conn, wmethod, path, NULL,
                                        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!req) { WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return strdup(""); }

    // Set timeout
    int timeout = 30000;
    WinHttpSetTimeouts(req, timeout, timeout, timeout, timeout);

    BOOL ok;
    if (body && strlen(body) > 0) {
        ok = WinHttpSendRequest(req, L"Content-Type: application/json\r\n", -1L,
                                 (LPVOID)body, (DWORD)strlen(body), (DWORD)strlen(body), 0);
    } else {
        ok = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }
    if (!ok) { WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return strdup(""); }

    ok = WinHttpReceiveResponse(req, NULL);
    if (!ok) { WinHttpCloseHandle(req); WinHttpCloseHandle(conn); WinHttpCloseHandle(session); return strdup(""); }

    HttpBuf buf;
    buf_init(&buf);
    DWORD n = 0;
    char chunk[4096];
    while (WinHttpReadData(req, chunk, sizeof(chunk), &n) && n > 0) {
        buf_append(&buf, chunk, n);
        n = 0;
    }

    WinHttpCloseHandle(req);
    WinHttpCloseHandle(conn);
    WinHttpCloseHandle(session);
    return buf.data;
}

char* http_get(const char* url) {
    return winhttp_request(url, "GET", NULL);
}

char* http_post(const char* url, const char* body) {
    return winhttp_request(url, "POST", body);
}

char* http_get_json(const char* url) {
    return winhttp_request(url, "GET", NULL);
}

#else
// ── Unix: POSIX sockets ─────────────────────────────────────────────────

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

static char* socket_request(const char* url, const char* method, const char* body) {
    // Parse URL: http://host[:port]/path
    if (strncmp(url, "http://", 7) != 0) return strdup("");
    const char* hp = url + 7;
    const char* slash = strchr(hp, '/');
    const char* path = slash ? slash : "/";

    char host[256];
    int port = 80;
    int hlen = slash ? (int)(slash - hp) : (int)strlen(hp);
    if (hlen > 255) hlen = 255;
    memcpy(host, hp, hlen);
    host[hlen] = '\0';

    char* colon = strchr(host, ':');
    if (colon) { *colon = '\0'; port = atoi(colon + 1); }

    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    char ports[8];
    snprintf(ports, sizeof(ports), "%d", port);
    if (getaddrinfo(host, ports, &hints, &res) != 0) return strdup("");

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); return strdup(""); }

    // 30s timeout
    struct timeval tv = {30, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
        close(fd); freeaddrinfo(res); return strdup("");
    }
    freeaddrinfo(res);

    // Build request
    char req[4096];
    int rlen;
    if (body && strlen(body) > 0) {
        rlen = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\n"
            "Content-Length: %d\r\nConnection: close\r\n\r\n%s",
            method, path, host, (int)strlen(body), body);
    } else {
        rlen = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
            method, path, host);
    }
    send(fd, req, rlen, 0);

    // Read response
    int cap = 8192, total = 0;
    char* buf = (char*)malloc(cap);
    int n;
    while ((n = recv(fd, buf + total, cap - total - 1, 0)) > 0) {
        total += n;
        if (total >= cap - 1) { cap *= 2; buf = (char*)realloc(buf, cap); }
    }
    buf[total] = '\0';
    close(fd);

    // Strip headers: find \r\n\r\n
    char* body_start = strstr(buf, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        char* result = strdup(body_start);
        free(buf);
        return result;
    }
    return buf;
}

char* http_get(const char* url) {
    return socket_request(url, "GET", NULL);
}

char* http_post(const char* url, const char* body) {
    return socket_request(url, "POST", body);
}

char* http_get_json(const char* url) {
    return socket_request(url, "GET", NULL);
}

#endif
