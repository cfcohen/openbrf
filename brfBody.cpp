/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <vector>
#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;

#include "brfBody.h"

#include "saveLoad.h"
#include "platform.h"

// for hitboxes:
// HACK: store/read original skel name into 2nd, 3rd, 4th bytes of flags of all parts
// (1st flag is used to signal "for_ragdoll_only")
char* BrfBody::GetOriginalSkeletonName() const{
  static char res[255];
  int j=0;
  for (int i=0; i<(int)part.size(); i++ ) {
    const BrfBodyPart &p(part[i]);
    res[j++] = 255&(p.flags>>8);
    res[j++] = 255&(p.flags>>16);
    res[j++] = 255&(p.flags>>24);
  }
  res[j+1]=0;
  return res;
}
void BrfBody::SetOriginalSkeletonName(const char* n){
  const char* c = n;
  for (int i=0; i<(int)part.size(); i++ ) {
    BrfBodyPart &p(part[i]);
    p.flags &= 0xFF;
    p.flags |= ((unsigned int)(*c))<<8;  if (*c) c++;
    p.flags |= ((unsigned int)(*c))<<16; if (*c) c++;
    p.flags |= ((unsigned int)(*c))<<24; if (*c) c++;
  }
}

void BrfBodyPart::Scale(float f){
    center*=f;
    dir*=f;
    radius*=f;
    for (unsigned int i=0; i<pos.size(); i++) pos[i]*=f;
}

BrfBody::BrfBody()
{
}

template<class T>
static void invertV(std::vector<T> v){
  int n=(int)v.size();
  for (int i=0; i<n/2; i++){
    T tmp=v[i]; v[i]=v[n-1-i]; v[n-1-i]=tmp;
  }
}

void BrfBodyPart::Flip(){
  switch(type) {
  case MANIFOLD: {
    for (unsigned int i=0; i<pos.size(); i++) pos[i].X()*=-1;
    for (unsigned int i=0; i<face.size(); i++) invertV(face[i]);
  };
  case CAPSULE: {
    center.X()*=-1;
    dir.X()*=-1;
    break;
  }
  case SPHERE: {
    center.X()*=-1;
    break;
  }
  case FACE:{
    for (unsigned int i=0; i<pos.size(); i++) pos[i].X()*=-1;
    invertV(pos);
    break;
  }
  default: return;
  }
}

void BrfBody::MakeQuadDominant(){
  for (unsigned int i=0; i<part.size(); i++)
    if (part[i].type==BrfBodyPart::MANIFOLD) part[i].MakeQuadDominant();
}

void BrfBody::Flip(){
  for (unsigned int i=0; i<part.size(); i++) part[i].Flip();
  UpdateBBox();
}

void BrfBody::Scale(float f){
  for (unsigned int i=0; i<part.size(); i++) part[i].Scale(f);
  bbox.min*=f;
  bbox.max*=f;
  if (f<0) std::swap(bbox.min,bbox.max);
}

void BrfBody::Transform(float *f){
  for (unsigned int i=0; i<part.size(); i++) part[i].Transform(f);
  UpdateBBox();
}

float* BrfBodyPart::GetRotMatrix() const{
  vcg::Point3f dx, dy, dz(1,1.1f,1.3f);
  dx = (dir-center)/2.0;
  dy = dz ^ dx;
  dz = dx ^ dy;
  dx.Normalize();
  dy.Normalize();
  dz.Normalize();
  static float res[16];
  for (int i=0; i<4; i++) {
    res[i]    = (i==3)?0:dx[i];
    res[i+4]  = (i==3)?0:dy[i];
    res[i+8]  = (i==3)?0:dz[i];
    res[i+12] = (i==3)?1:0;
  }
  return res;
}

const char* BrfBodyPart::name() const{
  if (IsEmpty()) return "empty";
  switch(type) {
  case MANIFOLD: return "manifold";
  case CAPSULE: return "cylinder";
  case SPHERE: return "sphere";
  case FACE:return "polygon";
  default: return "unknown";
  }

}

static void _addHypotesis(Point3f &a, Point3f b, Point3f c){
  Point3f an=a; an.Normalize();
  float b0 = an*b;
  float c0 = an*c;
  if (fabs(b0)>fabs(c0)){
    a+=(b0>0)?b:-b;
  } else {
    a+=(c0>0)?c:-c;
  }
}

Point3f  BrfBodyPart::Baricenter() const{
  Point3f res(0,0,0);
  for (unsigned int i=0; i<pos.size(); i++) res+=pos[i];
  if (pos.size()>0) res/=pos.size();
  return res;
}

