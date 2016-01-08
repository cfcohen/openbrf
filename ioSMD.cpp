/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "brfData.h"

#include "ioSMD.h"
//#include "qdebug.h"
#include "vcg/math/quaternion.h"

// everything is scaled up when expoerted, down when imported...
static const float SCALE = 10.0f;

// used to export/import skeletons, rigged meshes, animations...
// return 0 on success. +... on error. -... on warnings

static float Norm(const Matrix44f &m){
  float res=0;
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++) res+=m.ElementAt(i,j)*m.ElementAt(i,j);
  return res;
}


Matrix44f euler2matrix(float* eul){
  Matrix44f m;
  m.FromEulerAngles(eul[0], eul[1], eul[2]);
  float f[16]={1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1};
  Matrix44f inv(f);
  m = inv*m*inv;
  return m;
}

float* matrix2euler(const Matrix44f &_m){
  static float res[3];
  float f[16]={1,0,0,0, 0,0,1,0, 0,1,0,0, 0,0,0,1};
  Matrix44f inv(f);
  Matrix44f m = _m;
  m = inv*m.transpose()*inv;
  m.ToEulerAngles(res[0], res[1], res[2]);
	if (res[1]!=res[1]) res[1]=-M_PI/2;
  if (fabs(fabs(res[1])-M_PI/2)<0.000001) {
    // ouch: pivot angle... let's try everythin...
    float best = 1000;
    static float win[3];
    for (int k=-1; k<=1; k+=2)
    for (int i=0; i<4; i++)

    for (int j=0; j<4; j++) {
        res[0]=(i-2)*M_PI/2;
        res[1]=(k)*M_PI/2;
        res[2]=(j-2)*M_PI/2;
        Matrix44f m2 = euler2matrix(res);
        float score =Norm(m2-_m.transpose());
        if (score<best) {
          win[0]=res[0];win[1]=res[1];win[2]=res[2];
          best = score;
        }
    }
    return win;
  }
  return res;

  /*static vcg::Quaternionf q;
  q.FromMatrix(m);
  q.ToEulerAngles( res[0], res[1], res[2]);*/
  return res;
}


static int lastErr;
static int nMaxBones;
const char *expectedErr, *foundErr;
int versionErr;

static bool expect(FILE* f, const char* what){
  static char str[255];
  fscanf(f, "%s", str);
  if (strcmp(str,what)){
    expectedErr = what;
    foundErr = str;
    lastErr = 4;
    return false;
  }
  return true;
}

static bool expectLine(FILE* f, const char* what){
  static char str[255];
  fscanf(f, "%s\n", str);
  if (strcmp(str,what)){
    expectedErr = what;
    foundErr = str;
    lastErr = 4;
    return false;
  }
  return true;
}

static bool fscanln(FILE*f, char *ln){
  int i=0;
  while (1) {
    fread(&ln[i],1,1,f);
    if (ln[i]=='\n') { ln[i]=0; return true;}
    i++;
  }
}

// defined in vcgmesh.cpp
void setGotNormals(bool b);
void setGotMaterialName(bool b);
void setGotTexture(bool b);
void setGotColor(bool b);

static bool ioSMD_ImportTriangles(FILE*f, BrfMesh &m ){

	setGotNormals(true);
	setGotMaterialName(true);
	setGotTexture(true);
	setGotColor(false);

  int pi=0;
  m.frame.resize(1);
  m.frame[0].time=0;

  if (!expectLine(f,"triangles")) return false;

  while (1){
    char matName[4096];
    fscanln(f, matName); //
    //fscanf(f,"%s\n", matName);
    if (strcmp(matName,"end")==0) break;
    sprintf(m.material,"%s",matName);
    for (int w=0; w<3; w++) {
      int bi;
      Point3f p;
      Point3f n;
      Point2f t;
      char line[4096];
      BrfRigging r;
      int nr=0;
      fscanln(f, line);
      int nread =
      sscanf(line,"%d %f %f %f %f %f %f %f %f %d %d %f %d %f %d %f %d %f",
        &bi,
        &(p[0]),&(p[2]),&(p[1]),
        &(n[0]),&(n[2]),&(n[1]),
        &(t[0]),&(t[1]),
        &nr,
        &(r.boneIndex[0]), &(r.boneWeight[0]),
        &(r.boneIndex[1]), &(r.boneWeight[1]),
        &(r.boneIndex[2]), &(r.boneWeight[2]),
        &(r.boneIndex[3]), &(r.boneWeight[3])
      );
      p/=SCALE;
      if (nr>nMaxBones ) nMaxBones = nr;
      if (nr>4) { nr=4;}
      //if  (!( nread==9 || nread == 9+1+nr*2)) qDebug("[%s] (w:%d f:%d),",line,w,m.face.size());
      assert( nread==9 || nread == 9+1+nr*2);
      for (int k = nr; k<4; k++) {
        r.boneIndex[k]=-1; r.boneWeight[k]=0;
      }
      float sumW = 0;
      for (int k = 0; k<4; k++) sumW += r.boneWeight[k];
      if (sumW<0.999999) {
        if (nr<4) {
          r.boneIndex[nr] = bi;
          r.boneWeight[nr] = 1-sumW;
        }
      }

      t[1]=1-t[1];
      BrfVert v;
      v.index=pi;
      v.ta = v.tb = t;
      v.__norm = n;
      v.col=0xffffffff; // white

      m.vert.push_back(v);
      m.frame[0].pos.push_back(p);
      m.frame[0].norm.push_back(n);

      m.rigging.push_back(r);
      pi++;
    }
    m.face.push_back( BrfFace( pi-3, pi-2, pi-1 ) );
  }
  return true;
}

