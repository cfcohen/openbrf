/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QtCore>
#include <math.h>
#include <QTextBrowser>

#include "brfData.h"
#include "iniData.h"

const char* txtFileName[TXTFILE_NONE+1] = {
  "actions.txt",          // TXTFILE_ACTIONS,
  "skins.txt",            // TXTFILE_SKIN,
  "item_kinds1.txt",      // TXTFILE_ITEM,
  "meshes.txt",           // TXTFILE_MESHES,
  "map_icons.txt",        // TXTFILE_ICONS,
  "scene_props.txt",      // TXTFILE_PROP,
  "particle_systems.txt", // TXTFILE_PARTICLE,
  "tableau_materials.txt",// TXTFILE_TABLEAU
  "flora_kinds.txt",      // TXTFILE_FLORA_KINDS,
  "ground_specs.txt",     // TXTFILE_GROUND_SPECS,
  "skyboxes.txt",         // TXTFILE_SKYBOXES,
  "scenes.txt",           // TXTFILE_SCENES,
  "[ERROR]",              // N_TXTFILES,
  "[core system]",        // TXTFILE_CORE,
  "[ERROR]",        //TXTFILE_NONE,

};

template <class T> int _findByName( const vector<T> &v, const QString &s){
  for (unsigned int i=0; i<v.size(); i++)
    if (QString::compare(v[i].name,s,Qt::CaseInsensitive)==0) return i;
  return -1;
}

template <class T> int _findByNameNoExt( const vector<T> &v, const QString &s){
  for (unsigned int i=0; i<v.size(); i++) {
    QString st = v[i].name;
    int k = st.lastIndexOf('.');
    if (k>0) st.truncate(k);
    QString s2 = s;
    k = s2.lastIndexOf('.');
    if (k>0) s2.truncate(k);

    if (QString::compare(st,s2,Qt::CaseInsensitive)==0) return i;
  }
  return -1;
}

template <class T> void _addNames( const vector<T> &v, std::map<QString, ObjCoord> &m, int j){
  for (unsigned int i=0; i<v.size(); i++)
    m[v[i].name] = ObjCoord(j,i,T::tokenIndex());
}

template <class T> void _addNamesNoExt( const vector<T> &v, std::map<QString, ObjCoord> &m, int j){
  for (unsigned int i=0; i<v.size(); i++) {
    QString st = v[i].name;
    int k = st.lastIndexOf('.');
    if (k>0) st.truncate(k);
    m[st] = ObjCoord(j,i,T::tokenIndex());
  }
}

void IniData::setAllUsedByNone(){
  for (int t=MESH; t<N_TOKEN; t++){
    usedByV[t].resize(file.size());
    for (int fi=0; fi<(int)file.size( ); fi++){
      usedByV[t].at(fi).resize(file[fi].size(t));
      for (int fo=0; fo<(int)file[fi].size(t); fo++){
        usedBy( ObjCoord(fi,fo,t) ).clear();
      }
    }
  }
}

void IniData::setAllUsedInNone(){
  for (int t=MESH; t<N_TOKEN; t++){
    usedInV[t].resize(file.size());
    for (int fi=0; fi<(int)file.size( ); fi++){
      usedInV[t].at(fi).resize(file[fi].size(t));
      for (int fo=0; fo<(int)file[fi].size(t); fo++){
        usedIn( ObjCoord(fi,fo,t) ).direct = 0;
        usedIn( ObjCoord(fi,fo,t) ).indirect = 0;
      }
    }
  }
}

void IniData::propagateUsedIn(int from){
  ObjCoord i;
  i.t=from;
  for (i.fi=0; i.fi<(int)file.size(); i.fi++){
    for (i.oi=0; i.oi<int(usedByV[i.t].at(i.fi).size()); i.oi++) {
      ObjCoordV &v(usedBy( i ));
      for (unsigned int k = 0; k<v.size(); k++){
        usedIn( i ).indirect |= usedIn( v[k] ).direct;
        usedIn( i ).indirect |= usedIn( v[k] ).indirect;
      }
    }
  }
}

void IniData::propagateUsedIn(){
  propagateUsedIn(MATERIAL); // material from mesh
  propagateUsedIn(SHADER); // shaders from materials
  propagateUsedIn(TEXTURE); // textures from materials
}

// helper class to scan txt files
class TextFile{
public:
  QFile qf;
  int line;
  char data[4024];
  char format[1000];
  char res[1000];
  char to[255][255];
  QString path;
  QString trymeFirstPath;
  QString errorString;

  TextFile(){
    trymeFirstPath = "";
  }

