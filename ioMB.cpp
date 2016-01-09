/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <map>
#include <stdio.h>

#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>

using namespace std;
using namespace vcg;

#include "brfMesh.h"
#include "brfSkeleton.h"
#include "brfAnimation.h"
#include "platform.h"

#include "ioMB.h"

#include <QString>
#include <QFile>
#include <QDebug>


static FILE *f; // mmmm a good old global
static bool lineEnd;
static int lineN;
static QString lastErr;
static QString nextT;
static FILE *debug;
static const int SCALE = 20;

Matrix44f euler2matrix(float* eul);
float* matrix2euler(const Matrix44f &_m);



static QString token(){
  if (!nextT.isEmpty()) {
    QString res = nextT;
    nextT.clear();
    //fprintf(debug,"reuse:[%s]\n",res.toAscii().data());fflush(debug);
    return res;
  }

  QString res;
  char str[1024];
  int ttt = fscanf(f,"%s",str);

  if (ttt==0 || ttt==EOF) { fprintf(debug,"[-----]\n");fflush(debug); return res; }
  //qDebug("just read token: '%s'",str);

  res = QString("%1").arg(str);
  lineEnd = false;
  res = res.trimmed();
  if (res.endsWith(";")) {
    if (res.size()>1) res.chop(1);
    lineEnd=true;
  }

  //fprintf(debug,"[%s]\n",res.toAscii().data());fflush(debug);
  return res;
}

static bool expect(const char* st){
  QString t = token();
  if (t!=st) {
    lastErr= QString("expected '%1' got '%2'").arg(st).arg(t);
    return false;
  }
  return true;
}

static int oneOf(const char* st1, const char* st2){
  QString t = token();
  if (t==st1) return 1;
  else if (t==st2) return 2;
  else {
    lastErr= QString("expected '%1' or '%2', got '%3'").arg(st1).arg(st2).arg(t);
    return 0;
  }
}

static bool expectType(const char* st){
  if (!expect("-type")) return false;
  char s[512];
  sprintf(s,"\"%s\"",st);
  if (!expect(s)) return false;
  return true;
}

static QString readStrQuotes(){
  QString t = token();
  return t.replace("\"","");
}

static void readStrQuotesChar(char*c){
  sprintf(c,"%s",readStrQuotes().toAscii().data());
}


static void skipLine(){
  int guard=0;
  while (!lineEnd) {
    assert(guard++<20000);
    if (token().isEmpty()) return;
  }
}

static bool nextSetAttr(){
  int guard =0;
  while (1) {
    QString t = token();
    if (t.isEmpty()) return false;
    if (t=="setAttr") return true;
    if (t=="createNode") {
      nextT = t;
      return false;
    }
    skipLine();
    assert(guard++<10000);
  }
}

static void skipCreate(){
  int guard =0;
  while (1) {
    QString t = token();
    if (t.isEmpty()) break;
    if (t=="createNode") {
      nextT = t;
      break;
    }
    assert(guard++<100000);
  }
}

static bool nextCreateNode(){
  int guard =0;
  while (1) {
    QString t = token();
    if (t.isEmpty()) return false;
    if (t=="createNode") {
      return true;
    }
    assert(guard++<100000);
  }
}

/*
static void testNext(){
  long pos = ftell(f);

  char str[1024];
  fscanf(f,"%s",str);

  qDebug("next token: '%s'",str);

  fseek(f,pos,SEEK_SET);
}*/