static void ioSMD_ExportTriangles(FILE*f,const BrfMesh &m , int fi){
  fprintf(f,"triangles\n");
  assert(m.rigging.size()==m.frame[fi].pos.size());
  for (unsigned int i=0; i<m.face.size(); i++){
    if (m.material[0]==0)
      fprintf(f,"%s\n","material_name");
    else
      fprintf(f,"%s\n",m.material);
    for (int w=0; w<3; w++){
      int vi = m.face[i].index[w]; // vertex index
      int pi = m.vert[vi].index; // position index
      fprintf(f," %d %f %f %f %f %f %f %f %f",
        m.rigging[pi].boneIndex[0],
        m.frame[fi].pos[pi][0]*SCALE,
        m.frame[fi].pos[pi][2]*SCALE,
        m.frame[fi].pos[pi][1]*SCALE,
        m.frame[fi].norm[vi][0],
        m.frame[fi].norm[vi][2],
        m.frame[fi].norm[vi][1],
        m.vert[vi].ta[0],
        1.0f-m.vert[vi].ta[1]
      );
      int nrig=0;
      for (int j=0; j<4; j++) if (m.rigging[pi].boneIndex[j]!=-1) nrig++;
      if (nrig>0) {
        fprintf(f," %d",nrig); // number of links except 1st one
        for (int j=0; j<nrig; j++) {
          fprintf(f," %d %f", m.rigging[pi].boneIndex[j], m.rigging[pi].boneWeight[j] );
        }
      }
      fprintf(f," \n");

    }
  }

  fprintf(f,"end\n");
}

static bool ioSMD_ImportBoneStruct(FILE*f,BrfSkeleton &s ){
  int v=-1;
  if (!expect(f,"version")) return false;
  fscanf(f, "%d\n",&v);
  if (!expectLine(f,"nodes")) return false;
  if (v!=1) { versionErr = v; lastErr=3; return false;}
  bool rootFound = false;
  while (1) {
    int a, b;
    char line[4096];
    char st[4096];
    fscanln(f,line);
    int res = sscanf(line,"%d \"%s %d",&a, st, &b);
    if (res<3) {
      assert(strcmp(line,"end")==0);
      break;
    }
    // remove ending '"'
    assert(st[strlen(st)-1]=='"');
    st[strlen(st)-1]=0;

    if (b==-1) {// here is a root
      if (rootFound) continue; // ignore extra roots;
      rootFound=true;
    }
    //qDebug("bone a=%d, b=%d",a,b);
    if (a>=(int)s.bone.size()) s.bone.resize(a+1);
    //qDebug("size %d, a=%d",s.bone.size(),a);
    s.bone[a].attach=b;
    sprintf(s.bone[a].name,"%s",st);
  }
  s.BuildTree();
  return true;
}

static void ioSMD_ExportBoneStruct(FILE*f,const BrfSkeleton &s ){
  fprintf(f,"version 1\n"); // header

  fprintf(f,"nodes\n");
  for (unsigned int i=0; i<s.bone.size(); i++)
    fprintf(f,"%d \"%s\" %d\n", i, s.bone[i].name, s.bone[i].attach);
  fprintf(f,"end\n");
}

template <class T>
static void ioSMD_ExportPose(FILE* f, const BrfSkeleton &s,  const T& pose, int time){
  fprintf(f,"time %d\n",time);
  for (unsigned int i=0; i<s.bone.size(); i++) {
    Matrix44f ma,mb,mc;
    float* euler=matrix2euler( ma=pose.getRotationMatrix(i).transpose() );
    /*
      tests:
    float e0=euler[0];
    float e1=euler[1];
    float e2=euler[2];
    mb = euler2matrix(euler);
    euler=matrix2euler( mb );
    float b0=euler[0];
    float b1=euler[1];
    float b2=euler[2];
    mc = euler2matrix(euler);*/
    fprintf(f," %d %f %f %f %f %f %f \n",
            i,
            s.bone[i].t[0]*SCALE, s.bone[i].t[2]*SCALE, s.bone[i].t[1]*SCALE,
             euler[0],  euler[1],  euler[2]);
  }
}

