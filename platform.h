FILE* wfopen(const wchar_t* filename, const char* mode);

#ifdef __linux__
// Make the APIENTRY keyword go away.
#ifndef APIENTRY
#define APIENTRY
#endif

#include <strings.h>
#define stricmp(A, B) strcasecmp(A, B)

#endif