static float readFloat(){
  float res=0;
  /*long pos = ftell(f);
    char tmp[1024];
    fscanf(f, "%s",tmp);
    qDebug("ReadingA [%s]...",tmp);
    fseek(f,pos,SEEK_SET);*/
/*    fscanf(f, "%s",tmp);
    qDebug("ReadingB [%s]...",tmp);
    fseek(f,pos,SEEK_SET);*/

  int i=fscanf(f,"%f",&res);
  /*if (!i) qDebug("cannot read A; res = %f",res);
  if (!i) {
    fseek(f,pos,SEEK_SET);
    char tmp[1024];
    fscanf(f, "%s",tmp);
    qDebug("Reading [%s]...",tmp);
    fseek(f,pos,SEEK_SET);
    int resi=0;
    i = fscanf(f,"%d",&resi); res=resi;
  }*/
  if (!i) qDebug("cannot read B; res = %f",res);
  assert(i);
  return res;
}

static int readInt(){
  int res;
  int i=fscanf(f,"%d",&res);
  assert(i);
  return res;
}

static vcg::Point3f readPoint(){
  vcg::Point3f res;
  //res.X() = -readFloat();
  //res.Z() =  readFloat();
  //res.Y() = -readFloat();
  res.X() =  readFloat();
  res.Y() =  readFloat();
  res.Z() =  readFloat();
  //fprintf(debug,"[%f %f %f]\n",res.X(), res.Z(), res.Y() ); fflush(debug);
  return res;
}

static void  writePoint(vcg::Point3f p){
  fprintf(f," %f %f %f",-p[0], p[2],-p[1]);
}


static vcg::Point2f readPoint2(){
  vcg::Point2f res;
  res.X() = readFloat();
  res.Y() = 1.0-readFloat();
  return res;
}

static void  writePoint2(vcg::Point2f p){
  fprintf(f," %f %f",p[0],1-p[1]);
}


static int readSize(){
  return readInt();
}

static vcg::Point3f readPointRot(){
  vcg::Point3f res;
  res.X() = readFloat();
  res.Y() = readFloat();
  res.Z() = readFloat();

  return res*(M_PI/180.0);
}

static char lastIntervalName[255];
static bool readInterval(QString s, const char* token, int*a, int *b){
  QString t = QString("\"%1[").arg(token);
  sprintf(lastIntervalName, "%s", token);
  if (s.startsWith(t)) {
    QString format = QString("\"%1[%2d:%2d]").arg(token).arg('%');
    sscanf(s.toAscii().data(),format.toAscii().data(),a,b);
    return true;
  }
  return false;
}

static bool readInterval(QString s, const char* tokenA, int *n, const char* tokenB, int*a, int *b){
  QString t = QString("\"%1[").arg(tokenA);
  sprintf(lastIntervalName, "%s...%s", tokenA,tokenB);
  if (s.startsWith(t)) {
    QString format1 = QString("\"%1[%2d].%3[%2d:%2d]").arg(tokenA).arg('%').arg(tokenB);
    QString format2 = QString("\"%1[%2d].%3[%2d]").arg(tokenA).arg('%').arg(tokenB);
    //qDebug()<<format1<< " and " << format2 << " for [" << s << "]";
    int x,y;
    if ((x=sscanf(s.toAscii().data(),format1.toAscii().data(),n,a,b))==3) return true;
    if ((y=sscanf(s.toAscii().data(),format2.toAscii().data(),n,a))==2) { *b=*a; return true;}
    //qDebug()<<"FAILED"<<x<<y;
    return false;
  }
  return false;
}

static bool checkInterval(int a, int b, int max){
  if (b>max) {
    lastErr=QString("Invalid %4 interval %1 %2 not in [0..%3]")
            .arg(a).arg(b).arg(max).arg(lastIntervalName);
    return false;
  }
  return true;
}


class Edge{public:
  Edge(int _a, int _b) {a=_a; b=_b;}
  Edge() {}
  int a; int b;
  bool operator < (const Edge &e) const{
    if (a<e.a) return true;
    return (b<e.b);
  }
  void flip(){ int t=a; a=b; b=t;}
};

static Edge readEdge(){
  Edge e;
  e.a=readInt();
  e.b=readInt();
  int third = readInt();
  assert(third==0);
  return e;
}

