/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <set>
#include <stdio.h>

#include <vcg/space/box3.h>
#include <vcg/math/matrix44.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>

#include <vector>

using namespace vcg;
using namespace std;
#include "brfMesh.h"
#include "ioMD3.h"
#include "saveLoad.h"
#include "platform.h"

#define ERRSIZE 512
static wchar_t errorStr[ERRSIZE];
static const unsigned int MAGIC_MD3 = 0x33504449;
static const unsigned int MAGIC_MD2 = 0x32504449; // "IDP2"


typedef unsigned int uint;
typedef unsigned char Byte;

bool LoadStringFix(FILE* f,char *res, int max){
  if (fread(res,max,1,f) != 1) throw std::runtime_error("Read in LoadStringFix() failed!");
  return true;
}

void SaveStringFix(FILE* f,const char *res, int max){
  bool inside = true;
  for (int i=0; i<max-1; i++){
    if (inside) SaveByte(f,res[i]); else
    SaveByte(f,0);
    if (res[i]==0) inside = false;
  }
  SaveByte(f,0);
}

static const float RATIO = 100.0;

void SavePoint16(FILE *f, vcg::Point3f p) {
  SaveShort(f,  int(p.X()*RATIO*64) );
  SaveShort(f,  int(p.Z()*RATIO*64) );
  SaveShort(f,  int(p.Y()*RATIO*64) );
}

void LoadPoint16(FILE *f, vcg::Point3f &p) {
  short int x,y,z;
  LoadShort(f,x);
  LoadShort(f,z);
  LoadShort(f,y);
  p = vcg::Point3f(
    x/(RATIO*64),
    y/(RATIO*64),
    z/(RATIO*64)
  );
}

static void tryExtractNumber(char* st, int &res){
  while (!(*st>='0' && *st<='9')) {
    if (!st) return;
    st++;
  }
  sscanf(st,"%d",&res);
}

wchar_t* IoMD::LastErrorString(){
  return errorStr;
}

static vcg::Point3f int2norm(Byte zen,Byte azi){
  double zend = zen * (2 * M_PI ) / 255.0;
  double azid = azi * (2 * M_PI ) / 255.0;
  vcg::Point3f res;
  res.X() =  (float)(cos ( zend ) * sin ( azid ));
  res.Z() =  (float)(sin ( zend ) * sin ( azid ));
  res.Y() =  (float)(cos ( azid ));
  return res;
}

static void norm2int(vcg::Point3f  res, Byte &zenb,Byte &azib){

  double zen = atan2(res.Z(), res.X() );
  double azi = acos(res.Y() );

  int zeni = int( zen / (2 * M_PI ) * 255.0);
  int azii = int( azi / (2 * M_PI ) * 255.0);
  if (zeni>255) zeni -= 255;
  if (zeni<0)   zeni += 255;
  if (azii>255) azii -= 255;
  if (azii<0)   azii += 255;

  zenb = zeni;
  azib = azii;
}


bool IoMD::ExportMD2(const wchar_t *filename, const BrfMesh &m){
    FILE *f = wfopen(filename,"wb");
    if (!f){
        swprintf(errorStr,ERRSIZE,L"Cannot write on file:\n %ls",filename);
        return false;
    }
    SaveUint(f,MAGIC_MD2);
    SaveInt(f,8); // version
    SaveInt(f,1024); // text width
    SaveInt(f,1024); // text width
    SaveInt(f, m.frame.size() );

    int nframes = m.frame.size();

    SaveInt(f, 1 );      // number of textures
    SaveInt(f, m.frame[0].pos.size() * nframes ); // total n of xyz
    SaveInt(f, m.vert.size() ); //
    SaveInt(f, m.face.size() );
    SaveInt(f, 0 ); // openGL commands?
    SaveInt(f,  nframes );

    /*
    int     ofs_skins;          // offset to skin names (64 bytes each)
    int     ofs_st;             // offset to s-t texture coordinates
    int     ofs_tris;           // offset to triangles
    int     ofs_frames;         // offset to frame data
    int     ofs_glcmds;         // offset to opengl commands
    int     ofs_end;            // offset to end of file
    */

    unsigned int ofs_pos = ftell( f ); //

    SaveUint( f, ofs_pos );
    //swprintf(errorStr,ERRSIZE,"TEST1 %d == %d\n",ftell(f),64+11*4);

    // frames
    for (uint i=0; i<m.frame.size(); i++){
        SavePoint(f,m.bbox.min*RATIO); // 4*3
        SavePoint(f,m.bbox.max*RATIO); // 4*3
        SavePoint(f,vcg::Point3f(0,0,0)); // 4*3
        SaveFloat(f,m.bbox.Diag()*RATIO); // 4
        char tmp[255];
        sprintf(tmp, "T%d", m.frame[i].time);
        SaveStringFix(f,tmp,16);  // 16
    }


    fclose(f);
    return true;
}



