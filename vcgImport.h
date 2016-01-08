/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef __VCGLIB_IMPORT
#define __VCGLIB_IMPORT

#include <wrap/io_trimesh/import_obj.h>
#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/import_stl.h>
#include <wrap/io_trimesh/import_off.h>
#include <wrap/io_trimesh/import_dae.h>

#include <locale>

namespace vcg {
namespace tri {
namespace io {

/**
This class encapsulate a filter for automatically importing meshes by guessing
the right filter according to the extension
*/

template <class OpenMeshType>
class Importer
{
private:
  enum KnownTypes { KT_UNKNOWN, KT_PLY, KT_STL, KT_OFF, KT_OBJ , KT_DAE };
static int &LastType()
{
  static int lastType= KT_UNKNOWN;
return lastType;
}

public:
// simple aux function that returns true if a given file has a given extesnion
static bool FileExtension(std::string filename,  std::string extension)
{
  std::locale loc1 ;
  std::use_facet<std::ctype<char> > ( loc1 ).tolower(&*filename.begin(),&*filename.rbegin());
  std::use_facet<std::ctype<char> > ( loc1 ).tolower(&*extension.begin(),&*extension.rbegin());
  std::string end=filename.substr(filename.length()-extension.length(),extension.length());
  return end==extension;
}

// Open Mesh
static int Open(OpenMeshType &m, const char *filename, CallBackPos *cb=0)
{
  int dummymask = 0;
  return Open(m,filename,dummymask,cb);
}

// Open Mesh and return the load mask (the load mask must be initialized first)
static int Open(OpenMeshType &m, const char *filename, int &loadmask, CallBackPos *cb=0)
{
  int err;
  if(FileExtension(filename,"ply"))
  {
    err = ImporterPLY<OpenMeshType>::Open(m, filename, loadmask, cb);
    LastType()=KT_PLY;
  }
  else if(FileExtension(filename,"stl"))
  {
    err = ImporterSTL<OpenMeshType>::Open(m, filename, loadmask, cb);
    LastType()=KT_STL;
  }
  else if(FileExtension(filename,"off"))
  {
    err = ImporterOFF<OpenMeshType>::Open(m, filename, loadmask, cb);
    LastType()=KT_OFF;
  }
  else if(FileExtension(filename,"obj"))
  {
    err = ImporterOBJ<OpenMeshType>::Open(m, filename, loadmask, cb);
    LastType()=KT_OBJ;
  }
  else if(FileExtension(filename,"dae"))
  {
    InfoDAE infoDAE;
    err = ImporterDAE<OpenMeshType>::Open(m, filename, infoDAE, cb);
    loadmask=infoDAE.mask;
    LastType()=KT_DAE;
  }  else {
    err=1;
    LastType()=KT_UNKNOWN;
  }

  return err;
}

static bool ErrorCritical(int error)
{
  switch(LastType())
  {
    case KT_PLY : return (error>0); break;
    case KT_STL : return (error>0); break;
    case KT_OFF : return (error>0); break;
    case KT_OBJ : return ImporterOBJ<OpenMeshType>::ErrorCritical(error); break;
    case KT_DAE : return (error>0); break;
  }

  return true;
}

static const char *ErrorMsg(int error)
{
  switch(LastType())
  {
    case KT_PLY : return ImporterPLY<OpenMeshType>::ErrorMsg(error); break;
    case KT_STL : return ImporterSTL<OpenMeshType>::ErrorMsg(error); break;
    case KT_OFF : return ImporterOFF<OpenMeshType>::ErrorMsg(error); break;
    case KT_OBJ : return ImporterOBJ<OpenMeshType>::ErrorMsg(error); break;
    case KT_DAE : return ImporterDAE<OpenMeshType>::ErrorMsg(error); break;
  }
  return "Unknown type";
}

static bool LoadMask(const char * filename, int &mask)
{
  bool err;

  if(FileExtension(filename,"ply"))
  {
    err = ImporterPLY<OpenMeshType>::LoadMask(filename, mask);
    LastType()=KT_PLY;
  }
  else if(FileExtension(filename,"stl"))
  {
    err=false;
    mask = Mask::IOM_VERTCOORD | Mask::IOM_FACEINDEX;
    LastType()=KT_STL;
  }
  else if(FileExtension(filename,"off"))
  {
    mask = Mask::IOM_VERTCOORD | Mask::IOM_FACEINDEX;
    err = ImporterOFF<OpenMeshType>::LoadMask(filename, mask);
    LastType()=KT_OFF;
  }
  else if(FileExtension(filename,"obj"))
  {
    err = ImporterOBJ<OpenMeshType>::LoadMask(filename, mask);
    LastType()=KT_OBJ;
  }
  else if(FileExtension(filename,"dae"))
  {
    //err = ImporterDAE<OpenMeshType>::LoadMask(filename, mask);
    err= false;
    LastType()=KT_DAE;
  }
  else
  {
    err = false;
    LastType()=KT_UNKNOWN;
  }

  return err;
}
}; // end class
} // end Namespace tri
} // end Namespace io
} // end Namespace vcg

#endif
