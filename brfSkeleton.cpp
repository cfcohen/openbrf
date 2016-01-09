/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <vcg/math/matrix44.h>
#include <vcg/math/quaternion.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
#include <vcg/space/box3.h>
using namespace vcg;

#include "brfSkeleton.h"
#include "brfAnimation.h"
using namespace std;
#include "brfMesh.h"
#include "brfBody.h"

#include "saveLoad.h"
#include "platform.h"

static float values[16] = {
  0, 0,  1, 0,
  0, -1, 0, 0,
   1, 0, 0, 0,
  0, 0, 0, 1,
};

static vcg::Matrix44f matr(values);

int BrfSkeleton::FindBoneByName(const char * name) const{
  for (unsigned int i=0; i<bone.size(); i++){
    if (strcmp(bone[i].name,name)==0) return i;
  }
  return -1; // not found
}

// for translations and points:
vcg::Point3f BrfSkeleton::adjustCoordSyst(vcg::Point3f p){
  return matr*p;
}

// for rotations (full):
vcg::Point4f   BrfSkeleton::adjustCoordSyst(vcg::Point4f p){
  //generic:
  //vcg::Quaternionf q,r; q.FromMatrix(matr);
  //return r=q*vcg::Quaternionf(p)*q;
  return vcg::Point4f(p[0],p[3],-p[2],p[1]);
}
BrfBone        BrfSkeleton::adjustCoordSyst(BrfBone p) {
  BrfBone res=p;
  /*res.setRotationMatrix( adjustCoordSyst(res.getRotationMatrix()) );
  res.t = adjustCoordSyst( res.t );*/
  res.x =  adjustCoordSyst( p.z );
  res.y = -adjustCoordSyst( p.y );
  res.z =  adjustCoordSyst( p.x );
  res.t =  adjustCoordSyst( res.t );
  return  res;
}
vcg::Matrix44f BrfSkeleton::adjustCoordSyst(vcg::Matrix44f m){
  return matr*m*matr;
}

  // for rotations (half):
vcg::Point4f   BrfSkeleton::adjustCoordSystHalf(vcg::Point4f p){
  //generic: vcg::Quaternionf q; q.FromMatrix(matr);
  static vcg::Quaternionf q(0, float(1.0/sqrt(2.0)),0, float(1.0/sqrt(2.0)));
  return vcg::Quaternionf(p)*q;
}
BrfBone   BrfSkeleton::adjustCoordSystHalf(BrfBone p){
  BrfBone res=p;
  res.x =  p.z;
  res.y =  -p.y;
  res.z =  p.x;
  //res.setRotationMatrix( adjustCoordSystHalf(res.getRotationMatrix()) );
  return  res;
}
vcg::Matrix44f BrfSkeleton::adjustCoordSystHalf(vcg::Matrix44f m){
  return matr*m;
}

float BrfSkeleton::BoneSizeX(){return 0.12;}
float BrfSkeleton::BoneSizeY(){return 0.06;}
float BrfSkeleton::BoneSizeZ(){return 0.04;}

std::vector<int> BrfSkeleton::Bone2BoneMap(const BrfSkeleton & s) const{
  std::vector<int> res(bone.size(),-1);
  for (int i=0; i<(int)bone.size(); i++) {
    res[i] = s.FindBoneByName( bone[i].name );
  }
  return res;
}
std::vector<vcg::Point4<float> > BrfSkeleton::BoneRotations() const{

  std::vector< vcg::Point4<float> > res(bone.size());
  for (int i=0; i<(int)bone.size(); i++) {
    vcg::Quaternionf q;
    q.FromMatrix(getRotationMatrix( i ).transpose());
    res[i] = q;
  }
  return res;
}


int BrfSkeleton::FindSpecularBoneOf(int i) const{
	char boneName[255];
	sprintf(boneName, "%s", bone[i].name );
	int l = strlen(boneName)-1;
	if (l<1) return i;
	if (boneName[l-1]!='.') return i;
	if (boneName[l]=='L') boneName[l]='R';
	else if (boneName[l]=='R') boneName[l]='L';
	else if (boneName[l]=='l') boneName[l]='r';
	else if (boneName[l]=='r') boneName[l]='l';
	else return i;
	return FindBoneByName(boneName);
}