static char* substitute(const char* a, char from, char to){
  static char res[1024];
  sprintf(res,"%s",a);
  int i=0;
  while (res[i]!=0) {
    if (res[i]==from) res[i]=to;
    i++;
  }
  return res;
}

static void writeEdge(const Edge &e){
  fprintf(f," %d %d 0",e.a,e.b);
}


static void ioMB_exportRigging(const BrfMesh &m){
  fprintf(f,
    "createNode skinCluster -n \"%s_skin\";\n"
    "\tsetAttr -s %ld \".wl\";\n"
    ,m.name, m.rigging.size()
  );
  for (unsigned int i=0; i<m.rigging.size(); i++) {
    int minj = m.rigging[i].boneIndex[0];
    int maxj = minj;
    int nj=0;
    for (int j=0; j<4; j++) {
      if (m.rigging[i].boneWeight[j]>0) {
        int k = m.rigging[i].boneIndex[j];
        if (maxj<k) maxj=k;
        if (minj>k) minj=k;
        nj++;
      }
    }
    assert(nj!=0);
    if (nj==1)
      fprintf(f,
        "\tsetAttr \".wl[%d].w[%d]\"  1;\n",
        i,minj
      );
    else if (nj == maxj-minj+1) {
      fprintf(f,
        "\tsetAttr -s %d \".wl[%d].w[%d:%d]\"",
        maxj-minj+1,i,minj,maxj
      );
      for (int j=minj; j<=maxj; j++) fprintf(f," %f",m.rigging[i].WeightOf(j));
      fprintf(f,";\n");
    } else {
      fprintf(f,
        "\tsetAttr -s %d \".wl[%d].w\";\n",
        nj,i
      );
      int test=0;
      for (int j=minj; j<=maxj; j++) {
        float w = m.rigging[i].WeightOf(j);
        if (w!=0) {
          fprintf(f,"\tsetAttr \".wl[%d].w[%d]\" %f;\n",i,j,w);
          test++;
        }
      }
      assert(test==nj);
    }
  } 
}

static void ioMB_exportSkeletonAfter(const BrfSkeleton &s){
  for (unsigned int i=0; i<s.bone.size(); i++) {
    fprintf(f,
       "connectAttr \"%s\" \"skinCluster1.ma[%d]\";\n",
      substitute(s.bone[i].name,'.','_'),i
    );

  }

}