  void open(QString filename) throw (int){
    line = 0;
    if (!trymeFirstPath.isEmpty()){
      qf.setFileName(QString("%1/%2").arg(trymeFirstPath).arg(filename));
      if (qf.open(QIODevice::ReadOnly)) return;
    }
    qf.setFileName(QString("%1/%2").arg(path).arg(filename));
    if (!qf.open(QIODevice::ReadOnly)) {
      error(QTextBrowser::tr("cannot open file"));
    }
  }
  void expectLine(const char* st) throw (int){
    qf.readLine(data,4023);
    line++;
    QString dataS = QString(data).remove(QChar('\n'), Qt::CaseSensitive);
    if (!dataS.startsWith(QString(st))) {
      error(QTextBrowser::tr("expected '%1',\ngot '%2'").arg(st).arg(dataS));
    }
  }
  void nextLine() throw (int){
    if (qf.readLine(data,4023)==-1) error(QTextBrowser::tr("unexpected end of file"));
    line++;
  }
  void skipLines(int n) throw (int){
    for (int i=0; i<n; i++) nextLine();
  }
  // reads the n^th string token from last read line
  char* stringT(int n) throw (int){
    if (n>255)
      error(QString("Internal error: StringT parameter %1").arg(n));
    makeFormat(n);
    int k=sscanf(
      data,format,
      to[0x00],to[0x01],to[0x02],to[0x03],to[0x04],to[0x05],to[0x06],to[0x07],to[0x08],to[0x09],to[0x0A],to[0x0B],to[0x0C],to[0x0D],to[0x0E],to[0x0F],
      to[0x10],to[0x11],to[0x12],to[0x13],to[0x14],to[0x15],to[0x16],to[0x17],to[0x18],to[0x19],to[0x1A],to[0x1B],to[0x1C],to[0x1D],to[0x1E],to[0x1F],
      to[0x20],to[0x21],to[0x22],to[0x23],to[0x24],to[0x25],to[0x26],to[0x27],to[0x28],to[0x29],to[0x2A],to[0x2B],to[0x2C],to[0x2D],to[0x2E],to[0x2F],
      to[0x30],to[0x31],to[0x32],to[0x33],to[0x34],to[0x35],to[0x36],to[0x37],to[0x38],to[0x39],to[0x3A],to[0x3B],to[0x3C],to[0x3D],to[0x3E],to[0x3F],
      to[0x40],to[0x41],to[0x42],to[0x43],to[0x44],to[0x45],to[0x46],to[0x47],to[0x48],to[0x49],to[0x4A],to[0x4B],to[0x4C],to[0x4D],to[0x4E],to[0x4F],
      to[0x50],to[0x51],to[0x52],to[0x53],to[0x54],to[0x55],to[0x56],to[0x57],to[0x58],to[0x59],to[0x5A],to[0x5B],to[0x5C],to[0x5D],to[0x5E],to[0x5F],
      to[0x60],to[0x61],to[0x62],to[0x63],to[0x64],to[0x65],to[0x66],to[0x67],to[0x68],to[0x69],to[0x6A],to[0x6B],to[0x6C],to[0x6D],to[0x6E],to[0x6F],
      to[0x70],to[0x71],to[0x72],to[0x73],to[0x74],to[0x75],to[0x76],to[0x77],to[0x78],to[0x79],to[0x7A],to[0x7B],to[0x7C],to[0x7D],to[0x7E],to[0x7F],
      to[0x80],to[0x81],to[0x82],to[0x83],to[0x84],to[0x85],to[0x86],to[0x87],to[0x88],to[0x89],to[0x8A],to[0x8B],to[0x8C],to[0x8D],to[0x8E],to[0x8F],
      to[0x90],to[0x91],to[0x92],to[0x93],to[0x94],to[0x95],to[0x96],to[0x97],to[0x98],to[0x99],to[0x9A],to[0x9B],to[0x9C],to[0x9D],to[0x9E],to[0x9F],
      to[0xA0],to[0xA1],to[0xA2],to[0xA3],to[0xA4],to[0xA5],to[0xA6],to[0xA7],to[0xA8],to[0xA9],to[0xAA],to[0xAB],to[0xAC],to[0xAD],to[0xAE],to[0xAF],
      to[0xB0],to[0xB1],to[0xB2],to[0xB3],to[0xB4],to[0xB5],to[0xB6],to[0xB7],to[0xB8],to[0xB9],to[0xBA],to[0xBB],to[0xBC],to[0xBD],to[0xBE],to[0xBF],
      to[0xC0],to[0xC1],to[0xC2],to[0xC3],to[0xC4],to[0xC5],to[0xC6],to[0xC7],to[0xC8],to[0xC9],to[0xCA],to[0xCB],to[0xCC],to[0xCD],to[0xCE],to[0xCF],
      to[0xD0],to[0xD1],to[0xD2],to[0xD3],to[0xD4],to[0xD5],to[0xD6],to[0xD7],to[0xD8],to[0xD9],to[0xDA],to[0xDB],to[0xDC],to[0xDD],to[0xDE],to[0xDF],
      to[0xE0],to[0xE1],to[0xE2],to[0xE3],to[0xE4],to[0xE5],to[0xE6],to[0xE7],to[0xE8],to[0xE9],to[0xEA],to[0xEB],to[0xEC],to[0xED],to[0xEE],to[0xEF],
      to[0xF0],to[0xF1],to[0xF2],to[0xF3],to[0xF4],to[0xF5],to[0xF6],to[0xF7],to[0xF8],to[0xF9],to[0xFA],to[0xFB],to[0xFC],to[0xFD],to[0xFE],to[0xFF]
    );
    if (k!=n) error(QTextBrowser::tr("cannot read token n. %1 from:\n '%2'").arg(n).arg(data));
    return to[n-1];
  }
  // reads the n^th int token from last read line
  int intT(int n, int min=0, int max=10000) throw (int){
    int num;
    int k=sscanf(stringT(n),"%d",&num);
    if (k!=1) error(QTextBrowser::tr("expected number istead of '%1' (token %2)").arg(stringT(n)).arg(n));
    if (num<min || num>max ) error(QTextBrowser::tr("wrong number : %1 (not in [%2, %3]) (token %4)").arg(num).arg(min).arg(max).arg(n));
    return num;
  }
  // reads the n^th int token from last read line
  long long longT(int n) throw (int){
    long long num;
		int k=sscanf(stringT(n),"%lld",&num);
    if (k!=1) error(QTextBrowser::tr("expected number istead of '%1' (token %2)").arg(stringT(n)).arg(n));
    return num;
  }
  void close(){
    qf.close();
  }

private:
  void error(QString s) throw (int){
    errorString =  QString(
        QTextBrowser::tr("Error reading file '%1',\nat line %3:\n%2\n").arg(qf.fileName()).arg(s).arg(line)
    );
    throw 1;
  }
  void makeFormat(int n){
    int h=0;
    for (int i=0; i<n; i++){format[h++]='%';format[h++]='s';format[h++]=' '; }
    h--;
    format[h]=0;
  }

  //QString tr(char* s){return qApp->QTextBrowser::tr(s);}



};

//QString IniData::QTextBrowser::tr(char* s) {return qApp->QTextBrowser::tr(s);}

IniData::ModuleTxtNameList::ModuleTxtNameList(int _tok, int _txtF):brfToken(_tok),txtIndex(_txtF){
  name.clear();
}
void IniData::ModuleTxtNameList::append(const QString &s){
  name.append(s);
}
void IniData::ModuleTxtNameList::appendLRx(const QString &s){
  name.append(s);
  if (s.endsWith("_L")) {
    QString s2(s);
    s2.chop(2);
    name.append(s2+"_R");
  }
}

void IniData::ModuleTxtNameList::appendNon0(const QString &s){
  if (s!="0") name.append(s);
}
void IniData::ModuleTxtNameList::appendNonNone(const QString &s){
  if (s!="none") name.append(s);
}

QString IniData::ModuleTxtNameList::test(){
  return QString(QTextBrowser::tr("%1 %2 from '%3' <font size=-1>('%4', '%5', '%6'...)</font>\n\n"))
      .arg(name.size())
      .arg(IniData::tokenFullName(brfToken))
      .arg(txtFileName[txtIndex])
      .arg((name.size()>1)?name[0]:"")
      .arg((name.size()>2)?name[1]:"")
      .arg((name.size()>3)?name[2]:"");
}

QString R4L(QString s){
  QString t(s);
  if (t.endsWith('L',Qt::CaseSensitive)) t[t.length()-1]='R';
  if (t.endsWith('l',Qt::CaseSensitive)) t[t.length()-1]='r';
  return t;
}

