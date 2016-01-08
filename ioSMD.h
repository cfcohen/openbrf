/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef IOSMD_H
#define IOSMD_H

/*static*/ class ioSMD
{
public:

  static int Export(const wchar_t*filename, const BrfMesh &m , const BrfSkeleton &s, int fi);
  static int Import(const wchar_t*filename, BrfMesh &m , BrfSkeleton &s);
  static int Export(const wchar_t*filename, const BrfAnimation &a, const BrfSkeleton &s);
  static int Import(const wchar_t*filename, BrfAnimation &a, BrfSkeleton &s);
  static const char* LastErrorString();
  static char* LastWarningString();
  static bool Warning();

private:


};

#endif // IOSMD_H