static void ioMB_exportMesh(const BrfMesh &m, int fr){

  fprintf(f,
    "createNode transform -n \"myTransform%d\";\n"
    "\tsetAttr \".t\" -type \"double3\" 0.0 0.0 0.0 ;\n"
    ,1
  );
  fprintf(f,
//    "createNode transform -n \"transform%d\" -p \"objpolySurface%d\";\n"
    "createNode mesh -n \"%s\"  -p \"myTransform%d\";\n"
//    ,1,1
    ,m.name,1
  );
/*
  fprintf(f,
    "createNode mesh -n \"%s\";\n"
    ,m.name
  );*/
  // vertex texture coords
  fprintf(f,
    "\tsetAttr -k off \".v\";\n"
    "\tsetAttr \".io\" yes;\n"
    "\tsetAttr \".iog[0].og[0].gcl\" -type \"componentList\" 1 \"f[0:%ld]\";\n"
    "\tsetAttr \".uvst[0].uvsn\" -type \"string\" \"%s\";\n"
    "\tsetAttr -s %ld \".uvst[0].uvsp\";\n"
    "\tsetAttr \".uvst[0].uvsp[0:%ld]\" -type \"float2\"",
    m.face.size()-1,m.material,  m.vert.size(), m.vert.size()-1
  );

  for (unsigned int i=0,k=4; i<m.vert.size(); i++,k+=2) {
    if (k>9) { fprintf(f,"\n\t\t"); k=0; }
    writePoint2( m.vert[i].ta );
  }
  fprintf(f,";\n");

  fprintf(f,
    "\tsetAttr \".cuvs\" -type \"string\" \"map1\";\n"
    "\tsetAttr \".dcc\" -type \"string\" \"Ambient+Diffuse\";\n"
  );
  // positions
  fprintf(f,
    "\tsetAttr -s %ld \".vt\";\n"
    "\tsetAttr \".vt[0:%ld]\"",
    m.frame[fr].pos.size(),m.frame[fr].pos.size()-1
  );
  for (unsigned int i=0,k=2; i<m.frame[fr].pos.size(); i++,k+=3) {
    if (k>9) { fprintf(f,"\n\t\t"); k=0; }
    writePoint( m.frame[fr].pos[i]*SCALE );
  }
  fprintf(f,";\n");

  // edges
  // convert face structure into edge structure
  std::vector<Edge> tmped;
  std::vector<BrfFace> tmpfa;
  std::map<Edge,int> map;
  typedef std::map<Edge,int>::iterator MapIte;

  tmpfa.resize(m.face.size());
  for (unsigned int i=0; i<m.face.size(); i++)
  for (int e=0; e<3; e++) {
    Edge edg(
      m.vert[ m.face[i].index[e      ] ].index,
      m.vert[ m.face[i].index[(e+1)%3] ].index
    );
    MapIte mi;
    mi=map.find(edg);
    if (mi!=map.end() ) {
      tmpfa[i].index[e] = mi->second;
      continue;
    }
    edg.flip();
    mi=map.find(edg);
    if (mi!=map.end()){
      tmpfa[i].index[e] = -mi->second-1;
      continue;
    }

    edg.flip();
    tmpfa[i].index[e] = map[edg] = tmped.size();
    tmped.push_back(edg);
  }


  fprintf(f,
    "\tsetAttr -s %ld \".ed\";\n"
    "\tsetAttr \".ed[0:%ld]\"",
    tmped.size(),tmped.size()-1
  );
  for (unsigned int i=0,k=2; i<tmped.size(); i++,k+=3){
     if (k>9) { fprintf(f,"\n\t\t"); k=0; }
     writeEdge(tmped[i]);
  }
  fprintf(f,";\n");

  // normals
  int vertsize = m.face.size()*3;

  fprintf(f,
    "\tsetAttr -s %d \".n\";\n"
    "\tsetAttr \".n[0:%d]\" -type \"float3\"",
    vertsize, vertsize-1
  );
  for (unsigned int i=0,k=4; i<m.face.size(); i++)
  for (int w=0; w<3; w++,k+=3)
  {
    if (k>9) { fprintf(f,"\n\t\t"); k=0; }
    writePoint( m.frame[fr].norm[ m.face[i].index[w] ] );
  }
  fprintf(f,";\n");

  // faces
  fprintf(f,
    "\tsetAttr -s %ld \".fc\";\n"
    "\tsetAttr \".fc[0:%ld]\" -type \"polyFaces\"\n" ,
    m.face.size(), m.face.size()-1
  );
  for (unsigned int i=0,k=4; i<m.face.size(); i++,k+=3) {
    fprintf(f,
     "\t\tf 3 %d %d %d\n"
     "\t\tmu 0 3 %d %d %d%c",
     tmpfa[ i ].index[0],tmpfa[ i ].index[1],tmpfa[ i ].index[2],
     m.face[i].index[0],m.face[i].index[1],m.face[i].index[2],
     (i==m.face.size()-1)?' ':'\n'
    );
  }
  fprintf(f,";\n");

  fprintf(f,
    "\tsetAttr \".cd\" -type \"dataPolyComponent\" Index_Data Edge 0 ;\n"
    "\tsetAttr \".cvd\" -type \"dataPolyComponent\" Index_Data Vertex 0 ;\n"
    "\tsetAttr \".tgsp\" 1;\n"
   );

}