bool IniData::readModuleTxts(const QString &pathMod, const QString& pathData){
  TextFile tf;
  tf.path = pathMod;

  txtNameList.clear();

  // add default things used by the system
  {
    ModuleTxtNameList list(MESH, TXTFILE_CORE);
    list.append("sample_flare");
    list.append("sun");
    list.append("flying_missile");
    list.append("battle_icon");
    list.append("battle_track");
    list.append("track");
    list.append("compass");
    txtNameList.push_back(list);
  }
  {
    ModuleTxtNameList list(TEXTURE, TXTFILE_CORE);
    list.append("cursor.dds");
    txtNameList.push_back(list);
  }
  {
    ModuleTxtNameList list(SHADER, TXTFILE_CORE);
    list.append("def_shader");
    txtNameList.push_back(list);
  }

  try {
  {
    // READING ITEMS.TXT
    int txtFile = TXTFILE_ITEM;

    tf.open(txtFileName[txtFile]);
    ModuleTxtNameList list(MESH, txtFile);

    tf.nextLine();//tf.expectLine("itemsfile version 2");
    int ver = tf.intT(3,2,3);
    if (ver==3) isWarband = true;
    tf.nextLine();
    int n = tf.intT(1,0,10000);

    for  (int i=0; i<n; i++) {
      tf.nextLine();
      int tmp = tf.intT(4,0,10);
      for (int j=0; j<tmp; j++)
        list.appendLRx( QString(tf.stringT(5+j*2)) );
      if (ver == 3) {
        tf.nextLine();
        if (tf.intT(1)!=0) tf.nextLine();
      }
      tf.nextLine();
      tf.skipLines( tf.intT(1,0,10) );
      tf.nextLine();
    }
    txtNameList.push_back(list);
    //errorStringOnScan = QString("Loaded %1 objs!!\n('%2', '%3'...,)")
    //                    .arg(list.name.size()).arg(list.name[0]).arg(list.name[0]);
    //return false;
    tf.close();
  }

  {
    // READING ACTIONS.TXT
    int txtFile = TXTFILE_ACTIONS;
    tf.open(txtFileName[txtFile]);
    ModuleTxtNameList list(ANIMATION, txtFile);

    tf.nextLine();
    int n = tf.intT(1);
    for  (int i=0; i<n; i++) {
      tf.nextLine();
      int tmp = tf.intT((isWarband)?4:3,0,30);
      for (int j=0; j<tmp; j++) {
        tf.nextLine();
        list.append( QString(tf.stringT(2)) );
      }
    }
    //errorStringOnScan = list.test(); return false;
    txtNameList.push_back(list);
    tf.close();
  }

    {
      // READING SKINS.TXT
      int txtFile = TXTFILE_SKIN;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList listMe(MESH, txtFile);
      ModuleTxtNameList listMa(MATERIAL, txtFile);
      ModuleTxtNameList listSk(SKELETON, txtFile);

      tf.expectLine("skins_file version 1");
      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        tf.nextLine();
        listMe.append( tf.stringT(1) ); // body
        listMe.append( tf.stringT(2) ); // calfl
        listMe.append( R4L( tf.stringT(2) ) ); // calfr
        listMe.append( tf.stringT(3) ); // handl
        listMe.append( R4L( tf.stringT(3) ) ); // handr
        tf.nextLine();
        listMe.append( tf.stringT(1) ); // head
        tf.nextLine();
        int tmp = tf.intT(1,0,40);
        tf.nextLine();
        for (int j=0; j<tmp; j++) listMe.append( tf.stringT(j+1) ); // hair mesh

        tf.nextLine();
        tmp = tf.intT(1,0,40);
        for (int j=0; j<tmp; j++) {
          tf.nextLine();
          listMe.append( tf.stringT(1) ); // beard mesh
        }

        tf.nextLine(); // empty line

        // materials for hair, beard: seems game uses only 1st one?
        tf.nextLine();
        tmp = tf.intT(1,0,30);
        for (int j=0; j<tmp; j++) if (j==0) listMa.append( tf.stringT(j+2) ); // hair mat

        tf.nextLine();
        tmp = tf.intT(1,0,30);
        for (int j=0; j<tmp; j++) if (j==0) listMa.append( tf.stringT(j+2) ); // beard mat

        tf.nextLine();
        tmp = tf.intT(1,0,40);
        int h=2;
        for (int j=0; j<tmp; j++) {
          listMa.append( tf.stringT(h++) ); // skin mat
          h++;
          int a = tf.intT(h++);
          int b = tf.intT(h++);
          for (int k=0; k<a; k++)
            listMa.append( tf.stringT(h++) ); // beard mat
          h+=b;
        }
        tf.nextLine(); // sounds
        tf.nextLine(); // skel
        listSk.append( tf.stringT(1));
        tf.nextLine(); // two numbers?
        tf.nextLine();
        tmp = tf.intT(1,0,40);
        tf.skipLines(tmp+1);

      }

      //listSk.append("skel_horse"); // bonus!
      
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(listMa);
      txtNameList.push_back(listSk);
      txtNameList.push_back(listMe);
      //errorStringOnScan = listMa.test()+listSk.test()+listMe.test(); return false;
      tf.close();
    }

    {
      // READING MAP_ICONS.TXT
      int txtFile = TXTFILE_ICONS;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MESH, txtFile);

      tf.expectLine("map_icons_file version 1");

      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        list.append( QString(tf.stringT(3)) );
        tf.skipLines(tf.intT(9)+2);
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }

    {
      // READING MESHES.TXT
      int txtFile = TXTFILE_MESHES;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MESH, txtFile);

      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        list.appendNon0( QString(tf.stringT(3)) );
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }
    {
      // READING PARTICLE_SYSTEM.TXT
      int txtFile = TXTFILE_PARTICLE;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MESH, txtFile);

      tf.expectLine("particle_systemsfile version 1");
      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        list.append( QString(tf.stringT(3)) );
        tf.skipLines(7);
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }
    {
      // READING SCENES_PROP.TXT
      int txtFile = TXTFILE_PROP;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList listMe(MESH, txtFile);
      ModuleTxtNameList listCo(BODY, txtFile);

      tf.expectLine("scene_propsfile version 1");
      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        listMe.appendNon0( tf.stringT(4) );
        listCo.appendNon0( tf.stringT(5) );
        tf.skipLines( tf.intT(6)+2);
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(listMe);
      txtNameList.push_back(listCo);
      tf.close();
    }
    {
      // READING TABLEAU
      int txtFile = TXTFILE_TABLEAU;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MATERIAL, txtFile);

      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        list.append( QString(tf.stringT(3)) );
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }
    {
      // READING SCENES
      int txtFile = TXTFILE_SCENES;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList listB(BODY, txtFile);
      ModuleTxtNameList listM(MESH, txtFile);

      tf.expectLine("scenesfile version 1");
      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        tf.nextLine();
        listM.appendNonNone( tf.stringT(4) );
        listB.appendNonNone( tf.stringT(5) );
        tf.nextLine();
        tf.nextLine();
        tf.nextLine();
        listM.appendNon0( tf.stringT(1) );
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(listM);
      txtNameList.push_back(listB);
      tf.close();
    }


    // data path
    ///////////////////////
    tf.path = pathData;
    tf.trymeFirstPath = pathMod+"/Data";
    {
      // READING FLORA_KINDS
      int txtFile = TXTFILE_FLORA_KINDS;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList listMe(MESH, txtFile);
      ModuleTxtNameList listBo(BODY, txtFile);

      tf.nextLine();
      int n = tf.intT(1);

      if (isWarband){
        tf.nextLine(); // peek next line
        for  (int i=0; i<n; i++) {
          // ugly hack:
          int invert = ((tf.data[0]>='A') && (tf.data[0]<='Z')) ;
          int m = tf.intT(3);
          QStringList qla,qlb;
          qla.clear();qlb.clear();
          int x2 = 1;
          for  (int j=0; j<m; j++) {
            tf.nextLine();
            qla.append(QString(tf.stringT(1))); qlb.append(QString(tf.stringT(2)));
          }
          if (i<n-1) {
            tf.nextLine();
            if (tf.data[0]==' ') { // must read twice as many items and keep evens
              x2 = 2;
              for  (int j=0; j<m; j++) {
                qla.append(QString(tf.stringT(1))); qlb.append(QString(tf.stringT(2)));
                tf.nextLine();
              }
            }
          }
          for (int j=0; j<m; j++){
            listMe.append(qla[j*x2+invert*(x2-1)]);
            listBo.appendNon0(qlb[j*x2+invert*(x2-1)]);
          }
        }

      } else {
        for  (int i=0; i<n; i++) {
          tf.nextLine();
          int m = tf.intT(3);
          for  (int j=0; j<m; j++) {
            tf.nextLine();
            listMe.append( tf.stringT(1) );
            listBo.appendNon0( tf.stringT(2) );
          }
        }
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(listMe);
      txtNameList.push_back(listBo);
      tf.close();
    }

    {
      // READING GROUND_SPECS
      int txtFile = TXTFILE_GROUND_SPECS;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MATERIAL, txtFile);

      while  (1) {
        try{
          tf.nextLine();
        } catch (int) {
          break;
        }
        list.appendNonNone( QString(tf.stringT(3)) );
      }

      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }

    {
      // READING SKY_BOXES
      int txtFile = TXTFILE_SKYBOXES;
      tf.open(txtFileName[txtFile]);
      ModuleTxtNameList list(MESH, txtFile);

      tf.nextLine();
      int n = tf.intT(1);
      for  (int i=0; i<n; i++) {
        if (isWarband) {
          tf.nextLine();
          list.append( QString(tf.stringT(1) ));
          tf.nextLine();
        } else{
          tf.nextLine();
          list.append( QString(tf.stringT((i==0)?1:12)) );
        }
      }
      //errorStringOnScan = list.test(); return false;
      txtNameList.push_back(list);
      tf.close();
    }


  } catch (int) {
    errorStringOnScan = QString(tf.errorString);
    return false;
  }


  return true;


}