void BrfSkeleton::BuildDefaultMesh(BrfMesh & m) const{ // builds a mesh with just an octa x bone...
  int nb = bone.size();
  m.vert.resize(nb*6);
  m.frame.resize(1);
  m.frame[0].pos.resize(nb*6);
  m.frame[0].norm.resize(nb*6);
  m.face.resize(nb*8);
  m.rigging.resize(nb*6);

  float
    X=BrfSkeleton::BoneSizeX(),
    Y=BrfSkeleton::BoneSizeX(),
    Z=BrfSkeleton::BoneSizeX();
  vcg::Point3f pos[6]={
    vcg::Point3f(+X,0,0), //0
    vcg::Point3f(0,+Y,0), //1
    vcg::Point3f(0,0,+Z), //2
    vcg::Point3f(-X,0,0), //3
    vcg::Point3f(0,-Y,0), //4
    vcg::Point3f(0,0,-Z), //5
  };
  int facei[8][3] = {
    {0,1,2},{0,2,4},{0,4,5},{0,5,1},
    {3,1,5},{3,5,4},{3,4,2},{3,2,1},
  };

  std::vector<vcg::Matrix44f> mat = GetBoneMatrices();

  for (int i=0, pi=0, fi=0; i<nb; i++) {
    // set up rigging...
    for (int j=0; j<6; j++,pi++){
      m.rigging[pi].boneIndex[0]=i;
      m.rigging[pi].boneWeight[0]=1;
      for (int h=1; h<4; h++) {
        m.rigging[pi].boneIndex[h]=-1;
        m.rigging[pi].boneWeight[h]=0;
      }

      // set up pos and norm
      m.frame[0].pos[pi]=mat[i]*pos[j];
      m.frame[0].norm[pi]=mat[i]*(pos[j]/pos[j].Norm()) - mat[i]*Point3f(0,0,0);

      // set up uv coords
      m.vert[pi].index = pi;
      m.vert[pi].ta = m.vert[pi].tb = Point2f(0,0);
      m.vert[pi].col = 0xFFFFFFFF;

    }
    // set up face index
    for (int j=0; j<8; j++, fi++)
    for (int w=0; w<3; w++) {
      m.face[fi].index[w] = facei[j][2-w] + i*6;
    }
  }

  sprintf(m.name,"meshFromSkeleton");
  sprintf(m.material,"none");
  m.AdjustNormDuplicates();
  m.hasVertexColor=false;

}

BrfSkeleton::BrfSkeleton()
{
  float h=1;
  bbox.Add( vcg::Point3f(h,2*h,h));
  bbox.Add(-vcg::Point3f(h,0,h));
}

BrfBone::BrfBone()
{}


std::vector<Matrix44f>  BrfSkeleton::GetBoneMatrices(const BrfAnimationFrame &fr) const
{
  std::vector<Matrix44f> res;
  if (fr.rot.size()!=bone.size()) return res;
  //vcg::Matrix44 m; m.Set
  res.resize(fr.rot.size());
  Matrix44f first;
  first.SetTranslate( fr.tra );
  SetBoneMatrices(fr, root, res, first);

  std::vector<Matrix44f> tmp;
  tmp = GetBoneMatrices();

  for (unsigned int i=0; i<tmp.size(); i++) {
	tmp[i] = vcg::Inverse( tmp[i] );
    res[i]= res[i] * tmp[i] ;
    //res[i]= tmp[i] * res[i]  ;
  }
  return res;
}




void BrfSkeleton::SetBoneMatrices(const BrfAnimationFrame &fr, int bi,
                                  std::vector<Matrix44f> &output, const Matrix44f &curr) const
{
  Matrix44f tr; tr.SetTranslate( bone[bi].t );
  output[ bi ] = curr * tr * (fr.getRotationMatrix( bi ).transpose());

  for (unsigned int k=0; k<bone[bi].next.size(); k++) {
    SetBoneMatrices(fr, bone[bi].next[k] , output, output[bi] );
  }
}

