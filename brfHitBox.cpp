/* OpenBRF -- by marco tarini. Provided under GNU General Public License */


#include <vector>
#include <vcg/space/box3.h>
#include <vcg/space/point3.h>
#include <vcg/space/point2.h>
using namespace vcg;

#include "brfBody.h"

//#include "brfHitBox.h"

#include "brfData.h"

#include <QFile>
#include <QDomDocument>

/*void BrfHitBoxSet::clear(){
    data.clear();
}*/

static bool qDomElement_to_BrfBodyPart(const QDomElement & n, BrfBodyPart & p){
  QString type = n.attribute("type");
  if (type == "capsule") {
      p.ori = 1;
      p.flags = 0;
      bool ok = true, res = true;
      p.type = BrfBodyPart::CAPSULE;
      p.radius = n.attribute("radius").toDouble(&res); ok &=res;
      p.center.Z() = n.attribute("pos_x").toDouble(&res); ok &=res;
      p.center.X() = n.attribute("pos_y").toDouble(&res); ok &=res;
      p.center.Y() = -n.attribute("pos_z").toDouble(&res); ok &=res;
      p.dir.Z() = n.attribute("pos2_x").toDouble(&res); ok &=res;
      p.dir.X() = n.attribute("pos2_y").toDouble(&res); ok &=res;
      p.dir.Y() = -n.attribute("pos2_z").toDouble(&res); ok &=res;
      if (n.attribute("for_ragdoll_only","0").toInt()==1) p.flags = 1;
      //p.dir -= p.center;
      //p.center -= p.dir;
      //if (!ok) qDebug("WRONG (%s)", n.attribute("radius").toAscii().data());
      return ok;
  } else {
      p.SetEmpty();
      //qDebug("UNKOWN (%s)", type.toAscii().data());
      return false;
  }
}

static bool BrfBodyPart_to_qDomElement(const BrfBodyPart & p, QDomElement & n, QDomDocument &mainDoc){

  if (p.IsEmpty()) {
    n.clear();
    return true;
  }

  if (p.type!=BrfBodyPart::CAPSULE) {
      //qDebug("not a capsule: %d",p.type);
      BrfData::LastHitBoxesLoadSaveError("Hitboxes can be capsules only");
      return false;
  }

  n = mainDoc.createElement("Bodies");
  QDomElement n2 = mainDoc.createElement("body");


  n2.setAttribute("type",QString("capsule"));
  n2.setAttribute("pos_x", p.center.Z());
  n2.setAttribute("pos_y", p.center.X());
  n2.setAttribute("pos_z", -p.center.Y());
  n2.setAttribute("pos2_x", p.dir.Z());
  n2.setAttribute("pos2_y", p.dir.X());
  n2.setAttribute("pos2_z", -p.dir.Y());
  n2.setAttribute("radius", p.radius);

  if (p.flags&1) n2.setAttribute("for_ragdoll_only","1");

  if (n.appendChild(n2).isNull()) {
    BrfData::LastHitBoxesLoadSaveError("Connot append <body> into <bodies>");
    return false;
  }

  return true;
}