IniData::ObjCoordV& IniData::usedBy(ObjCoord o){
  return usedByV[o.t].at(o.fi).at(o.oi);
}
IniData::ObjCoordV IniData::usedBy(ObjCoord o) const{
  return usedByV[o.t].at(o.fi).at(o.oi);
}

IniData::UsedInType &IniData::usedIn(ObjCoord o){
  return usedInV[o.t].at(o.fi).at(o.oi);
}
IniData::UsedInType IniData::usedIn(ObjCoord o) const{
  return usedInV[o.t].at(o.fi).at(o.oi);
}

QString IniData::nameShort(ObjCoord o) const{
  return QString(file[o.fi].GetName(o.oi,o.t));
}

QString IniData::nameFull(ObjCoord o) const{
  static QString st[3] = {QString(),QString(" (CommonRes)"),QString(" (CoreRes)")};

  QString res = QString("%3 %1%2")
        .arg(file[o.fi].GetName(o.oi,o.t)).arg(st[origin[o.fi]]).arg(tokenFullName(o.t));
  return res;
}

QString IniData::linkShort(int i, int j, int kind) const{
  QString res;
  res = QString("<a href=\"#%1.%2.%3\">%4</a>")
        .arg(i).arg(j).arg(kind).arg(shortFileName(i));
  return res;
}

QString IniData::shortFileName(int i) const {
  if (origin[i]==MODULE_RES) return QString(filename[i]).replace(modPath,"");
  else return QString(filename[i]).replace(mabPath,"");
}

QString IniData::link(int i, int j, int kind) const {
  QString res;
  res = QString(QTextBrowser::tr("%6 <a href=\"#%1.%2.%3\">%4</a> (in %5)"))
        .arg(i).arg(j).arg(kind)
        .arg(file[i].GetName(j,kind)).arg(shortFileName(i)).arg(tokenFullName(kind));
  return res;
}

void IniData::checkFile(int i, int j, int kind, char* usedFile, QDir *d0, QDir *d1, bool forFrame){

  bool res = false;
  if (d0) if (d0->exists(usedFile)) res = true;
  if (!res) if (d1) if (d1->exists(usedFile)) res = true;

  if (!res) {
    if (!forFrame) {
      errorList.push_back(
        QTextBrowser::tr("<b>File-not-found:</b> can't find image file for %1.")
                .arg(link(i,j,kind))
      );
    } else {
        errorList.push_back(
          QTextBrowser::tr("<b>File-not-found:</b> can't find frame \"%2\" for %1.")
                 .arg(link(i,j,kind)).arg(usedFile)
        );
    }
  }

}



template <class T>
bool IniData::checkDuplicated(std::vector<T> &v, int j, int maxErr){
  int kind = T::tokenIndex();
  for (unsigned int i=0; i<v.size(); i++) {
    if (errorList.size()>maxErr) return false;
    ObjCoord d = indexOfStrict(v[i].name, kind );
    if (d.fi==-1) {
      errorList.push_back("<b>Internal error:</b> this should never happen");
    } else
    if (d.fi!=j || d.oi!=(int)i) {
      errorList.push_back(
          QTextBrowser::QTextBrowser::tr("<b>Duplicate:</b> %1 was already defined in file %2")
          .arg(link(j,i,kind)).arg(linkShort(d.fi,d.oi,kind))
      );
    }
  }
  return true;
}

