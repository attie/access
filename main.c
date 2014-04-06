#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/* simple utility to test read access to a file
     if access is granted, returns 0
     if access is not granted, or on error returns 1 */

void usage(const char *bin) {
	printf("usage:\n");
	printf("  %s <filename> [user]\n", bin);
	printf("  %s <filename> [uid]\n", bin);
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	if (argc >= 3) {
		int i;
		struct passwd *pw;
		for (i = 0; argv[2][i] != '\0'; i++) {
			if (!isdigit(argv[2][i])) break;
		}
		if (argv[2][i] == '\0') {
			pw = getpwuid(atoi(argv[2]));
		} else {
			pw = getpwnam(argv[2]);
		}
		if (pw == NULL) {
			perror("getpwnam()");
			return 1;
		}
		if (setgid(pw->pw_gid)) {
			perror("setgid()");
			return 1;
		}
		if (setuid(pw->pw_uid)) {
			perror("setuid()");
			return 1;
		}
	}
	
	if (access(argv[1], R_OK)) {
		perror("access()");
		return 1;
	}

	return 0;
}