static int ioMB_importRiggingSize(){
  while (nextSetAttr()) {
    QString t =  token();
    if (t=="-s") {
      int k = readSize();
      expect("\".wl\"");
      return k;
    }
  }
  return 0;
}

static int ioMB_importRigging(BrfMesh &m){
  int a,b,n;
  int max = m.frame[0].pos.size();
  m.rigging.resize(max);
  //qDebug("Start...");

  //int last = 0;

  while (nextSetAttr()) {
    QString t =  token();
    //testNext();
    //testNext();
    //qDebug("token:[[[%s]]]",t.toAscii().data());

    if (t=="-s") {
      readInt(); // skip size
      t =  token();
      //qDebug("skip: %d",k);
    }
    //testNext();

    if (readInterval(t,".wl",&n,"w",&a,&b)) {
     //qDebug("%d [%d %d]",n,a,b);
     assert(a>=0);
     if (m.maxBone<b) m.maxBone=b;
     for (int i=a; i<=b; i++) {
       if (n>=max) return false; // overflow!
       //testNext();
       float w = readFloat();
       //qDebug("adding (%d,%f) to pos %d",i,w,n);
       m.rigging[n].Add(i,w);
       //m.rigging[n].Add(0,1);



       //if ((n!=last) && (n!=last+1)) qDebug("From %d to %d",last,n);
       //last = n;
     }

     if (!expect(";")) return false;
    }

  }
  //qDebug("Last was %d on %d -- max bone =%d",last,max,m.maxBone);
  return true;
}
static bool ioMB_importMesh(BrfMesh &m ){
  //if (!expect("-n")) return false;
  //QString parentName = readStrQuotes(  );
  skipLine(); // -n "..."
  std::vector<vcg::Point2f> tmptc;

  std::vector<Edge> tmped;

  m.frame.resize(1);
  m.vert.clear();
  m.frame[0].pos.clear();
  m.face.clear();
  m.material[0]=0;
  m.maxBone=0;
  m.rigging.clear();

  int vcount =0;
  int fsize = 0;

  while (nextSetAttr()) {
    QString t =  token();
    int a, b;
    if (t=="-s") {
      int siz = readSize();
      QString an = readStrQuotes(  );
      if (an==".uvst[0].uvsp") tmptc.resize(siz);
      if (an==".vt") m.frame[0].pos.resize(siz);
      if (an==".ed") tmped.resize(siz);
      if (an==".n") m.vert.resize(siz);
      if (an==".fc") fsize = siz; //m.face.resize(siz);
    }
    else if (readInterval(t, ".uvst[0].uvsp",&a, &b)) {
      if (a==0) if (!expectType("float2")) return false;
      if (!checkInterval(a,b,tmptc.size())) return false;

      for (int i=a; i<=b; i++) tmptc[i]=readPoint2();
      if (!expect(";")) return false;
    }
    else if (readInterval(t, ".vt",&a, &b)) {
      if (!checkInterval(a,b,m.frame[0].pos.size())) return false;
      for (int i=a; i<=b; i++) m.frame[0].pos[i]=readPoint()/SCALE;// /4;
      if (!expect(";")) return false;
    }
    else if (readInterval(t, ".ed",&a, &b)) {
      if (!checkInterval(a,b,tmped.size())) return false;
      for (int i=a; i<=b; i++)  tmped[i] = readEdge();
      if (!expect(";")) return false;
    }
    else if (readInterval(t, ".n",&a, &b)) {
      if (!expectType("float3")) return false;
      if (!checkInterval(a,b,m.vert.size())) return false;
      for (int i=a; i<=b; i++) m.vert[i].__norm=readPoint();
      if (!expect(";")) return false;
    }
    else if (readInterval(t, ".fc",&a, &b)) {
      if (!checkInterval(a,b,fsize)) return false; //m.face.size())) return false;
      if (a==0) expectType("polyFaces");
      //assert(m.face.size()*3 == m.vert.size() );
      for (int i=a; i<=b; i++) {
        if (!expect("f")) return false;
        int ne = readInt();
        if (ne!=3 && ne!=4) {
          lastErr = QString("Polygon with %1 faces (I want only tri and quads)").arg(ne);
          return false;
        }

        //if (!expect("3")) return false;
        int e0, e1, e2, /*e3,*/ tc0, tc1, tc2, tc3;
        e0 = readInt();
        e1 = readInt();
        e2 = readInt();
        if (ne==4) readInt();
        //fprintf(debug,"%d [%d %d %d]\n",i,e0,e1,e2); fflush(debug);
        if (!expect("mu")) return false;
        if (!expect("0")) return false;
        int ne2 = readInt();
        if (ne2!=ne) {
          lastErr = QString("Polygon size: %1 then %2 ???").arg(ne).arg(ne2);
          return false;
        }
        tc0 = readInt();
        tc1 = readInt();
        tc2 = readInt();
        if (ne==4) tc3 = readInt();
        int v0a = (e0>=0)?tmped[e0].a:tmped[-1-e0].b;
        int v0b = (e0>=0)?tmped[e0].b:tmped[-1-e0].a;
        int v1a = (e1>=0)?tmped[e1].a:tmped[-1-e1].b;
        int v1b = (e1>=0)?tmped[e1].b:tmped[-1-e1].a;
        if (v0b!=v1a) { lastErr=QString("Edge mismatch at face %1 (edges %2 %3)...").arg(i).arg(e0).arg(e1); return false;}

        int v2a, v2b;
        if (ne==4) {
          v2a = (e2>=0)?tmped[e2].a:tmped[-1-e2].b;
          v2b = (e2>=0)?tmped[e2].b:tmped[-1-e2].a;
          if (v1b!=v2a) {
            lastErr=QString("Edge mismatch at face %1 (edges %2 %3 %4)...").arg(i).arg(e0).arg(e1).arg(e2);
            return false;
          }
        }

        //m.face[i]=BrfFace(v0a,v1a,v1b);
        //m.face.push_back(BrfFace(v0a,v1a,v1b));

        int v0, v1, v2, v3;
        //v0=i*3;
        //v1=i*3+1;
        //v2=i*3+2;
        v0 = vcount++;
        v1 = vcount++;
        v2 = vcount++;
        v3 = 0; // What was really intended when ne!=4? 
        if (ne==4) v3 = vcount++;

        //m.face[fcount]=BrfFace(v0,v1,v2);
        m.face.push_back(BrfFace(v1,v0,v2));
        if (ne==4) m.face.push_back(BrfFace(v3,v2,v0));

        m.vert[v0].index=v0a;
        m.vert[v1].index=v1a;
        m.vert[v2].index=v1b;
        if (ne==4) m.vert[v3].index = v2b;

        m.vert[v0].ta = m.vert[v0].tb = tmptc[tc0];
        m.vert[v1].ta = m.vert[v1].tb = tmptc[tc1];
        m.vert[v2].ta = m.vert[v2].tb = tmptc[tc2];
        if (ne==4) m.vert[v3].ta = m.vert[v3].tb = tmptc[tc3];
      }
      if (!expect(";")) return false;
    }
    //else if (t=="\".tgsp\"") break; else skipLine();
  }

  m.frame[0].norm.resize(m.vert.size());
  for (unsigned int i=0; i<m.vert.size(); i++) {
    m.frame[0].norm[i] = m.vert[i].__norm;
    m.vert[i].col=0xFFFFFFFF;
  }
  /*if (m.face.size()==0) {
    lastErr=QString("Mesh with no faces?");
    return false;
  }*/

  m.AfterLoad();
  m.flags=0;

  return true;

}