bool _startsWith(const char*a, const char* b){
  char s[512];
  sprintf(s,"%s",a);
  s[ strlen(b) ] =0;
  return !stricmp(s,b);
}
void BrfBodyPart::InferTypeFromString(char* str){
  type = MANIFOLD;
  if (_startsWith(str,"sphere")) type = SPHERE;
  if (_startsWith(str,"cylinder")) type = CAPSULE;
  if (_startsWith(str,"cilinder")) type = CAPSULE;
  if (_startsWith(str,"capsule")) type = CAPSULE;
  if (_startsWith(str,"polygon")) type = FACE;
  if (_startsWith(str,"face")) type = FACE;
}

void BrfBodyPart::GuessFromManyfold(){
  switch(type) {
    case SPHERE: {
      center = Baricenter();
      radius = 0;
      for (unsigned int i=0; i<pos.size(); i++){
        float dist = (center-pos[i]).SquaredNorm();
        if (radius<dist) radius = dist;
      }
      radius = (float)sqrt(radius);
      face.clear();
      pos.clear();
      break;
    }
    case FACE: {
      // keep only one face
      face.resize(1);
      std::vector<Point3f> v = pos;
      pos.resize(face[0].size());
      for (int i=0; i<(int)face[0].size(); i++) {
        pos[i]=v[ face[0][i] ];
        face[0][i]=i;
      }
      break;
    }
    case CAPSULE: {
      // guess an axis looking at quadrilaterals
      Point3f ip0(0,0,0), ip1(0,0,0); // axis hipotesis
      bool first=true;
      for (unsigned int i=0; i<face.size(); i++) if (face[i].size()==4) {
        int a = (face[i])[0];
        int b = (face[i])[1];
        int c = (face[i])[2];
        //int d = face[i][3];
        Point3f ipA = (pos[a]-pos[b]); ipA.Normalize();
        Point3f ipB = (pos[b]-pos[c]); ipB.Normalize();
        if (first) {
          ip0 = ipA;
          ip1 = ipB;
        } else {
          _addHypotesis(ip0, ipA, ipB);
          _addHypotesis(ip1, ipA, ipB);
        }
        first = false;
      }
      Point3f axis = (ip0.SquaredNorm()>ip1.SquaredNorm())?ip0:ip1;
      //axis = Point3f(0,0,1);
      axis.Normalize();
      Point3f ce = Baricenter();
      radius = 0;
      float min=0; float max=0;
      for (unsigned int i=0; i<pos.size(); i++){
        float d = (pos[i]-ce)*axis;
        if (d<min) min = d;
        if (d>max) max = d;
        float dist = ((ce+axis*d)-pos[i]).SquaredNorm();
        if (radius<dist) radius = dist;
      }
      radius=(float)sqrt(radius);
      center = ce+axis*max;
      dir = ce+axis*min;
      face.clear();
      pos.clear();
      break;
    }
    default:
      break;
  }
  flags=0;
  this->ori=-1;
}

static bool fscanln(FILE*f, char *ln){
  int i=0;
  while (1) {
    if (!fread(&ln[i],1,1,f)) return false;
    if (ln[i]=='\n') { ln[i]=0; return true;}
    i++;
  }
  return false;
}