bool IoMD::Export(const wchar_t *filename, const BrfMesh &m){
  if (m.frame.size()>1024){
    swprintf(errorStr,ERRSIZE,L"Too many frames %d. Max = 1024",m.frame.size());
    return false;
  }
  if (m.vert.size()>4096){
    swprintf(errorStr,ERRSIZE,L"Too many vertices %d. Max = 4096",m.vert.size());
    return false;
  }
  if (m.face.size()>8192){
    swprintf(errorStr,ERRSIZE,L"Too many faces: %d. Max = 8192",m.face.size());
    return false;
  }
  FILE *f = wfopen(filename,"wb");

  if (!f){
    swprintf(errorStr,ERRSIZE,L"Cannot write on file:\n %ls",filename);
    return false;
  }
  SaveUint(f,MAGIC_MD3);
  SaveInt(f,15); // version
  SaveStringFix(f,m.name,64);
  SaveInt(f,0); // flags
  SaveUint(f,m.frame.size());
  SaveInt(f,0); // tags
  SaveUint(f,1);  // surfaces
  SaveUint(f,0);  // skins
  long int off = 64+11*4;
  SaveUint(f,off); // framwa
  off+= m.frame.size()*(4*3 + 4*3 + 4*3 + 4 + 16);
  SaveUint(f,off); // tags
  SaveUint(f,off); // surfaces
  off += 10000 ;
  SaveUint(f,off); // oef

  //swprintf(errorStr,ERRSIZE,"TEST1 %d == %d\n",ftell(f),64+11*4);

  // frames
  for (uint i=0; i<m.frame.size(); i++){
    SavePoint(f,m.bbox.min*RATIO); // 4*3
    SavePoint(f,m.bbox.max*RATIO); // 4*3
    SavePoint(f,vcg::Point3f(0,0,0)); // 4*3
    SaveFloat(f,m.bbox.Diag()*RATIO); // 4
    char tmp[255];
    sprintf(tmp, "T%d", m.frame[i].time);
    SaveStringFix(f,tmp,16);  // 16
  }

  // surface
  //sprintf(errorStr,"%sTEST %d == %d",errorStr,ftell(f),off-10000);
  //return false;
  SaveUint(f,MAGIC_MD3); // 4
  SaveStringFix(f,m.name,64); //64
  SaveInt(f,0); // flags  // 4
  SaveUint(f,m.frame.size());  // 4
  SaveUint(f,1); // shaders  4
  SaveUint(f,m.vert.size()); // 4
  SaveUint(f,m.face.size()); // 4

  off = 64 + 11*4 + (64 + 4);
  SaveUint(f,off ); // offset tri  4
  SaveUint(f,64 + 11*4 ); // offset shader 4
  off += m.face.size()*3*4; // three int per face
  SaveUint(f,off ); // offset st 4
  off += m.vert.size()*2*4;
  SaveUint(f,off ); // offset xyznorm 4
  off += m.vert.size()*4*2*m.frame.size();
  SaveUint(f,off ); // offset endf 4

  // surface shader
  SaveStringFix(f,m.material,64);
  SaveInt(f,0); // index

  // surface triangles
  for (uint i=0; i<m.face.size(); i++){
    for (int j=0; j<3; j++){
      SaveInt(f,m.face[i].index[2-j]);
    }
  }

  // surface st
  for (uint i=0; i<m.vert.size(); i++){
    SavePoint(f,m.vert[i].ta);
  }

  // surface xyzn
  for (uint j=0; j<m.frame.size(); j++)
  for (uint i=0; i<m.vert.size(); i++){
    SavePoint16(f,m.frame[j].pos[ m.vert[i].index ]);
    Byte n0,n1;
    norm2int(m.frame[j].norm[ i ],n0,n1);
    SaveByte(f,n0);
    SaveByte(f,n1);
  }

  fclose(f);
  return true;
}


