/* See LICENSE file for copyright and license details. */

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

void die(const char *fmt, ...);
void *xmalloc(size_t len);
void *ecalloc(size_t nmembm, size_t size);
void safe_create_dir(const char *dir);
void rec_mkdir(char *path);
FILE *fopen_mkdir(char *path, char *mode);
//int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
int rmrf(char *path);
