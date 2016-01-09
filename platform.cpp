#ifdef __linux__
// ==================================================================
// Linux
// ==================================================================

#include <QString>
#include <iostream>

FILE* wfopen(const wchar_t* filename, const char* mode) {
  // There's no direct conversion from wchar_t* to UTF8 because the
  // C++ standard doesn't actually require unicode to be the encoding
  // for wide characters.  QString however does make this assumption,
  // and will convert for us without any additional dependencies.
  QString qfilename = QString::fromWCharArray(filename);
  size_t size = qfilename.length() + 1;
  char *utf8name = new char[ size ];
  strncpy(utf8name, qfilename.toUtf8().constData(), size);
  FILE *fh = fopen(utf8name, mode);
  delete []utf8name;
  return fh;    
}
#else
// ==================================================================
// Windows
// ==================================================================

#include <stdlib.h>

FILE* wfopen(const wchar_t* filename, const char* mode) {
  wchar_t char_mode[50];
  if (wcstombs(char_mode, mode, 50) == -1) {
    throw std::runtime_error("Bad wfopen mode");
  }
  return _wfopen(filename, char_mode);
}
  
#endif