static void ioMB_exportHeader(){

  fprintf(f,
     "// created by OpenBrf, Marco Tarini\n"
     "// exporting from a BRF resource file\n"
    // "// rigged mesh: %s"
    // "// skeleton mesh: %s"
"requires maya \"2008\";\n"
"currentUnit -l centimeter -a degree -t film;\n"
//"fileInfo \"application\" \"maya\";\n"
//"fileInfo \"product\" \"Maya Complete 2008\";\n"
//"fileInfo \"version\" \"2008 Extension 2 x64\";\n"
//"fileInfo \"cutIdentifier\" \"200802242349-718079\";\n"
//"fileInfo \"osv\" \"Microsoft Windows Vista Service Pack 1 (Build 6001)\\n\";\n"
  );
}


static void ioMB_exportSkeleton(const BrfSkeleton &s){
  for (int i=0; i<(int)s.bone.size(); i++) {
    const BrfBone &bone(s.bone[i]);

    fprintf(f,"createNode joint -n \"%s\" ",substitute(bone.name,'.','_'));
    if (bone.attach!=-1)
    fprintf(f,"-p \"%s\" ",substitute(s.bone[bone.attach].name,'.','_'));
    fprintf(f,";\n");

    float *abc;
    abc = matrix2euler(s.getRotationMatrix( i ).transpose() );
    Point3f t = bone.t;
    if (bone.attach==-1) { float tmp=t[1]; t[1]=t[2]; t[2]=tmp;}
    t.X()*=-1;
    t.Y()*=-1;
    t*=SCALE;

    fprintf(f,
      "\taddAttr -ci true -sn \"liw\" -ln \"lockInfluenceWeights\" -bt \"lock\" -min 0 -max 1 -at \"bool\";\n"
      "\tsetAttr \".uoc\" yes;\n"
      "\tsetAttr \".t\" -type \"double3\" %f %f %f ;\n"
      "\tsetAttr \".r\" -type \"double3\" %f %f %f ;\n"
      "\tsetAttr \".mnrl\" -type \"double3\" -360 -360 -360 ;\n"
      "\tsetAttr \".mxrl\" -type \"double3\" 360 360 360 ;\n",
      -t[0],t[2],-t[1],
      abc[0]*180/M_PI,abc[1]*180/M_PI,abc[2]*180/M_PI
    );

    //qDebug()<< "[" << i << "]: abc" << abc[0]*180/M_PI << abc[1]*180/M_PI <<abc[2]*180/M_PI <<"\n";
  }
}

