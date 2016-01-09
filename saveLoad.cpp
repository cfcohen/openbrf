/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <strings>
#include <vector>
#include <stdexcept>

#include <vcg/space/point4.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>

using namespace vcg;

void SaveString(FILE *f, const char *st){
  int x=strlen(st);
  fwrite(&x, 4, 1,  f);
  fwrite(st, 1, x, f);
}

void SaveStringNotempty(FILE *f, const char *st, const char *ifnot){
  if (st[0]==0) SaveString(f,ifnot);
  else SaveString(f,st);
}

void SaveInt(FILE *f, int x){
  fwrite(&x, 4, 1,  f);
}

void SaveByte(FILE *f, unsigned char x){
  fwrite(&x, 1, 1,  f);
}


void SaveUint(FILE *f, unsigned int x){
  fwrite(&x, 4, 1,  f);
}

void SaveShort(FILE *f, short int x){
  fwrite(&x, 2, 1,  f);
}

void SaveFloat(FILE *f, float x){
  fwrite(&x, 4, 1,  f);
}

void SavePoint(FILE *f, vcg::Point3f p) {
  SaveFloat(f,  p.X() ); 
  SaveFloat(f,  p.Z() );
  SaveFloat(f,  p.Y() ); 
}

void SavePoint(FILE *f, vcg::Point2f p) {
  SaveFloat(f,  p.X() ); 
  SaveFloat(f,  p.Y() ); 
}

void SavePoint(FILE *f, vcg::Point4f p) {
  SaveFloat(f,  p.Y() );
  SaveFloat(f,  p.W() );
  SaveFloat(f,  p.Z() );
  SaveFloat(f,  p.X() );
}


void SaveVector(FILE *f,const std::vector<int> & v){
  SaveUint(f,v.size());
  for (unsigned int i=0; i<v.size(); i++) SaveInt(f,v[i]);
}

void SaveVector(FILE *f,const std::vector<Point3f> &v){
  SaveUint(f,v.size());
  for (unsigned int i=0; i<v.size(); i++) SavePoint(f,v[i]);
}


bool LoadStringMaybe(FILE *f, char *st, const char *ifnot){
  unsigned int x;
  if (fread(&x, 4, 1,  f) != 1) throw std::runtime_error("Read 1 in LoadStringMaybe() failed.");
  if (x<99 && x>0) {
    if (fread(st, 1, x, f) != x) throw std::runtime_error("Read 2 in LoadStringMaybe() failed.");
    st[x]=0;
    return true;
  } else {
    fseek(f,-4,SEEK_CUR);
    sprintf(st,"%s",ifnot);
    return false;
  }

}

bool LoadString(FILE *f, char *st){
  unsigned int x;
  if (fread(&x, 4, 1,  f) != 1) throw std::runtime_error("Read 1 in LoadString() failed.");
  if (x>=256) return false;

  if (fread(st, 1, x, f) != x) throw std::runtime_error("Read 2 in LoadString() failed.");
  st[x]=0;
  
 //printf("\"%s\"...\n",st);
  return true;
}

void LoadInt(FILE *f, int &i){
  if (fread(&i, 4, 1,  f) != 1) throw std::runtime_error("Read in LoadInt() failed.");
  //printf("%d ",i);
}


void LoadByte(FILE *f, unsigned char &i){
  if (fread(&i, 1, 1,  f) != 1) throw std::runtime_error("Read in LoadByte() failed.");
  //printf("%d ",i);
}

void LoadFloat(FILE *f, float &x){
  if (fread(&x, 4, 1,  f) != 1) throw std::runtime_error("Read in LoadFloat() failed.");
}

void LoadUint(FILE *f, unsigned int &x){
  if (fread(&x, 4, 1,  f) != 1) throw std::runtime_error("Read in LoadUint() failed.");
  //printf("%ud ",x);
}

void LoadShort(FILE *f, short int &x){
  if (fread(&x, 2, 1,  f) != 1) throw std::runtime_error("Read in LoadShort() failed.");
  //printf("%ud ",x);
}

void LoadPoint(FILE *f, vcg::Point3f &p){
  LoadFloat(f,  p.X() ); 
  LoadFloat(f,  p.Z() );
  LoadFloat(f,  p.Y() ); 
  
  //printf("(%f,%f,%f) ",p[0],p[1],p[2]);
}

void LoadPoint(FILE *f, vcg::Point4f &p){
  LoadFloat(f,  p.Y() );
  LoadFloat(f,  p.W() );
  LoadFloat(f,  p.Z() );
  LoadFloat(f,  p.X() );
}


void LoadPoint(FILE *f, vcg::Point2f &p){
  LoadFloat(f,  p.X() ); 
  LoadFloat(f,  p.Y() ); 
  //printf("(%f,%f) ",p[0],p[1]);
}

bool LoadVector(FILE *f,std::vector<int> &v){
  unsigned int k;
  LoadUint(f,k);
  v.resize(k);
  for (unsigned int i=0; i<v.size(); i++) LoadInt(f,v[i]);
  return true;
}

bool LoadVector(FILE *f,std::vector<Point3f> &v){
  unsigned int k;
  LoadUint(f,k);
  v.resize(k);
  for (unsigned int i=0; i<v.size(); i++) LoadPoint(f,v[i]);
  return true;
}

template<class T> bool LoadVector(FILE *f,std::vector< std::vector<T> > &v){
  unsigned int k;
  LoadUint(f,k);
  v.resize(k);
  for (unsigned int i=0; i<v.size(); i++) if (!LoadVector(f,v[i])) return false;
  return true;
}

template<class T> bool LoadVector(FILE *f,std::vector<T, std::allocator<T> > &v){
  unsigned int k;
  LoadUint(f,k);
  v.resize(k);
  for (unsigned int i=0; i<v.size(); i++) if (!v[i].Load(f)) return false;
  return true;
}


void Skip(FILE *f, int k){
  fseek(f,k, SEEK_CUR);
}

void SkipString(FILE *f){
  int k;
  LoadInt(f,k);
  fseek(f,k, SEEK_CUR);
}