std::vector<Matrix44f>  BrfSkeleton::GetBoneMatrices() const
{
  std::vector<Matrix44f> res;
  res.resize(bone.size());
  Matrix44f first;
  first.SetIdentity( );
  SetBoneMatrices( root, res, first);
  return res;
}

std::vector<Matrix44f>  BrfSkeleton::GetBoneMatricesInverse() const
{
  std::vector<Matrix44f> res;
  res.resize(bone.size());
  Matrix44f first;
  first.SetIdentity( );
  SetBoneMatricesInverse( root, res, first);
  return res;
}

bool BrfSkeleton::LayoutHitboxes(const BrfBody &in, BrfBody &out, bool inverse) const{
  if (bone.size()!=in.part.size())  return false;
  std::vector<Matrix44f> p = (!inverse)?GetBoneMatrices():GetBoneMatricesInverse();

  out = in;
  //Matrix44f m; m.V()
  for (unsigned int i=0; i<bone.size(); i++) {

      p[i].transposeInPlace();
      out.part[i].Transform(p[i].V());
  }
  out.UpdateBBox();

  return true;
}

void BrfSkeleton::SetBoneMatrices(int bi,
                                  std::vector<Matrix44f> &output, const Matrix44f &curr) const
{
  Matrix44f tr; tr.SetTranslate( bone[bi].t );
  output[ bi ] = curr * tr * Matrix44f( bone[bi].getRotationMatrix().transpose());

  for (unsigned int k=0; k<bone[bi].next.size(); k++) {
    SetBoneMatrices( bone[bi].next[k] , output, output[bi] );
  }
}

void BrfSkeleton::SetBoneMatricesInverse(int bi,
                                  std::vector<Matrix44f> &output, const Matrix44f &curr) const
{
  Matrix44f tr; tr.SetTranslate( -bone[bi].t );
  output[ bi ] = Matrix44f( bone[bi].getRotationMatrix()) * tr * curr;

  for (unsigned int k=0; k<bone[bi].next.size(); k++) {
    SetBoneMatricesInverse( bone[bi].next[k] , output, output[bi] );
  }
}

bool BrfBone::Skip(FILE *f){
  ::Skip(f,4);
  char str[255];
  if (LoadStringMaybe(f, str, "bone"))
    ::Skip(f,4*3*4+4);
  else
    ::Skip(f,4*3*4);
  return true;
}

bool BrfBone::Load(FILE*f, int /*verbose*/){
  LoadInt(f, attach);

  if (LoadStringMaybe(f, name, "bone")) // for back compatibility!!!
    LoadInt(f, b);

  LoadPoint(f,x);
  LoadPoint(f,z);
  LoadPoint(f,y);
  LoadPoint(f,t);

  if (attach>=0)
    *this = BrfSkeleton::adjustCoordSyst(*this);
  else
    *this = BrfSkeleton::adjustCoordSystHalf(*this);


    //t = vcg::Point3f(-t[0],t[2],t[1]);
    //x =  vcg::Point3f(-x[0],x[2],x[1]);
    //y =  vcg::Point3f(-y[0],y[2],y[1]);
    //z =  vcg::Point3f(-z[0],z[2],z[1]);
  //}

  return true;
}

void BrfBone::Save(FILE*f) const{
  SaveInt(f, attach);
  SaveStringNotempty(f, name, "noname");
  SaveInt(f, b);

  BrfBone b;
  if (attach>=0) {
    b = BrfSkeleton::adjustCoordSyst(*this);
  } else {
    b = BrfSkeleton::adjustCoordSystHalf(*this);
  }
  /*Point3f nx,ny,nz,nt;
  if (attach>=0) {
    nt = vcg::Point3f(-t[0],t[2],t[1]);
    nx = vcg::Point3f(-x[0],x[2],x[1]);
    ny = vcg::Point3f(-y[0],y[2],y[1]);
    nz = vcg::Point3f(-z[0],z[2],z[1]);
  } else {
    nx=x;ny=y;nz=z;nt=t;
  }
  SavePoint(f,-nx);
  SavePoint(f,ny);
  SavePoint(f,nz);
  SavePoint(f,nt);*/

  SavePoint(f,b.x);
  SavePoint(f,b.z);
  SavePoint(f,b.y);
  SavePoint(f,b.t);

}

