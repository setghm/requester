#include "http_response.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// requester
#include "http_headers.h"
#include <http_default.h>
#include <utils/utils.h>

HttpResponse* HttpResponse_New(void) {
    HttpResponse* res = (HttpResponse*)malloc(sizeof(HttpResponse));

    if (res) {
        memset(res, 0, sizeof(HttpResponse));

        res->headers = StringMap_New();
    }

    return res;
}

void HttpResponse_Delete(HttpResponse* res) {
    RETURN_IF_NULL(res);

    StringMap_Delete(res->headers);
    HttpContent_Delete(res->content);

    free(res);
}

char* HttpResponse_StartLineToString(HttpResponse* res) {
    RETURN_NULL_IF_NULL(res);

    /*
        Calculate the lenght of the string.
    */
    size_t len = 3;  // NULL character + 2 whitespaces

    len += U32_MAX_CIPHERS;  // Port length.

    const char* status_str = HttpStatus_ToString(res->status);
    len += strlen(status_str);

    const char* version_str = HttpVersion_ToString(res->version);
    len += strlen(version_str);

    /*
            Alloc the string.
    */
    char* str = (char*)malloc(len);

    if (str) {
        memset(str, 0, len);

        /*
            Dump values.
        */
        sprintf(str, "%s %d %s", version_str, res->status, status_str);
    }

    return str;
}

char* HttpResponse_ToString(HttpResponse* res) {
    /*
        Calculate the length of the string.
    */
    size_t len = 1;  // NULL terminator

    /*
        Convert the start line to string.
    */
    char* head_str = HttpResponse_StartLineToString(res);

    if (head_str) {
        /*
            Append the "\r\n" terminator.
        */
        len += strlen(head_str) + 2;
    }

    /*
        Convert the headers to string.
    */
    char* headers_str = HttpHeaders_ToString(res->headers);

    if (headers_str) {
        /*
            Append the "\r\n" payload separator.
        */
        len += strlen(headers_str) + 2;
    }

    /*
        Allocate the string.
    */
    char* str = (char*)malloc(len);

    if (str) {
        memset(str, 0, len);

        /*
            Dump values.
        */
        sprintf(str, "%s\r\n%s\r\n", head_str, headers_str);
    }

    /*
        Release the strings.
    */
    RELEASE_IF_NOT_NULL(head_str);
    RELEASE_IF_NOT_NULL(headers_str);

    return str;
}

void HttpResponse_ParseStartLine(HttpResponse* res, const char* src, size_t src_size) {
    char arg1[20] = {0};
    int arg2 = 0;

    int result = sscanf(src, "%s %d", arg1, &arg2);

    if (result == 2) {
        res->version = HttpVersion_FromString(arg1, 20);
        res->status = arg2;
    }
}

HttpResponse* HttpResponse_ParseNew(const char* src, size_t src_size) {
    RETURN_NULL_IF_NULL(src);
    RETURN_NULL_IF_ZERO(src_size);

    /*
        Allocate a new response struct.
    */
    HttpResponse* res = HttpResponse_New();

    /*
        Read line by line.
    */
    char* start = src;
    char* end = strstr(src, HTTP_DELIMITER_MESSAGE_LINE);

    /*
        Parse response line.
    */
    if (end) {
        size_t buffer_len = (size_t)(end - start);
        char* buffer = strnclone(src, buffer_len);

        HttpResponse_ParseStartLine(res, buffer, buffer_len);

        free(buffer);
    } else {
        HttpResponse_ParseStartLine(res, src, src_size);
    }

    /*
        Check if the string has the payload delimiter.
    */
    char* payload_delimiter = strstr(src, HTTP_DELIMITER_MESSAGE_PAYLOAD);

    /*
        Parse response headers.
    */
    while (end && end != payload_delimiter) {
        start = end + 2;
        end = strstr(start, HTTP_DELIMITER_MESSAGE_LINE);

        size_t buffer_len = end ? (size_t)(end - start) : src_size - (size_t)(start - src);

        if (buffer_len != 0) {
            char* buffer = strnclone(start, buffer_len);

            StringMap* header = HttpHeader_ParseNew(buffer, buffer_len);

            free(buffer);

            if (header != NULL) {
                StringMap_AddExisting(res->headers, header);
            }
        }
    }

    return res;
}