template <class T>
void IniData::searchAllNamesV(const QString &s, int t,const std::vector<T> &v, int j, QString &res) const{
  int kind = T::tokenIndex();
  if (t!=-1 && t!=kind) return;
  for (unsigned int i=0; i<v.size(); i++) {

    if (QString(v[i].name).contains(s,Qt::CaseInsensitive))
      res+=link(j,i,kind)+"<br>";

  }
}

void IniData::checkUses(int i, int j, int kind, char* usedName, int usedKind){

  if (usedKind==TEXTURE && QString(usedName)=="none") return;

  ObjCoord d = indexOf( usedName, usedKind );
  if (d.fi==-1) {
    errorList.push_back(
      QTextBrowser::tr("<b>Missing:</b> %1 uses unknown %2 <u>%3</u>")
      .arg(link(i,j,kind)).arg(tokenFullName(usedKind)).arg(usedName)
    );
  } else
  if (d.fi>i) {
    errorList.push_back(
      QTextBrowser::tr("<b>Ordering problem:</b> %1 uses %2, which appears later in <i>module.ini</i>")
      .arg(link(i,j,kind)).arg(link(d.fi,d.oi,usedKind))
    );
  }
}

static QString noDot(QString s){
  int n = s.indexOf('.');
  if (n!=-1) s.truncate(n);
  return s;
}

const char* IniData::getName(ObjCoord o){
  return file[o.fi].GetName(o.oi,o.t);
}

unsigned int IniData::getSize(ObjCoord o){
  return file[o.fi].size(o.t);

}

QString IniData::stats() {
  QString res;
  res.append(QTextBrowser::tr("<h1>Module <b>%1</b></h1>").arg(name()));
  bool once = true;
  res.append("<p><br></p><table border=0><tr>");
  for (int mod=1; mod>=0; mod--){
    res.append("<td>");
    if (mod==1)
      res.append(QTextBrowser::tr("<h2>Original BRF files: %1</h2>").arg(totFiles(mod)));
    else
      res.append(QTextBrowser::tr("<h2>CommonRes BRF files: %1</h2>").arg(totFiles(mod)));
    for (int t=0; t<N_TOKEN; t++){
      int tot = totSize(t,mod);
      if (updated>=4) {
        int u = totUsed(t,mod);
        res.append(QString("%1: %2 (%3+%4)").arg(tokenPlurName(t)).arg(tot).arg(u).arg(tot-u));
      } else
        res.append(QString("%1: %2").arg(tokenPlurName(t)).arg(tot));
      if (once)res.append(QTextBrowser::tr("<i>(used+unused)</i>")); once=false;
      res.append(QTextBrowser::tr("<br>"));
    }
    res.append("</td>");
  }
  res.append("</tr></table>");

  res.append(QTextBrowser::tr("<h2>Txt data:</h2>"));
  for (unsigned i=0; i<txtNameList.size(); i++){
    res.append(txtNameList[i].test());
    res.append("<br>");
  }
  return res;
}

int IniData::totSize(uint t,bool commonRes) const{
  int res=0;
  for (unsigned int i=0; i<file.size(); i++) {
    if ((origin[i]!=MODULE_RES) == !commonRes) res+=file[i].size(t);
  }
  return res;
}

int IniData::totFiles( bool commonRes) const{
  int res=0;
  for (unsigned int i=0; i<file.size(); i++) {
    if ((origin[i]!=MODULE_RES) == !commonRes) res++;
  }
  return res;
}


int IniData::totUsed(uint t, bool commonRes) const{
  if (updated<4) return 0;
  int res=0;
  ObjCoord o;
  o.t=t;
  for (o.fi=0; o.fi<(int)file.size(); o.fi++) {
    if ((origin[o.fi]!=MODULE_RES) == !commonRes) {
      for (o.oi=0; o.oi<int(usedInV[o.t].at(o.fi).size()); o.oi++)
        if (usedIn(o).directOrIndirect()) res++;
    }
  }
  return res;
}

int IniData::totUsed(int fi) const{
  if (updated<4) return 0;
  int res=0;
  for (int t=0; t<N_TOKEN; t++) {
    ObjCoord o(fi,0,t);
    for (o.oi=0; o.oi<int(usedInV[o.t].at(o.fi).size()); o.oi++)
      if (usedIn(o).directOrIndirect()) res++;
  }
  return res;
}

int IniData::totSize(int fi) const{
  if (updated<4) return 0;
  int res=0;
  for (int t=0; t<N_TOKEN; t++) {
    res+=file[fi].size(t);
  }
  return res;
}

void IniData::addUsedBy(int i, int j, int kind,char* usedName,  int usedKind){

  if (kind==TEXTURE && QString(usedName)=="none") return;

  ObjCoord d = indexOf( usedName, usedKind );
  ObjCoord o (i,j,kind);

  if (d.fi>=0){
    usedBy(d).push_back( o );



    if ((usedKind==MESH)||(usedKind==MATERIAL) ){
      QString nameDot=noDot( getName(d) );

      for (unsigned int k=0; k<getSize(d); k++){
        ObjCoord d2(d.fi,k,d.t);
        if (noDot(getName(d2))==nameDot)
        usedBy(d2).push_back( o );
      }
    }
  }


}

void IniData::updateUsedBy(){

  setAllUsedByNone();

  for (unsigned int i=0; i<file.size(); i++) {

    // add for mesh->material
    for (unsigned int j=0; j<file[i].mesh.size(); j++) {
      addUsedBy(i,j,MESH, file[i].mesh[j].material, MATERIAL );
    }

    // add for material->etc
    for (unsigned int j=0; j<file[i].material.size(); j++) {
      addUsedBy(i,j,MATERIAL, file[i].material[j].diffuseA, TEXTURE );
      addUsedBy(i,j,MATERIAL, file[i].material[j].diffuseB, TEXTURE );
      addUsedBy(i,j,MATERIAL, file[i].material[j].bump, TEXTURE );
      addUsedBy(i,j,MATERIAL, file[i].material[j].enviro, TEXTURE );
      addUsedBy(i,j,MATERIAL, file[i].material[j].spec, TEXTURE );
      addUsedBy(i,j,MATERIAL, file[i].material[j].shader, SHADER );
    }
    // add for shader->shader
    for (unsigned int j=0; j<file[i].shader.size(); j++) {
      addUsedBy(i,j,SHADER, file[i].shader[j].fallback, SHADER );
    }

  }
}