void BrfBone::Export(FILE*f){
  fprintf(f,"  %s \n  ",name);
  fprintf(f,"  %f, %f, %f\n",x[0],x[1],x[2]);
  fprintf(f,"  %f, %f, %f\n",y[0],y[1],y[2]);
  fprintf(f,"  %f, %f, %f\n",z[0],z[1],z[2]);
  fprintf(f,"  %f, %f, %f\n",t[0],t[1],t[2]);
  fprintf(f,"  attach: %d,  [%d]\n",attach,b);


}

void BrfSkeleton::BuildTree(){
  root = -1;
  for (unsigned int i=0; i<bone.size(); i++)
    bone[i].next.clear();
  for (unsigned int i=0; i<bone.size(); i++){
    int a=bone[i].attach;
    if (a==-1) {
      assert (root==-1);
      root = i;
    }
    else {
      bone[a].next.push_back(i);
    }
  }
  assert(root!=-1);
}


void BrfSkeleton::Export(const wchar_t* fn){
  FILE* f = wfopen(fn,"wt");
  fprintf(f,"%s -- %ld bones:\n",name,bone.size());
  for (unsigned int i=0; i<bone.size(); i++){
    fprintf(f,"\n (%d) ",i);
    bone[i].Export(f);
  }
  fclose(f);
}

void BrfSkeleton::Save(FILE*f) const{
  SaveString(f, name);
  SaveVector(f,bone);
}

bool BrfSkeleton::Skip(FILE*f){
  if (!LoadString(f, name)) return false;
  SkipVectorR<BrfBone>(f);
  return true;
}

bool BrfSkeleton::Load(FILE*f, int verbose){
  if (!LoadString(f, name)) return false;
  if (verbose>0) printf("loading \"%s\"...\n",name);

  if (!LoadVector(f,bone)) return false;
  BuildTree();
  return true;
}

void BrfBone::setRotationMatrix(vcg::Matrix44f m){
  x = m.GetRow3(0);
  y = m.GetRow3(1);
  z = m.GetRow3(2);
}

vcg::Matrix44f BrfBone::getRotationMatrix() const{
  float res[16];
#if 0
  res[0+0*4]=x[0];res[1+0*4]=x[1];res[2+0*4]=x[2];res[3+0*4]=0;
  res[0+1*4]=y[0];res[1+1*4]=y[1];res[2+1*4]=y[2];res[3+1*4]=0;
  res[0+2*4]=z[0];res[1+2*4]=z[1];res[2+2*4]=z[2];res[3+2*4]=0;
  res[0+3*4]=t[0];res[1+3*4]=t[1];res[2+3*4]=t[2];res[3+3*4]=1;
#else
#if 0
  res[0+0*4]=x[0];res[0+1*4]=x[1];res[0+2*4]=x[2];res[0+3*4]=0;
  res[1+0*4]=y[0];res[1+1*4]=y[1];res[1+2*4]=y[2];res[1+3*4]=0;
  res[2+0*4]=z[0];res[2+1*4]=z[1];res[2+2*4]=z[2];res[2+3*4]=0;
  res[3+0*4]=0   ;res[3+1*4]=0   ;res[3+2*4]=0   ;res[3+3*4]=1;
#else
  res[0+0*4]=x[0];res[1+0*4]=x[1];res[2+0*4]=x[2];res[3+0*4]=0;
  res[0+1*4]=y[0];res[1+1*4]=y[1];res[2+1*4]=y[2];res[3+1*4]=0;
  res[0+2*4]=z[0];res[1+2*4]=z[1];res[2+2*4]=z[2];res[3+2*4]=0;
  res[0+3*4]=0   ;res[1+3*4]=0   ;res[2+3*4]=0   ;res[3+3*4]=1;
#endif
#endif
  return Matrix44f(res);

}
