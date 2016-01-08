/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef IOMD3_H
#define IOMD3_H

class IoMD
{
public:
  static bool Import(const wchar_t*filename, std::vector<BrfMesh> &mv);
  static bool Export(const wchar_t*filename, const BrfMesh &m);
	static bool ImportMD2(const wchar_t*filename, std::vector<BrfMesh> &mv);
  static bool ExportMD2(const wchar_t*filename, const BrfMesh &m);
  static wchar_t* LastErrorString();
private:
  static bool Import(FILE *f, BrfMesh &m);
  static bool Import(FILE *f, BrfFrame &fr);
	static bool ImportMD2(FILE *f, BrfMesh &m);
  static bool ImportMD2(FILE *f, BrfFrame &fr);
};

#endif // IOMD3_H
