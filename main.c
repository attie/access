#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

/* simple utility to test read access to a file
     if access is granted, returns 0
     if access is not granted, or on error returns 1 */

void usage(const char *bin) {
	printf("usage:\n");
	printf("  %s <filename> [user]\n", bin);
	printf("  %s <filename> [uid]\n", bin);
}

int test_access(const struct passwd *pw, const struct group *gr, const char *path, mode_t mode) {
	struct stat st;
	struct group gm, *gm_r; char gm_buf[4096];

	mode &= 07;

	if (stat(path, &st) != 0) {
		perror("stat()");
		return 1;
	}

	/* check for direct user access */
	if (pw->pw_uid == st.st_uid) {
		if ((st.st_mode & S_IRWXU) & (mode << 6)) return 0;
	}

	/* check for direct group access */
	if (pw->pw_gid == st.st_gid) {
		if ((st.st_mode & S_IRWXG) & (mode << 3)) return 0;
	}

	/* check for member group access */
	if (getgrgid_r(st.st_gid, &gm, gm_buf, sizeof(gm_buf), &gm_r) != 0 || gm_r == NULL) {
		/* failed to get the group info... don't treat this as a hard error */
	} else {
		int i;
		for (i = 0; gm.gr_mem[i] != NULL; i++) {
			if (!strcmp(gr->gr_name, gm.gr_mem[i])) {
				if ((st.st_mode & S_IRWXG) & (mode << 3)) return 0;
			}
		}
	}

	/* check for other access */
	if ((st.st_mode & S_IRWXO) & (mode)) return 0;

	return 1;
}

int main(int argc, char *argv[]) {
	char path[PATH_MAX];
	int i;
	struct passwd pw, *pw_r; char pw_buf[4096];
	struct group gr, *gr_r;  char gr_buf[4096];
	char *p;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	if (realpath(argv[1], path) == NULL) {
		perror("realpath()");
		return 1;
	}

	if (argc >= 3) {
		for (i = 0; argv[2][i] != '\0'; i++) {
			if (!isdigit(argv[2][i])) break;
		}
		if (argv[2][i] == '\0') {
			i = getpwuid_r(atoi(argv[2]), &pw, pw_buf, sizeof(pw_buf), &pw_r);
		} else {
			i = getpwnam_r(argv[2], &pw, pw_buf, sizeof(pw_buf), &pw_r);
		}
	} else {
		i = getpwuid_r(getuid(), &pw, pw_buf, sizeof(pw_buf), &pw_r);
	}
	if (i != 0) {
		perror("getpw*_r()");
		return 1;
	}
	if (pw_r == NULL) {
		fprintf(stderr, "getpw*_r(): no such user\n");
		return 1;
	}

	if (getgrgid_r(pw.pw_gid, &gr, gr_buf, sizeof(gr_buf), &gr_r) != 0) {
		perror("getgrgid()");
		return 1;
	}
	if (gr_r == NULL) {
		fprintf(stderr, "getgr*_r(): no such group\n");
		return 1;
	}

	if (test_access(&pw, &gr, path, 04) != 0) {
		fprintf(stderr, "%s has no access to '%s'\n", pw.pw_name, path);
		return 1;
	}

	while ((p = strrchr(path, '/')) != NULL) {
		p[1] = '\0';
		if (test_access(&pw, &gr, path, 01) != 0) {
			fprintf(stderr, "%s has no access to '%s'\n", pw.pw_name, path);
			return 1;
		}
		p[0] = '\0';
	}

	return 0;
}