bool IoMB::Export(const wchar_t*filename, const BrfMesh &m , const BrfSkeleton &s, int fi){
  f = wfopen(filename,"wt");
  if (!f) return false;
  ioMB_exportHeader();
  ioMB_exportSkeleton(s);
  ioMB_exportMesh(m,fi);
  ioMB_exportRigging(m);
  ioMB_exportSkeletonAfter(s);

  fclose(f);
  return 0;
}

static Point3f globalScale;
static bool ioMB_importBone(BrfSkeleton &s ){
  if (!expect( "-n" )) return false;
  BrfBone b;
  b.attach = -1;

  readStrQuotesChar( b.name );

  if (!lineEnd) {
    int tmp = oneOf("-p",";");
    if (!tmp) return false;
    if (tmp==1) {
      QString parentName = readStrQuotes(  );
      b.attach = s.FindBoneByName(parentName.toAscii().data());
      if (b.attach==-1) {
        lastErr = QString("Can't find bone \"%1\"").arg(parentName);
        return false;
      }
    }
  } else {
    if (s.root != -1) {
      lastErr = QString("Found multiple skeleton's roots (bone \"%1\"").arg(b.name);
      return false;
    }
    s.root = s.bone.size();
  }
  skipLine();
  bool hasT=false;
  bool hasR=false;
  vcg::Point3f rot;
  while (nextSetAttr()) {
    QString t =  token();
    if (t=="\".t\"") {
      if (!expectType("double3")) return false;
      b.t = readPoint()/SCALE;
      b.t.X()*=-1;
      b.t.Y()*=-1;
      if (b.attach==-1) { float tmp=b.t[1]; b.t[1]=b.t[2]; b.t[2]=tmp;}
      hasT=true;
      //b.t*=3.0;
    }
    if (t=="\".r\"") {
      if (!expectType("double3")) return false;
      rot = readPointRot();
      //rot[2]*=-1;
      hasR=true;
    }
    if (t=="\".bps\"") {
      qDebug("bps");
      if (!expectType("matrix")) return false;


      for (int i=0; i<4; i++) {
        float a,b,c,d;
        a = readFloat();b = readFloat();c = readFloat();d = readFloat();
        qDebug("%7.2f %7.2f %7.2f %7.2f",a,b,c,d);
      }
    }
    if (t=="\".s\"") {
      if (!expectType("double3")) return false;
      if (b.attach==-1) globalScale = readPointRot();
    }
    skipLine();
  }
  if (!hasT) {lastErr=QString("No translation found for bone '%1'").arg(b.name); return false; };
  if (!hasR) {lastErr=QString("No rotation found for bone '%1'").arg(b.name); return false; };
  s.bone.push_back(b);
  int i = s.bone.size()-1;
  s.setRotationMatrix( euler2matrix(&(rot[0])) , i );
  s.BuildTree();
  vector<Matrix44f> v = s.GetBoneMatrices();
      qDebug("Versus:");
      Matrix44f m = v[i]; //s.getRotationMatrix(i);
      for (int i=0; i<4; i++) {
        qDebug("%7.2f %7.2f %7.2f %7.2f",m[0][i],m[1][i],m[2][i],m[3][i]);
      }
      //qDebug("T: %7.2f %7.2f  %7.2f ",b.t[0],b.t[1],b.t[2]);
      qDebug("-----------");
  qDebug()<< "[" << i << "]: abc" << rot[0]*180/M_PI << rot[1]*180/M_PI <<rot[2]*180/M_PI <<"\n";
  return true;
}


