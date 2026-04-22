/*
 * test_curl_http_status.c
 * Extracted from curl lib/http.c — HTTP status code classification logic.
 * Pure integer arithmetic and switch/case. No pointers, no malloc.
 */
#include <stdio.h>

/* Classify HTTP status code into category string */
static const char *http_status_category(int code)
{
    switch (code / 100) {
    case 1:
        return "informational";
    case 2:
        return "success";
    case 3:
        return "redirect";
    case 4:
        return "client_error";
    case 5:
        return "server_error";
    default:
        return "unknown";
    }
}

/* Classify specific well-known HTTP codes */
static int http_status_is_redirect(int code)
{
    switch (code) {
    case 301:
    case 302:
    case 303:
    case 307:
    case 308:
        return 1;
    default:
        return 0;
    }
}

/* Return 1 if code means "continue processing" (1xx) */
static int http_status_is_informational(int code)
{
    return (code >= 100 && code < 200) ? 1 : 0;
}

/* Return 1 if code means "success" */
static int http_status_is_success(int code)
{
    return (code >= 200 && code < 300) ? 1 : 0;
}

/* Compute retry-after hint: 429 or 503 = should retry */
static int http_should_retry(int code)
{
    if (code == 429 || code == 503)
        return 1;
    return 0;
}

int main(void)
{
    int codes[] = {100, 101, 200, 201, 204, 206, 301, 302, 303, 304,
                   307, 308, 400, 401, 403, 404, 405, 408, 429, 500,
                   502, 503, 504, 0, 999};
    int n = 25;
    int i;

    printf("=== HTTP Status Classification ===\n");
    for (i = 0; i < n; i++) {
        int c = codes[i];
        printf("code=%d cat=%s redirect=%d info=%d success=%d retry=%d\n",
               c,
               http_status_category(c),
               http_status_is_redirect(c),
               http_status_is_informational(c),
               http_status_is_success(c),
               http_should_retry(c));
    }

    printf("=== Range Tests ===\n");
    /* Test boundary values */
    printf("199/100=%d 200/100=%d 299/100=%d 300/100=%d\n",
           199/100, 200/100, 299/100, 300/100);

    return 0;
}