static bool BrfBody_to_qDomElement(const BrfBody & b, QDomElement &skelNode, QDomDocument &mainDoc){

    skelNode.setAttribute("name",b.name);

    wchar_t filename[1]; filename[0] = 0;

    QDomElement bonesNode = skelNode.firstChildElement("Bones");
    if (bonesNode.isNull()) {
        BrfData::LastHitBoxesLoadSaveError("%ls No 'Bones' node found in XML skel \"%s\"",
           filename,skelNode.attribute("name").toAscii().data());
        return false;
    }

    QDomElement boneNode = bonesNode.firstChildElement("Bone");
    if (bonesNode.isNull()) {
        BrfData::LastHitBoxesLoadSaveError("%ls No 'Bone' node found in 'Bones' of XML skel \"%s\"",
           filename,skelNode.attribute("name").toAscii().data());
        return false;
    }

    int k=0; // bodypart counter
    while (!boneNode.isNull()) {
      if (k>=(int)b.part.size()) {
          qDebug("ERROR saving brf-part %d of brf-body '%s' (over xml-bone '%s' of xml-skel %s)",
                 k,b.name,boneNode.attribute("name").toAscii().data(),skelNode.attribute("name").toAscii().data());
          BrfData::LastHitBoxesLoadSaveError("Too many bones %ls:\n in base hitbox %s with respect to skeleton %s ",
             filename,b.name, skelNode.attribute("name").toAscii().data() );
          return false;
      }

      QDomElement oldBodiesEl=boneNode.firstChildElement("Bodies");
      QDomElement newBodiesEl;

      if (!BrfBodyPart_to_qDomElement( b.part[k], newBodiesEl, mainDoc )) return false;

      if (!oldBodiesEl.isNull() && !newBodiesEl.isNull() ) {
        if (boneNode.replaceChild( newBodiesEl, oldBodiesEl).isNull()) {
          BrfData::LastHitBoxesLoadSaveError("Error replacing <Bodies> in <Bone>");
          return 0;
        }
      } else if (!oldBodiesEl.isNull()) {
        if (boneNode.removeChild(oldBodiesEl).isNull()) {
          BrfData::LastHitBoxesLoadSaveError("Error removing <Bodies> from <Bone>");
          return 0;
        }
      } else if (!newBodiesEl.isNull()) {
         if (boneNode.appendChild( newBodiesEl ).isNull()) {
           BrfData::LastHitBoxesLoadSaveError("Error adding <Bodies> in <Bone>");
           return 0;
         }
      }

      boneNode = boneNode.nextSiblingElement("Bone"); // next bone in xml...
      k++; // ... and next part of hitbox

    }
    if (k!=(int)b.part.size()){
        BrfData::LastHitBoxesLoadSaveError("Too many bones (file %ls):\n in base hitbox %s with respect to skeleton %s",
           filename,b.name, skelNode.attribute("name").toAscii().data());
        return false;
    }
    return true;
}

char* BrfData::LastHitBoxesLoadSaveError(const char *st, const wchar_t *subst1, const char *subst2, const char *subst3){
  static char str[512];
  if (st && subst1 && subst2 && subst3) {
    sprintf(str, st, subst1, subst2, subst3);
  }
  if (st && subst1 && subst2) {
    sprintf(str, st, subst1, subst2);
  }
  if (st && subst1) {
    sprintf(str, st, subst1);
  }
  if (st) {
    sprintf(str, "%s", st);
  }
  return str;
}


int BrfData::SaveHitBoxesToXml(const wchar_t *fin, const wchar_t *fout) {

    const wchar_t *filename = fin;

    QFile file( QString::fromWCharArray(filename) );
    if( !file.open( QIODevice::ReadOnly ) ) {
        LastHitBoxesLoadSaveError("Could not open '%ls' for reading.",filename);
        return -1; // file not found
    }

    QDomDocument doc;
    if( !doc.setContent( &file ) ) {
        LastHitBoxesLoadSaveError("Could not scan XML contents from %ls",filename);
        file.close();
        return 0; // file not found
    }
    file.close();

    QDomNode root = doc.documentElement();

    QDomNode skelSetOld = root.firstChildElement("Skeletons");

    if (skelSetOld.isNull()) {
        LastHitBoxesLoadSaveError("no Skeletons found in %ls",filename);
        return false;
    };

    QDomElement skelSetNew = doc.createElement("Skeletons"); // skelSetOld.cloneNode();
    while(1) {
      QDomNode n = skelSetNew.firstChild();
      if (n.isNull()) break;
      skelSetNew.removeChild(n);
    }

    for (int i=0; i<(int)body.size();i++) {
        // find child "Skeleton" of name baseSkelName
        QDomElement skelNew;
        QDomElement skelOld = skelSetOld.firstChildElement("Skeleton");
        char *baseSkelName = body[i].GetOriginalSkeletonName();
        while (!skelOld.isNull()) {
            if  (skelOld.attribute("name")==baseSkelName) {
              skelNew = skelOld.cloneNode().toElement();
              break;
            }
            skelOld = skelOld.nextSiblingElement("Skeleton");
        }
        if (skelNew.isNull()) {
            LastHitBoxesLoadSaveError("in %ls, no Skeleton named %s found",fin, baseSkelName);
            return 0;
        }

        qDebug("Found skel %s for skel %s",baseSkelName,body[i].name);

        if (BrfBody_to_qDomElement(body[i],skelNew,doc)!=1)  return 0;
        // add body data
        skelSetNew.appendChild(skelNew);
    }


    root.replaceChild(skelSetNew,skelSetOld);

    // save final result in file...
    QFile fileout( QString::fromWCharArray(fout) );

    if( !fileout.open( QIODevice::WriteOnly ) ) {
        LastHitBoxesLoadSaveError("Could not open '%ls' for writing.",filename);
        return 0; // file not found
    }

    fileout.write(doc.toString(1).toAscii());
    //doc.save(fileout,0);
    fileout.close();

    return 1;

}

