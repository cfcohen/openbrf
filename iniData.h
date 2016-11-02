/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef INIDATA_H
#define INIDATA_H

#include <map>
#include <vector>
#include <qpair.h>
#include "brfData.h"


//typedef QPair<int,int> Pair;


class QDir;

class IniData
{

public:
  BrfData &currentBrf;
  typedef enum {MODULE_RES, COMMON_RES, CORE_RES} Origin;
  IniData(BrfData &currentBrf);
  std::vector<BrfData> file;
  std::vector<QString> filename; // full path included
  std::vector<int> iniLine; // at which line of moudle.ini
  std::vector<Origin> origin;

  bool isWarband;

  // bitmasks of txt files used
  class UsedInType{
  public:
    uint direct;
    uint indirect;
    uint directOrIndirect()const{return direct|indirect;}
  };
  typedef std::vector< ObjCoord > ObjCoordV; // vectors of objects used

  // which brfObject is used BY which other brfObject
  ObjCoordV& usedBy(ObjCoord);
  ObjCoordV  usedBy(ObjCoord) const;

  // which brfObject is used IN which txt file
  UsedInType& usedIn(ObjCoord);
  UsedInType  usedIn(ObjCoord) const;

  const char* getName(ObjCoord);
  unsigned int getSize(ObjCoord);

  void updateBeacuseBrfDataSaved();


  QString mat2tex(const QString &s, bool *hasBump, bool *hasSpec, bool * hasTransp);
  QString stats();

  bool loadAll(int howFast);
  // 1 = only filenames.
  // 2 = fast only (only Mat & Tex, skip the rest).
  // 3 = full BRF
  // 4 = full BRF + all txts
  int updated; // 0 = no.

  QString modPath;
  QString mabPath;
  QString name() const;

  int nRefObjects() const;
  int nObjects() const;

  bool addBrfFile(const char* name, Origin origin, int line, int howFast);
  bool setPath(QString mabPath, QString modPath); // return true if changed them

  QStringList namelist[N_TOKEN];
  void updateNeededLists(); // only these needed for autocompletion
  void updateAllLists(); // all
  bool saveLists(const QString &fn);

  int findFile(const QString &fn,bool onlyModFolder=false); // returns index of a given file

  // returns: index of file, of object inside file
  ObjCoord indexOf(const QString &name, int kind);
  // as above, bust strong matching (no ".")
  ObjCoord indexOfStrict(const QString &name, int kind);

  BrfShader* findShader(const QString &fn);
  BrfTexture* findTexture(const QString &fn);
  BrfMaterial* findMaterial(const QString &name, ObjCoord startFrom=ObjCoord() );

  QStringList errorList; // list all error strings when scanning module
  bool findErrors(int maxErr); // true if there's more
  QString searchAllNames(const QString &s,bool commonResToo, int token) const;

  QString errorStringOnScan; // string on scan

  static QString tokenFullName(int k);
  static QString tokenPlurName(int k);

  QString nameShort(ObjCoord o) const;
  QString nameFull(ObjCoord o) const;

  int totSize(uint t, bool commonRes) const;
  int totUsed(uint t,bool commonRes) const;
  int totSize(int fileIndex) const;
  int totUsed(int fileIndex) const;
  int totFiles(bool commonRes) const;

private:
  QString link(int i, int j, int kind) const; // given an object j of kind kind in file i, returns a strig link
  QString linkShort(int i, int j, int kind) const;
  QString shortFileName(int i) const;
  template<class T> bool checkDuplicated(std::vector<T> &v, int fi, int maxErr);
  void checkUses(int i, int j, int kind, char* usedName, int usedKind);
  void addUsedBy(int i, int j, int kind, char* usedName, int usedKind);
  void checkFile(int i, int j, int kind, char* fileName, QDir* d0, QDir* d1, bool forFrame );
  std::map<QString, ObjCoord> indexing[N_TOKEN+1];
  void prepareIndex(int kind);

  enum {MESH_NO_EXT = N_TOKEN };
  QStringList errorListOnLoad; // list all error strings on load
  template<class T>  void searchAllNamesV(const QString &s, int t, const std::vector<T> &v, int i, QString &res) const;


  class ModuleTxtNameList{
  public:
    ModuleTxtNameList(int _tok, int _);
    int brfToken; // mesh, or material...
    int txtIndex; // from which file
    QStringList name;
    QString test();
    void append(const QString& s);
    void appendLRx(const QString& s);
    void appendNon0(const QString& s);
    void appendNonNone(const QString& s);
  };
  std::vector<ModuleTxtNameList> txtNameList;

  void setAllUsedInNone();
  void setAllUsedByNone();
  bool readModuleTxts(const QString& pathMod, const QString& pathData );
  void updateUsedBy();
  void updateUsedIn();
  void propagateUsedIn();

  void propagateUsedIn(int from);

  std::vector< std::vector< ObjCoordV > > usedByV[N_TOKEN];
  std::vector< std::vector< UsedInType > > usedInV[N_TOKEN];

  static QString tr(char*);

  void debugShowUsedFlags();

};

#endif // INIDATA_H