bool IniData::findErrors(int maxErr){
  errorList = errorListOnLoad;//.clear();
  for (unsigned int i=0; i<file.size(); i++) {
    if (errorList.size()>maxErr) break;

    // check for dupilcates
    checkDuplicated(file[i].texture,i,maxErr);
    checkDuplicated(file[i].shader,i,maxErr);
    checkDuplicated(file[i].material,i,maxErr);
    checkDuplicated(file[i].mesh,i,maxErr);
    checkDuplicated(file[i].body,i,maxErr);
    checkDuplicated(file[i].skeleton,i,maxErr);
    checkDuplicated(file[i].animation,i,maxErr);

    // check for mesh->material
    for (unsigned int j=0; j<file[i].mesh.size(); j++) {
      if (errorList.size()>maxErr) break;
      checkUses(i,j,MESH, file[i].mesh[j].material, MATERIAL );
    }

    // check for material->etc
    for (unsigned int j=0; j<file[i].material.size(); j++) {
      if (errorList.size()>maxErr) break;
      checkUses(i,j,MATERIAL, file[i].material[j].diffuseA, TEXTURE );
      checkUses(i,j,MATERIAL, file[i].material[j].diffuseB, TEXTURE );
      checkUses(i,j,MATERIAL, file[i].material[j].bump, TEXTURE );
      checkUses(i,j,MATERIAL, file[i].material[j].enviro, TEXTURE );
      checkUses(i,j,MATERIAL, file[i].material[j].spec, TEXTURE );
      checkUses(i,j,MATERIAL, file[i].material[j].shader, SHADER );
    }

    // check for texture->fileondisk
    QDir d0(this->mabPath); d0.cd("Textures");
    QDir d1(this->modPath); d1.cd("Textures");
    for (unsigned int j=0; j<file[i].texture.size(); j++) {
      if (errorList.size()>maxErr) break;
      //if (QString(file[i].texture[j].name)!="waterbump") // waterbumb hack
      BrfTexture &tex(file[i].texture[j]);
      if (tex.NFrames()==0) {
        checkFile(i,j,TEXTURE, tex.name , &d0, &d1, false);
      } else {
        for (int ti=0; ti<tex.NFrames(); ti++) {
          char fullname[255];
          sprintf(fullname,"%s_%d.dds",tex.name,ti);
          checkFile(i,j,TEXTURE, fullname , &d0, &d1, true);
        }
      }
    }
  }
  if (errorList.size()>maxErr) {
    errorList.push_back(QTextBrowser::QTextBrowser::tr("<i>more errors to follow...</i>"));
    return true;
  } else return false;

}

QString IniData::searchAllNames(const QString &s, bool cr, int to) const{
  QString res;
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].texture,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].shader,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].material,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].mesh,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].body,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].skeleton,i,res);
  for (int i=0; i<(int)file.size(); i++) if (origin[i]==MODULE_RES || cr)
    searchAllNamesV(s,to,file[i].animation,i,res);
  if (res.isEmpty()) res+=QTextBrowser::QTextBrowser::tr("<i>[0 results]</i>");

  return res;
}


IniData::IniData(BrfData &_currentBrf): currentBrf(_currentBrf)
{
  filename.clear();
  file.clear();
  origin.clear();
  iniLine.clear();

  modPath.clear();;
  mabPath.clear();
  updated = 0;
  isWarband = false;
}

QString IniData::mat2tex(const QString &n, bool* hasBump, bool* hasSpec){
  // find in ini file
  int j = _findByName( currentBrf.material, n);
  if (j>=0) {
    *hasBump = currentBrf.material[j].HasBump();
    *hasSpec = currentBrf.material[j].HasSpec();
    return currentBrf.material[j].diffuseA;
  }
  for (unsigned int i=0; i<filename.size(); i++)
  for (unsigned int j=0; j<file[i].material.size(); j++) {
    BrfMaterial &m(file[i].material[j]);
    if (QString(m.name)==n) {
      *hasBump = m.HasBump();
      *hasSpec = m.HasSpec();
      return m.diffuseA;
    }
  }
  *hasBump = false;
  *hasSpec = false;
  return QString();
}

BrfMaterial* IniData::findMaterial(const QString &name,ObjCoord ) {
  int j = _findByName( currentBrf.material, name);
  if (j>=0) return &(currentBrf.material[j]);

  ObjCoord p = indexOf(name, MATERIAL );
  if (p.fi>=0) return &file[p.fi].material[p.oi];
  else return NULL;
}

BrfShader* IniData::findShader(const QString &name){
  ObjCoord p = indexOf(name, SHADER );
  if (p.fi>=0) return &file[p.fi].shader[p.oi];
  else return NULL;
}

BrfTexture* IniData::findTexture(const QString &fn){

  for (unsigned int i=0; i<file.size(); i++)
  for (unsigned int j=0; j<file[i].texture.size();j++ ){

    const BrfTexture &t(file[i].texture[j]);

    if (t.NFrames()==0) {
      if (!(fn.compare(t.name,Qt::CaseInsensitive))) {
        return &(file[i].texture[j]);
     }
    } else {
        for (int fi=0; fi<t.NFrames(); fi++)
          if (!(fn.compare(QString("%1_%2.dds").arg(t.name).arg(fi),Qt::CaseInsensitive))) {
              return &(file[i].texture[j]);
          }
    }
  }
  return NULL;
}

int IniData::findFile(const QString &fn,bool onlyModFolder) {
  for (unsigned int i=0; i<filename.size(); i++) {
    if (onlyModFolder) if (origin[i]!=MODULE_RES) continue;
    if (QString::compare(fn,filename[i],Qt::CaseInsensitive)==0) return i;
  }
  return -1;
}



void IniData::prepareIndex(int kind){
  if (indexing[kind].empty()) {
    for (unsigned int i=0; i<filename.size(); i++) {
      switch (kind) {
        default:
        case MATERIAL:  _addNames( file[i].material, indexing[kind],i ); break;
        case TEXTURE: _addNamesNoExt( file[i].texture, indexing[kind],i ); break;
        case SHADER: _addNames( file[i].shader, indexing[kind],i ); break;
        case MESH:
          _addNames( file[i].mesh, indexing[kind],i );
          _addNamesNoExt( file[i].mesh, indexing[MESH_NO_EXT],i );
          break;
        case ANIMATION: _addNames( file[i].animation, indexing[kind],i ); break;
        case SKELETON: _addNames( file[i].skeleton, indexing[kind],i ); break;
        case BODY: _addNames( file[i].body, indexing[kind],i ); break;
      }
    }
  }
}

ObjCoord IniData::indexOf(const QString &name, int kind){
  prepareIndex(kind);
  QString st(name);

  int use_list = kind;
  if (kind==TEXTURE)  {
    int k=st.lastIndexOf('.');
    if (k>0) st.truncate(k);
  }
  if (kind==MESH)  {
    int k=st.lastIndexOf('.');
    if (k>0) st.truncate(k);
    use_list = MESH_NO_EXT;
  }
  //assert(!indexing[use_list].empty());
  map<QString,ObjCoord>::iterator p=(indexing[use_list]).find(st);
  if (p==indexing[use_list].end()) return ObjCoord(-1,-1,NONE);
  else
    return p->second;


}