bool IoMB::Import(const wchar_t*filename, std::vector<BrfMesh> &m , BrfSkeleton &s, int want){
  debug=wfopen(L"debug.txt","w");

  lastErr = QString("Unkonwn error??");
  f = wfopen(filename,"rb"); lineN=0;
  if (!f) {
    lastErr =QString("cannot open file '%1' for reading").arg(QString::fromStdWString(filename));
    return false;
  }
  s.root= -1;
  s.bone.clear();

  while (nextCreateNode()) {
    QString t =  token();
    fprintf(debug,"[%s]\n",t.toAscii().data()); fflush(debug);
    if (t=="joint") {
      if (want ==1) if (!ioMB_importBone(s)) return false;
    }
    if (t=="mesh") {
      if (want ==0)  {
        BrfMesh m0;
        if (!ioMB_importMesh(m0)) return false;
        if (m0.face.size()>0) m.push_back(m0);
      }
    }
    if (t=="skinCluster"){
      //qDebug("rigging");

      if (want==0) {
        int n = ioMB_importRiggingSize();
        // find a mesh
        int found=-1;
        for (int i=0;i<(int)m.size();i++) if ((int)m[i].frame[0].pos.size()==n) found=i;

        if (found>=0) ioMB_importRigging(m[found]);
        else qDebug("cannot find any mesh with %d vert",n);
      }
    }
    skipCreate();
  }
  if (want==1) {
    if (!s.bone.size()) {
      lastErr = QString("No skeleton found (not a single bone).");
      return false;
    }
    s.BuildTree();
  }
  if (want==0) {
    if (m.size()==0) {
      lastErr = QString("No mesh found.");
      return false;
    }
  }
  //qDebug("Done");
  return true;
}

char* IoMB::LastErrorString(){
  return lastErr.toAscii().data();
}