int BrfData::LoadHitBoxesFromXml(const wchar_t *filename) {

  Clear();
  QString fn = QString::fromWCharArray(filename);
  QFile file( fn );
  if( !file.open( QIODevice::ReadOnly ) ) {
      LastHitBoxesLoadSaveError("Could not open '%ls' for reading.",filename);
      return -1; // file not found
  }

  QDomDocument doc;
  if( !doc.setContent( &file ) ) {
      LastHitBoxesLoadSaveError("Could not scan XML contents from %ls",filename);
      file.close();
      return 0; // file not found
  }
  file.close();
  QDomElement root = doc.documentElement();
  QDomNode n = root;

  /*
  n = root.firstChildElement("Data");
  if (n.isNull()) {
      qDebug("no Data found");
      return false; // no Data found
  } else   qDebug("data found");
*/

  n = n.firstChildElement("Skeletons");

  if (n.isNull()) {
      LastHitBoxesLoadSaveError("no Skeletons found in %ls",filename);
      return false;
  } //else qDebug("Skeletons found");

  n = n.firstChildElement("Skeleton");
  while (!n.isNull()) {
      QString skelName = n.toElement().attribute("name");
      //qDebug("Skeleton '%s'",skelName.toAscii().data() );
      QDomNode n1 = n.firstChildElement("Bones");
      n1 = n1.firstChildElement("Bone");

      BrfBody newBody;

      while (!n1.isNull()) {
        QString boneName = n1.toElement().attribute("name");
        // qDebug("  bone '%s'",boneName.toAscii().data() );
        QDomNode n2 = n1.firstChildElement("Bodies");
        n2 = n2.firstChildElement("body");

        BrfBodyPart part;
        part.SetAsDefaultHitbox();
        part.SetHitboxFlags(0);
        part.SetEmpty();


        while (!n2.isNull()) {
            qDomElement_to_BrfBodyPart(n2.toElement(),part);
            n2 = n2.nextSiblingElement("body");
        }
        newBody.part.push_back( part );


        n1 = n1.nextSiblingElement("Bone");

      }

      if (newBody.part.size()>0) {
          sprintf(newBody.name, "%s",skelName.toAscii().data());
          newBody.SetOriginalSkeletonName(newBody.name); // remember I was originated from myself

          //qDebug("Saving string: '%s'",newBody.name);
          //qDebug("Saved string: '%s'",newBody.GetOriginalSkeletonName());
          //qDebug("saved '%s', stored '%s'",newBody.name, newBody.GetOriginalSkeletonName());
          newBody.UpdateBBox();
          body.push_back(newBody);
      }



      n = n.nextSiblingElement("Skeleton");
  }
  return 1; // ok!

}