bool BrfBody::ImportOBJ(const wchar_t *fn){

  BrfBodyPart dump, curr;
  dump.flags=0;
  dump.ori=-1;
  dump.type = BrfBodyPart::MANIFOLD;

  int startV = 0; // starting v

  // to do: read all v and f fields, looking for "o" (objects)
  FILE* f = wfopen(fn,"rt");
  if (!f) return false;

  std::string s;
  char line[255];
  bool reading = false;
  int nvRead=0;
  while (fscanln(f, line)) {
    if (line[1]==' ') {
      char c=line[0];
      switch (c){
        case 'v':{
          Point3f p;
          sscanf(line, "v %f %f %f",&(p[0]),&(p[1]),&(p[2])); p[0]*=-1;
          if (reading) {
            curr.pos.push_back(p);
          } else {
            dump.pos.push_back(p);
          }
          nvRead ++;
          break;
        }
        case 'f':{
          static char buf[20][20];
          int res = sscanf(line, "f %s %s %s %s %s %s %s %s",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
          std::vector<int> fac;
          for (int i=0; i<res; i++){
            int d=2;
            //sscanf(buf[res-1-i],"%d",&d);
            sscanf(buf[i],"%d",&d);
            fac.push_back(d+startV-1);
          }
          if (reading) {
            curr.face.push_back(fac);
          } else {
            dump.face.push_back(fac);
          }
          break;
        }
        case 'o':
          char str[255];
          sscanf(line, "o %s", str);
          if (reading) {
            if (curr.face.size()>0) part.push_back(curr);
          }
          curr.face.clear();
          curr.pos.clear();
          curr.InferTypeFromString(str);
          reading = (curr.type != BrfBodyPart::MANIFOLD);
          startV = -nvRead;
          if (!reading) startV += dump.pos.size();
          break;
      }
    }
  }


  if (dump.face.size()) part.push_back(dump);
  if (reading) part.push_back(curr);
  for (unsigned int i=0; i<part.size(); i++) part[i].GuessFromManyfold();
  UpdateBBox();
  return true;
}

void BrfBodyPart::UpdateBBox(){
  bbox.SetNull();
  vcg::Point3f r(radius, radius, radius);
  if (type==MANIFOLD || type==FACE) {
    for (unsigned int i=0; i<pos.size(); i++) {
      bbox.Add(pos[i]);
    }
  }
  if (type==SPHERE) {
    bbox.Add(center+r);
    bbox.Add(center-r);
  }
  if (type==CAPSULE) {
    bbox.Add(center+r);
    bbox.Add(center-r);
    bbox.Add(dir+r);
    bbox.Add(dir-r);
  }
}

void BrfBody::UpdateBBox(){
  bbox.SetNull();
  for (unsigned int i=0; i<part.size(); i++) {
    part[i].UpdateBBox();
    bbox.Add(part[i].bbox);
  }
}


//typedef enum {RADIUS, LEN_TOP, LEN_BOT, POSX, POSY, POSZ, ROTX, ROTY} Attribute;
void BrfBodyPart::ChangeAttribute(int which, float howMuch){
  vcg::Point3f &a(center);
  vcg::Point3f &b(dir);
  vcg::Point3f d = b-a;
  vcg::Point3f dx,dy,mid;
  float l = d.Norm();
  float k;
  switch (which) {
  case RADIUS:
      radius+=howMuch;
      ChangeAttribute(LEN_TOP,-howMuch/2);
      ChangeAttribute(LEN_BOT,-howMuch/2);
      break;
  case LEN_TOP:
      k = (l+howMuch);
      if (k<0.001) k=0.001;
      b = a - (a-b)*k/l;
      break;
  case LEN_BOT:
      k = (l+howMuch);
      if (k<0.001) k=0.001;
      a = b - (b-a)*k/l;
      break;
  case POSX:
      a += (d)*(howMuch/l);
      b += (d)*(howMuch/l);
      break;
  case POSY:
  case POSZ:
  case ROTX:
  case ROTY:
      dx = d ^ vcg::Point3f(1,0,0);
      k = dx.Norm();
      if (!k) dx = (d ^ vcg::Point3f(1,0.2,0)).normalized();
      else dx/=k;
      if (which==POSZ || which==ROTX) {
        dx = (dx^d).normalized();
      }
      if (which==POSX || which==POSY || which==POSZ) {
        a += (dx)*(howMuch);
        b += (dx)*(howMuch);
        break;
      }
      // rotations...
      mid = (a+b)*0.5;
      a -= mid;
      a = a+dx*(howMuch*2)*(a.Norm()*4);
      a = (a.Normalize())*(l/2);
      b=mid-a;
      a=mid+a;
      break;

  }
}


bool BrfBodyPart::IsEmpty() const{
  //return ( (type==CAPSULE) && !radius);
  return ( (type==FACE) && !pos.size());
}

void BrfBodyPart::SetEmpty(){
  //radius = 0;type = CAPSULE;
  type = FACE; pos.clear(); face.clear();
  std::vector<int> f; face.push_back(f); // add a zero-vertices face
}


unsigned char BrfBodyPart::GetHitboxFlags() const{
  return flags&0xFF;
}
void BrfBodyPart::SetHitboxFlags(unsigned char f){
  flags &= 0xFFFFFF00;
  flags |= f;
}


void BrfBodyPart::SetAsDefaultHitbox(){
  type = CAPSULE;
  dir = vcg::Point3f( 0.125,0,0); // default axis values
  center    = vcg::Point3f(-0.03125,0,0);
  radius = 0.125;
  SetHitboxFlags(0);
}

void BrfBodyPart::SymmetrizeCapsule(const BrfBodyPart &p){
  if (p.IsEmpty()) { SetEmpty(); return;}
  if (type!=CAPSULE) return;
  if (p.type!=CAPSULE) return;
  radius = p.radius;
  center = p.center;
  dir = p.dir;
  center[2]*=-1;
  dir[2]*=-1;
  SetHitboxFlags( p.GetHitboxFlags() );

}

void BrfBodyPart::SymmetrizeCapsule(){
  if (type!=CAPSULE) return;
  Point3f &a(center), &b(dir);
  Point3f m = (a+b)/2.0;
  a[2]-=m[2]; b[2]-=m[2];
  float err1 = 2*((a-m)[2])*((a-m)[2]);
  float err2 = ((a-m)[0])*((a-m)[0]) + ((a-m)[1])*((a-m)[1]);

  if (err1<err2) a[2]=b[2]=0;
  else {
      a[0]=b[0]=m[0];
      a[1]=b[1]=m[1];
  }

}


bool BrfBodyPart::Load(FILE*f, char* _firstWord, int /*verbose*/ ){
  char firstWord[255];

  if (!_firstWord) {
    if (!LoadString(f,firstWord)) return false;
  }
  else sprintf(firstWord,"%s",_firstWord);

  if (!strcmp(firstWord,"manifold")) {
    type=MANIFOLD;
    if (!LoadVector(f, pos)) return false;// # points

    int k;
    LoadInt(f,k); // # faces
    for (int i=0; i<k; i++) {

      LoadInt(f,ori); // orientation? -1 or 1 apparently.
      //if (ori!=-1) ori=1; // patch?
      //assert(ori==1 || ori==-1);
      int h;
      LoadInt(f,h); // flags
      assert(h==0);
      LoadInt(f,h); // # verts
      std::vector<int> v;
      for (int j=0; j<h; j++) {
        int pp;
        LoadInt(f,pp);
        v.push_back(pp);
      }
      face.push_back(v);
    }
  } else if (!strcmp(firstWord,"capsule")) {
    type=CAPSULE;
    LoadFloat(f,radius);
    LoadPoint(f,center);
    LoadPoint(f,dir);
    LoadUint(f,flags);
    //fprintf(fff,"Flags cil: %u\n",flags);
    //fprintf(ftmp,"%f %f %f %f %f %f %f\n",radius,center[0],center[2],center[1],dir[0],dir[2],dir[1]);
    //fflush(ftmp);

  } else if (!strcmp(firstWord,"sphere")) {
    type=SPHERE;
    LoadFloat(f,radius);
    LoadPoint(f,center);
    LoadUint(f,flags);
    //fprintf(fff,"Flags sphere: %u\n",flags);
  } else if (!strcmp(firstWord,"face")) {
    type=FACE;
    if (!LoadVector(f,pos)) return false;

    int k = pos.size();

    // default face
    std::vector<int> aface;
    for (int i=0; i<k; i++) aface.push_back(i);
    face.push_back(aface);

    LoadUint(f,flags);
    //fprintf(fff,"Flags face: %u\n",flags);

  } else {
    printf("Unknown body (collision mesh) type `%s`\n",firstWord);
    assert(0);
  }

  //UpdateBBox();
  return true;
}


bool BrfBodyPart::Skip(FILE*f, char* _firstWord){
  char firstWord[255];

  if (!_firstWord) { if (!LoadString(f,firstWord)) return false; }
  else sprintf(firstWord,"%s",_firstWord);


  if (!strcmp(firstWord,"manifold")) {

    SkipVectorB<Point3f>(f);// # points

    int k;
    LoadInt(f,k); // # faces
    for (int i=0; i<k; i++) {
      ::Skip(f,8);
      SkipVectorB<int>(f);
    }
  } else if (!strcmp(firstWord,"capsule")) {
    ::Skip(f,4+12+12+4);
  } else if (!strcmp(firstWord,"sphere")) {
    ::Skip(f,4+12+4);
  } else if (!strcmp(firstWord,"face")) {
    SkipVectorB<Point3f>(f);

    ::Skip<int>(f);
  } else {
    assert(0);
  }
  return true;
}

void BrfBodyPart::Transform(float *f) {
  vcg::Matrix44f m(f); m.transposeInPlace();
  switch(type) {
  case MANIFOLD:
  case FACE:
    for (unsigned int i=0; i<pos.size(); i++){
      pos[i] = m*pos[i];
    }
    break;
  case CAPSULE:
  case SPHERE:
    center = m*center;
    dir = m*dir;
    radius *= pow(double(m.Determinant()),1.0/3.0);
    break;
  default: break;
  }
}

void BrfBodyPart::Save(FILE *f) const {
  switch(type) {
  case MANIFOLD:
      SaveString(f,"manifold");
      SaveVector(f,pos);

      SaveUint(f,face.size());
      for (unsigned int i=0; i<face.size(); i++) {
        SaveInt(f,ori);
        SaveUint(f,0); // flags
        SaveUint(f,face[i].size());
        for (unsigned int j=0; j<face[i].size(); j++) {
          SaveInt(f,face[i][j]);
        }
      }
      break;
  case CAPSULE:
      SaveString(f,"capsule");
      SaveFloat(f,radius);
      SavePoint(f,center);
      SavePoint(f,dir);
      SaveUint(f,flags);
      break;
  case SPHERE:
      SaveString(f,"sphere");
      SaveFloat(f,radius);
      SavePoint(f,center);
      SaveUint(f,flags);
      break;
  case FACE:
      SaveString(f,"face");
      assert(face.size()==1);
      SaveVector(f,pos);
      SaveUint(f,flags);
      break;
  default: assert(0);
  }
}

void BrfBody::Save(FILE*f) const{
  SaveString(f,name);
  if (part.size()==1) {
    part[0].Save(f);
  } else {
    SaveString(f,"composite");
    SaveVector(f,part);
  }
}

void BrfBodyPart::Merge(const BrfBodyPart &b){
  assert(type==MANIFOLD || type == FACE);
  assert(b.type==MANIFOLD || b.type == FACE);
  int k = (int)pos.size();
  for (unsigned int i=0; i<b.pos.size(); i++)
  pos.push_back(b.pos[i]);

  for (unsigned int i=0; i<b.face.size(); i++){
    std::vector<int> nf;
    for (unsigned int j=0; j<b.face[i].size(); j++){
      nf.push_back(b.face[i][j]+k);
    }
    face.push_back(nf);
  }
}

bool BrfBody::Merge(const BrfBody &b){
  int im = -1; // index of 1st manifold

  for (unsigned int i=0; i<part.size(); i++){
    if (part[i].type==BrfBodyPart::FACE || part[i].type==BrfBodyPart::MANIFOLD){
      im = i; break;
    }
  }
  for (unsigned int i=0; i<b.part.size(); i++){
    if (b.part[i].type==BrfBodyPart::FACE || b.part[i].type==BrfBodyPart::MANIFOLD){
      if (im>=0) part[im].Merge(b.part[i]);
      else {
        im = part.size();
        part.push_back(b.part[i]);
      }
    } else {
      part.push_back(b.part[i]);
    }

  }
  UpdateBBox();
  return true;

}

bool BrfBodyPart::ExportOBJ(FILE* f, int i, int &vc) const{
  fprintf(f,"o %s_%d\n",name(),i);
  bool asHitbox = false; //true;
  switch (type) {
  default:
    break;
  case MANIFOLD:
  case FACE:
    for (unsigned int i=0; i<pos.size(); i++) {
      fprintf(f,"v %f %f %f\n",-pos[i].X(),pos[i].Y(),pos[i].Z());
    }
    for (unsigned int i=0; i<face.size(); i++) {
      fprintf(f,"f ");
      for (unsigned int j=0; j<face[i].size(); j++)
      fprintf(f,"%d// ",vc+face[i][j]);
      fprintf(f,"\n");
    }
    vc+=pos.size();
    break;
  case CAPSULE:{
    vcg::Point3f a = this->center;
    vcg::Point3f b = this->dir;

    vcg::Point3f dx, dy, dz(1,1.1,1.3);
    dx = (a-b)/2.0;
    dy = dz ^ dx;
    dz = dx ^ dy;
    dx.Normalize();
    dy.Normalize();
    dz.Normalize();
    int N=10;
    for (int i=0; i<N; i++) {
      float ca = (float)cos(M_PI*2*i/N)*radius;
      float sa = (float)sin(M_PI*2*i/N)*radius;
      vcg::Point3f p;
      p = a+dy*sa+dz*ca;
      fprintf(f,"v %f %f %f\n",-p.X(), p.Y(), p.Z());
      if (asHitbox) fprintf(f,"vt 0.66 %f\n",i/(float(N)));
      p = b+dy*sa+dz*ca;
      fprintf(f,"v %f %f %f\n",-p.X(), p.Y(), p.Z());
      if (asHitbox) fprintf(f,"vt 0.33 %f\n",i/(float(N)));
    }
    if (asHitbox) {
      // caps!!!
      a+=dx*radius;
      b-=dx*radius;
    }
    fprintf(f,"v %f %f %f\n",-a.X(), a.Y(), a.Z());
    if (asHitbox) fprintf(f,"vt 1 0.5\n");

    fprintf(f,"v %f %f %f\n",-b.X(), b.Y(), b.Z());
    if (asHitbox) fprintf(f,"vt 0 0.5\n");

    for (int h=0; h<N; h++) {
      int i = h*2;
      int j = ((h+1)%N)*2;

      /*if (asHitbox) {
        int a = 0;
        fprintf(f,"f %d//%d %d//%d %d//%d\n",       a=vc+N*2,  a, a=vc+i,  a, a=vc+j,a   );
        fprintf(f,"f %d//%d %d//%d %d//%d %d//%d\n",a=vc+i,    a, a=vc+i+1,a, a=vc+j+1,a, a=vc+j,a );
        fprintf(f,"f %d//%d %d//%d %d//%d\n",       a=vc+N*2+1,a, a=vc+j+1,a, a=vc+i+1,a );
      } else*/ {
        fprintf(f,"f %d// %d// %d//\n",       vc+N*2, vc+i, vc+j );
        fprintf(f,"f %d// %d// %d// %d//\n",  vc+i, vc+i+1, vc+j+1, vc+j );
        fprintf(f,"f %d// %d// %d//\n",       vc+N*2+1, vc+j+1, vc+i+1 );
      }
    }
    vc+=2*N+2;
    break;
  }
  case SPHERE:{
    int N=10;
    int M=5;

    Point3f p;
    for (int j=0; j<M; j++)
    for (int i=0; i<N; i++) {     
      p.FromPolarRad(radius,M_PI*2*i/N,M_PI*(j+1)/(M+1)-M_PI/2); p+= center;
      fprintf(f,"v %f %f %f\n",-p.X(), p.Y(), p.Z());
    }
    p.FromPolarRad(radius,0,-M_PI/2); p+= center;
    fprintf(f,"v %f %f %f\n",-p.X(), p.Y(), p.Z());
    p.FromPolarRad(radius,0,+M_PI/2); p+= center;
    fprintf(f,"v %f %f %f\n",-p.X(), p.Y(), p.Z());

    for (int i=0; i<N; i++) {
      int ip = (i+1)%N;
      fprintf(f,"f %d// %d// %d//\n", vc+N*M, vc+ip, vc+i );
      for (int j=0; j<M-1; j++)
        fprintf(f,"f %d// %d// %d// %d//\n", vc+ip+j*N+N, vc+i+j*N+N, vc+i+j*N , vc+ip+j*N );
      fprintf(f,"f %d// %d// %d//\n", vc+N*M+1, vc+i+(M-1)*N , vc+ip+(M-1)*N);
    }
    vc+=N*M+2;
    break;
  }
  }
  return true;
}

bool BrfBody::ExportOBJ(const wchar_t* fn) const {
  FILE* f =wfopen(fn,"wb");
  if (!f) return false;
  fprintf(f,
    "# export of a body (Mount and Blade collision object)\n"
    "# by OpenBrf (Marco Tarini)\n");
  int vc=1; // vertex count
  for (unsigned int i=0; i<part.size(); i++)
    part[i].ExportOBJ(f,i,vc);
  fclose(f);
  return true;
}

bool BrfBody::Skip(FILE*f){
  // return Load(f,2); // TEMP!!!!! Slow loading, to count all used flags
  if (!LoadString(f, name)) return false;
  char str[255];
  if (!LoadString(f, str)) return false;

  if (!strcmp(str,"composite")) {
    SkipVectorR<BrfBodyPart>(f);
  } else {
    BrfBodyPart::Skip(f,str);
  }
  return true;

}
bool BrfBody::Load(FILE*f, int verbose){
  if (!LoadString(f, name)) return false;
  if (verbose>0) printf("loading \"%s\"...\n",name);

  char str[255];
  if (!LoadString(f, str)) return false;

  if (!strcmp(str,"composite")) {
    if (!LoadVector(f, part)) return false;
  } else {
    BrfBodyPart b;
    if (!b.Load(f,str,verbose)) return false;
    part.push_back(b);
  }
  UpdateBBox();

  return true;
}