ObjCoord IniData::indexOfStrict(const QString &name, int kind){
  prepareIndex(kind);
  QString st = name;
  if (kind==TEXTURE)  {
    int k=st.lastIndexOf('.');
    if (k>0) st.truncate(k);
  }

  map<QString,ObjCoord>::iterator p=(indexing[kind]).find(st);
  if (p==indexing[kind].end()) return ObjCoord(-1,-1,NONE);
  else return p->second;

}

bool IniData::setPath(QString _mabPath, QString _modPath){
  if (
   QString::compare(mabPath,_mabPath,Qt::CaseInsensitive)==0 &&
   QString::compare(modPath,_modPath,Qt::CaseInsensitive)==0
  ) return false;
  updated=0;
  mabPath=_mabPath;
  modPath=_modPath;
  return true;
}

QString IniData::name() const{
  return modPath;
}


bool IniData::loadAll(int howFast){

	errorStringOnScan = QString("Unspecified error");

	if (updated>=howFast) return true; // true = no error

	//int oldLvl = updated;
	updated = howFast;

	errorListOnLoad.clear();
	QFile f(modPath+"/module.ini");
	if (!f.open( QIODevice::ReadOnly| QIODevice::Text )) return false;
	char st[255];
	for (unsigned int i=0; i<file.size(); i++) file[i].Clear();
	file.clear();
	filename.clear();
	origin.clear();

	bool res=true;

	{
		// load core resources...
		addBrfFile("core_shaders",CORE_RES,0,howFast);
		addBrfFile("core_textures",CORE_RES,0,howFast);
		addBrfFile("core_materials",CORE_RES,0,howFast);
		addBrfFile("core_pictures",CORE_RES,0,howFast);
		addBrfFile("core_ui_meshes",CORE_RES,0,howFast);


		int lineN = 1;
		while (f.readLine(st,254)>-1)  {

			QString s = QString("%1").arg(st);
			lineN ++;

			// remove commented part
			int posOfComm = s.indexOf('#');
			if (posOfComm>-1) s.truncate(posOfComm);

			s = s.trimmed(); // removal of spaces
			if (s.isEmpty()) continue; // skip empty lines (including comments)
			QString com1, com2;
			//char com1[512], com2[512];
			QStringList p = s.split('=');
			//if (sscanf(s.toAscii().data(),"%s = %s",com1, com2)==2)
			if (p.size()==2){
				com1 = p[0].trimmed();
				com2 = p[1].trimmed();
				bool loadRes = QString(com1)=="load_resource";
				bool loadMod = ((QString(com1)=="load_mod_resource") || (QString(com1)=="load_module_resource"));
				if (loadRes || loadMod) {
					if (!addBrfFile(com2.toAscii().data(),(loadMod)?MODULE_RES:COMMON_RES,lineN,howFast)) res=false;
				}

			}
		}

		/*
		// DISCOVER ALL USED FLAGS
		for (uint fn=16; fn<17; fn++) {
			int nk=0;
			QString res;
			for (uint fi=0; fi<file.size(); fi++) {
				for (uint mi=0; mi<file[fi].material.size(); mi++) {
					unsigned int b = file[fi].material[mi].flags; // & ~(3<<16);

					QString t;
					t.sprintf("%s(0x%X) ",file[fi].material[mi].name,b);
					if (b&(1<<fn)) {
						if (nk<100 || (nk%10==0)) {
							res+=t;
							if (nk>=100) res+=" ... \n";
						};
						nk++;
					}
				}
			}
			if (nk>0)
				qDebug("[spoiler=textures with flag: %d]\n%s\n[/spoiler]",fn,res.toAscii().data());
		}
		*/

		updateNeededLists();
	}
	if (howFast>=3){
		res = true;
		updateUsedBy();
		if (updated>=3){

			isWarband = false; // we try MAB first...
			if (!readModuleTxts(modPath,mabPath+"/Data")) res = false;
			// remove duplicates
			for (int i=0; i<int(txtNameList.size()); i++)
				txtNameList[i].name=txtNameList[i].name.toSet().toList();
			updateUsedIn();
			propagateUsedIn();

			//debugShowUsedFlags();
		}
	}

	return res;
}


#include <set>

void IniData::debugShowUsedFlags(){
  // temp...
  std::set<uint> cap, sph, fac; std::set<int> man;
  for (uint i=0; i<file.size(); i++)
  for (uint j=0; j<file[i].body.size(); j++)
  for (uint k=0; k<file[i].body[j].part.size(); k++){
    BrfBodyPart &p(file[i].body[j].part[k]);
    switch (p.type){
    case BrfBodyPart::MANIFOLD: man.insert(p.ori);break;
    case BrfBodyPart::CAPSULE: cap.insert(p.flags);break;
    case BrfBodyPart::SPHERE: sph.insert(p.flags);break;
    case BrfBodyPart::FACE: fac.insert(p.flags);break;
	default: assert(0); break;
    }
  }
  qDebug("CAP");for (std::set<uint>::iterator i = cap.begin(); i!=cap.end(); i++) qDebug("%x",*i);
  qDebug("SPH");for (std::set<uint>::iterator i = sph.begin(); i!=sph.end(); i++) qDebug("%x",*i);
  qDebug("MAN");for (std::set< int>::iterator i = man.begin(); i!=man.end(); i++) qDebug("%x",*i);
  qDebug("FAC");for (std::set<uint>::iterator i = fac.begin(); i!=fac.end(); i++) qDebug("%x",*i);
}

void IniData::updateUsedIn(){
  setAllUsedInNone();
  for (unsigned int i=0; i<txtNameList.size(); i++){
    ModuleTxtNameList &list(txtNameList[i]);
    for (int j=0; j<(int)list.name.size(); j++){
      ObjCoord oc =indexOf( list.name[j], list.brfToken  );

      if (oc.fi>=0) {
        usedIn(oc).direct |= bitMask( list.txtIndex );

        // special: mesh and materials are used also by other files
        if ((list.brfToken==MESH)||(list.brfToken==MATERIAL) ){
          QString nameDot=noDot(list.name[j]);

          for (unsigned int k=0; k<file[oc.fi].size(oc.t); k++){

            QString objname = noDot(file[oc.fi].GetName(k,oc.t));
            // special2: in item, if there is a _L (or _R), then also _Lx (or _Rx)
            if ((list.brfToken==MESH)&&(list.txtIndex==TXTFILE_ITEM)){
              if (objname.endsWith("_Lx")||objname.endsWith("_Rx")) objname.chop(1);
            }

            if (objname==nameDot)
              usedIn(ObjCoord(oc.fi,k,oc.t)).direct |= bitMask( list.txtIndex );
          }

        }
      } else {
        errorListOnLoad.push_back(
          QTextBrowser::tr("<b>Missing in txt:</b> cannot find %1 <u>%2</u>, referred in '%3'")
          .arg(tokenFullName(list.brfToken)).arg(list.name[j]).arg(txtFileName[list.txtIndex])
        );

      }
    }
  }

  // all core files are used as "core"
  ObjCoord oc;
  for (oc.fi=0; oc.fi<(int)file.size(); oc.fi++){
    if (origin[oc.fi] == CORE_RES ){
      for (oc.t=0; oc.t<N_TOKEN; oc.t++){
        for (oc.oi=0; oc.oi<int(usedInV[oc.t].at(oc.fi).size()); oc.oi++)
          usedIn(oc).direct|=bitMask(TXTFILE_CORE);
      }
    }
  }
}