template <class T>
static bool ioSMD_ImportPose(FILE* f, BrfSkeleton &s,  T& pose, int &time){
  if (!expect(f,"time")) return false;

  fscanf(f,"%d",&time);
  //for (static int i=0;i<s.bone.size(); i++)
  //  s.bone[i].
  while (1) {
    int i;
    int res = fscanf(f,"%d",&i);
    if (res==0) break; // opefully it is an "end"
    //assert(i<(int)s.bone.size());
    float r[3];
    vcg::Point3f t;
    fscanf(f,"%f %f %f %f %f %f", &(t[0]),&(t[2]),&(t[1]), r+0, r+1, r+2);
    if (i>=(int)s.bone.size()) continue; // ignore rotation for non-existing bones
    s.bone[i].t = t/SCALE;
    pose.setRotationMatrix( euler2matrix(r) , i );
  }
  return true;
}

int ioSMD::Export(const wchar_t*filename, const BrfMesh &m , const BrfSkeleton &s, int fi){
  FILE* f=_wfopen(filename,L"wb");
  lastErr = 0;
  if (!f) return(lastErr=2);

  ioSMD_ExportBoneStruct(f,s);

  fprintf(f,"skeleton\n");
  ioSMD_ExportPose(f,s,s,0); // one pose: skeleton pose
  fprintf(f,"end\n");
  ioSMD_ExportTriangles(f,m,fi);
  fclose(f);

  return lastErr;
}


int ioSMD::Export(const wchar_t*filename, const BrfAnimation &a, const BrfSkeleton &s){

  FILE* f=_wfopen(filename,L"wb");
  lastErr = 0;
  if (!f) return(lastErr=2);

  ioSMD_ExportBoneStruct(f,s);


  fprintf(f,"skeleton\n");
  ioSMD_ExportPose(f,s,s,0); // one pose: skeleton pose
  for (unsigned int i=0; i<a.frame.size(); i++) {
    BrfSkeleton s0 = s;
    s0.bone[ s0.root ].t+=a.frame[i].tra;

    ioSMD_ExportPose(f,s0,a.frame[i], a.frame[i].index+1); // other poses (at index+1, to leave 0 for basic pose)
  }
  fprintf(f,"end\n");

  fclose(f);

  return lastErr;
}


int ioSMD::Import(const wchar_t*filename, BrfMesh &m , BrfSkeleton &s){
  lastErr = 0;
  nMaxBones = 0;
  FILE* f=_wfopen(filename,L"rt");
  if (!f) return(lastErr=1);

  if (!ioSMD_ImportBoneStruct(f,s)) return lastErr;

  if (!expect(f,"skeleton")) return false;
  int time;
  if (!ioSMD_ImportPose(f,s,s,time)) return lastErr;
  if (!expect(f,"end")) return false;

  if (!ioSMD_ImportTriangles(f,m)) return false;

  //m.UnifyPos();
  //m.UnifyVert(false);
  m.AfterLoad();
  m.flags=0;
  m.maxBone=s.bone.size();

  fclose(f);
  return lastErr;
}

int ioSMD::Import(const wchar_t*filename, BrfAnimation &a, BrfSkeleton &s){

  lastErr = 0;
  FILE* f=_wfopen(filename,L"rt");
  if (!f) return(lastErr=1);

  if (!ioSMD_ImportBoneStruct(f,s)) return lastErr;

  if (!expect(f,"skeleton")) return false;

  int last;
  if (!ioSMD_ImportPose(f,s,s,last)) return lastErr; // initial pose


  a.nbones = s.bone.size();
  BrfAnimationFrame af;
  af.rot.resize(a.nbones);
  af.tra = vcg::Point3f(0,0,0);
  while (1) {
    BrfSkeleton s0 = s;
    if (!ioSMD_ImportPose(f,s0,af,af.index)) break;
    if (af.index<=last) af.index=last+1; last = af.index; // enforce increasing order

    af.wasImplicit.resize(af.rot.size()+1, false);
    af.tra = s0.bone[ s.root ].t - s.bone[ s.root ].t;
    af.index--; // first frame was just a reference...
    a.frame.push_back(af);
  }

  expectedErr="end";
  if (strcmp(foundErr,expectedErr)==0) lastErr=0; else return false;

  fclose(f);
  return lastErr;
}


const char* ioSMD::LastErrorString(){
  static char res[255];
  switch(lastErr) {
  case 1: return "File not found"; break;
  case 2: return "Cannot open file for writing"; break;
  case 3: sprintf(res,"Version %d not supported",versionErr); return res; break;
  case 4: sprintf(res,"Expected '%s' found '%s'",expectedErr, foundErr); return res; break;
  case 0: return "(no error)"; break;
  default: return "undocumented error"; break;
  }
}

bool ioSMD::Warning(){
  return nMaxBones>4;
}

char* ioSMD::LastWarningString(){
  static char res[255];
  sprintf(res,
    "WARNING: found vertices rigged to %d bones.\n"
    "In M&B, limit is 4.",nMaxBones);
  return res;
}

