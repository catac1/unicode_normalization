#include <climits>
#include <cstring>
#include <cstdio>
#include <cstddef>
#include <iconv.h>
#include <errno.h>
#include "iconv_mac.h"

/*
   see: https://opensource.apple.com/source/Libc/Libc-1439.141.1/include/fts.h.auto.html
    typedef struct _ftsent {
        ...
        unsigned short fts_pathlen;  // strlen(fts_path)
        unsigned short fts_namelen;  // strlen(fts_name)
        ...
    }
*/
#define FTS_PATH_MAX USHRT_MAX

iconv_t iconv_alloc_descriptor;
static char outbuf[FTS_PATH_MAX + 1];


int iconv_open_utf8mac(void) {
    iconv_alloc_descriptor = iconv_open("utf8", "utf8-mac");
    if ( iconv_alloc_descriptor == (iconv_t) -1 ) {
        fprintf(stderr, "iconv initialization error\n");
        return 1;
    }
    return 0;
}

size_t iconv_utf8mac(std::string * dst_path, const char *src, size_t len)
{
    char * outbufp = (char *)outbuf;
    size_t outremain = FTS_PATH_MAX;

    if ( iconv(iconv_alloc_descriptor, (char **)&src, (size_t *)&len,
               &outbufp, &outremain) == (size_t) -1 ) {
        printf("conversion error: %d\n", errno);
        switch (errno) {
        case E2BIG: printf("There is not sufficient room at *outbuf\n");
            break;
        case EILSEQ : printf("An invalid multibyte sequence has been encountered in the input\n");
            break;
        case EINVAL : printf("An incomplete multibyte sequence has been encountered in the input\n");
            break;
        }
        return 0;
    }

    size_t outlen = FTS_PATH_MAX - outremain;
    outbuf[outlen] = '\0';
    *dst_path = outbuf;

    return outlen;
}