void IniData::updateBeacuseBrfDataSaved(){
  if (updated>=1) {
    updateNeededLists();
  }
  if (updated>=3) {
    updateUsedBy();
  }
  if (updated>=4) {
    updateUsedIn();
    propagateUsedIn();
  }
}

int IniData::nRefObjects() const{
  int res=0;
  for (unsigned int i=0; i<file.size(); i++) {
    res+=file[i].texture.size();
    res+=file[i].material.size();
    res+=file[i].shader.size();
  }
  return res;
}

int IniData::nObjects() const{
  int res=0;
  for (unsigned int i=0; i<file.size(); i++) {
    res+=file[i].texture.size();
    res+=file[i].material.size();
    res+=file[i].body.size();
    res+=file[i].mesh.size();
    res+=file[i].shader.size();
    res+=file[i].animation.size();
    res+=file[i].skeleton.size();
  }
  return res;
}

bool IniData::addBrfFile(const char* name, Origin ori, int line, int howFast){
  QString brfFn, brfPath;
  if (ori == MODULE_RES) {
    brfFn = modPath + "/Resource/" +name +".brf";
    brfPath = modPath + "/Resource";
  }
  else {
    brfFn = mabPath + "/CommonRes/" + name +".brf";
    brfPath = mabPath + "/CommonRes";
  }
  iniLine.push_back(line);
  file.push_back(BrfData());
  filename.push_back(brfFn);
  origin.push_back(ori);
  BrfData &d(file[file.size()-1]);
  //printf("Loading \"%s\"...\n",brfFn.toAscii().data());
  bool onlyMatAndTextures = (howFast<=2);
  if (howFast>1) {
    if (!d.LoadFast(brfFn.toStdWString().c_str(),onlyMatAndTextures)) {

      // ERROR!!
      if (!QDir(brfPath).exists( QString("%1.brf").arg(name)))
      errorListOnLoad.push_back(QTextBrowser::tr("<b>File-Not-Found:</b> could not read brf file <u>%1</u>, listed in module.ini file")
         .arg(shortFileName(file.size()-1)));
      else
      errorListOnLoad.push_back(QTextBrowser::tr("<b>File-Format Error:</b> could not read brf file <u>%1</u>")
         .arg(shortFileName(file.size()-1)));

      //file.pop_back();
      //filename.pop_back(brfFn);
      return false;
    } else {
      /*
        // TEST!
      for (uint i=0; i<d.mesh.size(); i++) if (d.mesh[i].HasTangentField()){
        int b = 0;
        for (uint j=0; j<d.mesh[i].vert.size(); j++) {
          int nb = d.mesh[i].vert[j].ti;
          if ((nb!=0)) b = nb;
        }
        if (b!=0) qDebug("mesh %s has ti = 0x%X",d.mesh[i].name, b);
      }
      */
    }
  } else d.Clear();
  return true;
}

template <class T>
void _updateList(QStringList &l, const vector<T> &d){
  for (unsigned int i=0; i<d.size(); i++) {
    l.append(d[i].name);
  }
}

template <class T>
void _updateListNoExt(QStringList &l, const vector<T> &d){
  for (unsigned int i=0; i<d.size(); i++) {
    QString s(d[i].name);
    int p = s.indexOf(".");
    if (p>0) s.truncate( p );
    l.append(s);
  }
}

void IniData::updateAllLists(){
  for (int i=0; i<N_TOKEN; i++)  namelist[i].clear();
  for (unsigned int i=0; i<file.size(); i++) {
    _updateListNoExt( namelist[TEXTURE], file[i].texture);
    _updateList( namelist[MATERIAL], file[i].material);
    _updateList( namelist[SHADER], file[i].shader);
    _updateListNoExt( namelist[MESH], file[i].mesh);
    _updateList( namelist[BODY], file[i].body);
    _updateList( namelist[ANIMATION], file[i].animation);
    _updateList( namelist[SKELETON], file[i].skeleton);
  }
  //namelist[TEXTURE].append("none");

  for (int i=0; i<N_TOKEN; i++) {
    // for safety, replace bad characters with underscores
    namelist[i] = namelist[i].replaceInStrings(" " , "_");
    namelist[i] = namelist[i].replaceInStrings("," , "_");
    namelist[i].sort();
    namelist[i].removeDuplicates();
  }

}

bool IniData::saveLists(const QString &fn){
  QFile f(fn);
  f.open(QIODevice::WriteOnly);
  if (!f.isOpen()) return false;
  for (int i=0; i<N_TOKEN; i++) {
    int s = namelist[i].size();
    f.write( QString("[%1] %2:\n").arg(tokenBrfName[i]).arg(s).toAscii().data());
    for (int j=0; j<s; j++){
      f.write( QString("%1,").arg(namelist[i][j]).toAscii().data() );
    }
    f.write( "\n" );
  }
  f.write("[end].\n");
  return true;
}

void IniData::updateNeededLists(){
  for (int i=0; i<N_TOKEN; i++)
    namelist[i].clear();
  for (unsigned int i=0; i<file.size(); i++) {
    _updateListNoExt( namelist[TEXTURE], file[i].texture);
    _updateList( namelist[MATERIAL], file[i].material);
    _updateList( namelist[SHADER], file[i].shader);
    // NO NEED:
    //_updateList( namelist[MESH], file[i].mesh);
    //_updateList( namelist[BODY], file[i].body);
    //_updateList( namelist[ANIMATION], file[i].animation);
    //_updateList( namelist[SKELETON], file[i].skeleton);
  }
  namelist[TEXTURE].append("none");

  // clear indexing
  for (int i=0; i<N_TOKEN+1; i++) indexing[i].clear();

}
/*
QStringList& IniData::nameList(int kind) const{
  return namelist[kind];
}*/




//Pair IniData::indexOf(const QString &name, int kind){
//}


