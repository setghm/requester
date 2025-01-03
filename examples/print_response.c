#include <requester.h>
#include <stdio.h>

int main(void) {
	/*
		Initialize library.
	*/
	HttpClient_Init();

	/*
		Read a URL from stdin.
	*/
	char url[2048] = { 0 };

	do {
		fputs("Write the URL: ", stdout);
		fgets(url, 2048, stdin);

		/*
			Remove the new line character.
		*/
		url[strcspn(url, "\n")] = '\0';

	} while (strlen(url) == 0);

	/*
		Create a new http request and send it.
	*/
	HttpResponse* res = HttpClient_Get("https://www.google.com/search?q=http+client");

	if (res == HTTP_INVALID_RESPONSE) {
		puts("Cannot get the HttpResponse");
		return -1;
	}

	printf("Http response: %d\n", res->status);

	/*
		Print response headers.
	*/
	size_t i;

	printf("Http response headers: {\n");

	for (i = 0; i < res->headers->length; i++) {
		const StringPair* header = StringMap_GetAt(res->headers, i);

		printf("\t%s: \"%s\"\n", header->key, header->value);
	}

	puts("}");

	/*
		Get the response content.
	*/
	puts("------- Http response content -------");

	const char buffer[256] = { 0 };
	size_t bytes_read = 0;

	do {

		bytes_read = StreamContent_Read(res->content, (const byte*)buffer, 256);

		fputs(buffer, stdout);

	} while (bytes_read > 0);

	puts("\n------- Http response content end -------\n");

	/*
		Cleanup resources.
	*/
	HttpResponse_Delete(res);
	HttpClient_Cleanup();

	return 0;
}
