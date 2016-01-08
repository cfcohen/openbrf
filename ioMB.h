/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef IOMB_H
#define IOMB_H

class IoMB
{
public:
  static bool Export(const wchar_t*filename, const BrfMesh &m , const BrfSkeleton &s, int fi);
  static bool Import(const wchar_t*filename, std::vector<BrfMesh> &m , BrfSkeleton &s, int want);
  static bool Export(const wchar_t*filename, const BrfAnimation &a, const BrfSkeleton &s);
  static bool Import(const wchar_t*filename, BrfAnimation &a, BrfSkeleton &s);
  static char* LastErrorString();
};

#endif // IOMB_H