bool IoMD::Import(FILE *f, BrfMesh &m){
  long pos = ftell(f);



  unsigned int magic=0;
  LoadUint(f,magic);
  if (magic != MAGIC_MD3) {
    swprintf(errorStr,ERRSIZE,L"Invalid magic number in surface: %X",magic);
    return false;
  }

  char str[65];
  LoadStringFix(f,str,64);
  sprintf(m.name,"%s",str);
  sprintf(m.material,"%s",str);

  unsigned int flags;
  LoadUint(f,flags);
  unsigned int nframes,nshaders,nverts,ntriangles;
  unsigned int otriangles,oshaders,ost,oxyz,oend;
  LoadUint(f,nframes);
  LoadUint(f,nshaders);
  LoadUint(f,nverts);
  LoadUint(f,ntriangles);
  LoadUint(f,otriangles);
  LoadUint(f,oshaders);
  LoadUint(f,ost);
  LoadUint(f,oxyz);
  LoadUint(f,oend);

  swprintf(errorStr,ERRSIZE,L"Loaded: %dv %dt %df\n",nverts,ntriangles,nframes);

  m.face.resize(ntriangles);
  m.vert.resize(nverts);
  m.frame.resize(nframes);

  for (unsigned int i=0; i<nframes; i++) {
    m.frame[i].pos.resize(nverts);
    m.frame[i].norm.resize(nverts);
    m.frame[i].time = (i<2)?i:99+(i-2)*5; // default timings for icon animations

  }


  // load triangles
  fseek(f,pos+otriangles,SEEK_SET);
  for (unsigned int i=0; i<ntriangles; i++)
  for (int j=0; j<3; j++) LoadInt(f,m.face[i].index[2-j]);

  // load shader into material
  fseek(f,pos+oshaders,SEEK_SET);
  LoadStringFix(f,m.material,32);

  // load text coords
  fseek(f,pos+ost,SEEK_SET);
  for (unsigned int j=0; j<nverts; j++) {
    LoadPoint(f,m.vert[j].ta);
    m.vert[j].tb = m.vert[j].ta;
	m.vert[j].tangi = 0;
    m.vert[j].index = j;
  }

  // load XYZ + norms
  fseek(f,pos+oxyz,SEEK_SET);
  for (unsigned int i=0; i<nframes; i++)
  for (unsigned int j=0; j<nverts; j++) {
    LoadPoint16(f,m.frame[i].pos[j]);
    Byte n0,n1;
    LoadByte(f,n0); // norm
    LoadByte(f,n1); // norm
    m.frame[i].norm[j] = int2norm(n0,n1);
  }

  m.ColorAll(0xFFFFFFFF);
  //m.ComputeNormals();
  m.AdjustNormDuplicates();
  m.UpdateBBox();
  m.skinning.clear();
  m.flags = 0;



  fseek(f,pos+oend,SEEK_SET);
  return true;
}

bool IoMD::Import(const wchar_t *filename, std::vector<BrfMesh> &mv){

  FILE *f = wfopen(filename,"rb");
  if (!f) {
    swprintf(errorStr,ERRSIZE,L"File not found");
    return false;
  }
  unsigned int magic=0,ver=0;
  LoadUint(f,magic);
  if (magic != MAGIC_MD3) {
    swprintf(errorStr,ERRSIZE,L"Invalid magic number: %X",magic);
    return false;
  }

  LoadUint(f,ver);
  char name[64];
  LoadStringFix(f,name,64);
  unsigned int nframes,ntags,nsurfaces,nskins;
  unsigned int oframes,otags,osurfaces,flags;
  LoadUint(f,flags);
  LoadUint(f,nframes);
  LoadUint(f,ntags);
  LoadUint(f,nsurfaces);
  LoadUint(f,nskins);
  LoadUint(f,oframes);
  LoadUint(f,otags);
  LoadUint(f,osurfaces);

  //sprintf(errorStr,"Found: %s (%d surface %d frames, off: %0X)",name,nsurfaces,nframes,osurfaces);

  if (nsurfaces>255 ) {
    swprintf(errorStr,ERRSIZE,L"File format error (%d surfaces?)",nsurfaces);
    return false;
  }

  if (nsurfaces==0 ) {
    swprintf(errorStr,ERRSIZE,L"no \"surface\" found");
    return false;
  }
  fseek(f,osurfaces,SEEK_SET);
  for (unsigned int i=0; i<nsurfaces; i++) {
    BrfMesh m;
    if (!Import(f,m)) return false;
    mv.push_back(m);
  }

  fseek(f,oframes,SEEK_SET);
  for (unsigned int i=0; i<nframes; i++) {
    Skip(f,10*4); // skip bounding box and stuff
    char tmp[17];
    LoadStringFix(f,tmp,16);
    for (unsigned int j=0; j<nsurfaces; j++){
      tryExtractNumber(tmp,mv[j].frame[i].time);
      //sprintf(mv[j].name,tmp);
    }
  }


  fclose(f);
  return true;

  //fseek("")
}
