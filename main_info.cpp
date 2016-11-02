/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "glwidgets.h"
#include "mainwindow.h"

#include <QtGui>
#include <QMessageBox>
#include <QApplication>
#include <QSpacerItem>
#include <QGridLayout>

const char* applVersion = "0.0.82e";
QString IniData::tokenFullName(int k){

  switch (k){
  case MESH: return QString(MainWindow::tr("Mesh"));
  case TEXTURE: return MainWindow::tr("Texture");
  case SHADER: return MainWindow::tr("Shader");
  case MATERIAL: return MainWindow::tr("Material");
  case SKELETON: return MainWindow::tr("Skeleton");
  case ANIMATION: return MainWindow::tr("Animation");
  case BODY: return MainWindow::tr("Collision");
  default:  return MainWindow::tr("???");
  }
}

QString IniData::tokenPlurName(int k){
  switch (k){
  case MESH: return MainWindow::tr("Meshes");
  case TEXTURE: return MainWindow::tr("Textures");
  case SHADER: return MainWindow::tr("Shaders");
  case MATERIAL: return MainWindow::tr("Materials");
  case SKELETON: return MainWindow::tr("Skeletons");
  case ANIMATION: return MainWindow::tr("Animations");
  case BODY: return MainWindow::tr("Collisions");
  default:  return MainWindow::tr("???");
  }
}


void MainWindow::about()
{
  /*
  QDialog msg(this);
  QBoxLayout l(QBoxLayout::LeftToRight);
  msg.setWindowTitle("OpenBrf");
  msg.setLayout(&l);
  QPixmap p = QIcon(":/openBrf.ico").pixmap(256,256);
  QLabel logo;
  logo.setPixmap( p );
  msg.layout()->addWidget(&logo);
  QLabel lab(*/

 QMessageBox msg(QMessageBox::Information,"OpenBrf",
  //QMessageBox::about(this, ("Open-Brf"),
              QString(tr(
               "<p>"
               "&nbsp; &nbsp; <b>OpenBrf</b><br>"
               "&nbsp; &nbsp; by <b>%2</b></p><p>"
               "&nbsp; &nbsp; <b>ver %6</b><br>"
               "&nbsp; &nbsp; (%1)<br></p>"
               "<p><i>Testing, bug reporting, suggestions by:</i> %3</p>"
               "<p><i>Additional art by:</i> %4</p>"
               "<p><i>Translations by:</i> %5</p>")

             ).arg(__DATE__)
              .arg("[mtarini] --- Marco Tarini")
              .arg(QString::fromLatin1(
                   " <br>[Abhuva], [amade], [Andrde Cuyne], [Barabas], "
                   "[Barf], [Bilwit], [Block], [Bluehawk], [Bolkonsky], [Brutus], [Caba`drin], "
                   "[captain lust], [cdvader], [Chel], [captain lust], [Computica], [cmpxchg8b], [Crazy-Q], [DaBlade], "
                   "[Dain Ironfoot], "
                   "[Darwin], [dia151], [dreamterror], [dunde], [ealabor], [eierkopf], "
                   "[EvolutiveBrain], "
                   "[Fafhrd], [fedeita], [Fei Dao], [foxyman], [Freddex], [FrisianDude], "
				   "[Geroj], [GetAssista], [giles], [Gothic Knight], [Harry_], [Hatonastick], "
                   "[havarez], [Hemiola], [Highelf], [Highlander], [HokieBT], [Ikaguia], "
                   "[iggorbb], [iLag], [Jai], [Khamukkamu], [killkhergit], "
                   "[Konar], [k0nr@d], [Llew], [Lord_Cheap], [Lord Kinlar], [lord_olafson], "
                   "[LordRaglan], [Lumos], [MadocComadrin], [MadVader], [Mandible], [Mekelan], "
                   "[Merlkir], [mr.master], "
                   "[mysstick], [N0body], [newaxekub], "
                   "[octoburn], [OrangeKnight], [pagan], [Percus], [phlpp], [qlithe], [Rath0s], [RATMdude92], [Red River], "
                   "[rgcotl], [Roemerboy], [Romainoir], [Ruthven], [SenpaiHinds], "
                   "[Septa Scarabae], [Sayd Ûthman], [Silesian], [Shik], "
                   "[Silver Wolf], [Shredzorz], [Slawomir of Aaarrghh], [Slytacular], [Somebody], [SonKidd], [Spak], "
                   "[Specialist], [Stefano], [stephan_dinavoa], [Sunnetci_Dede], [Swyter], "
                   "[themanwhocrys2],  [thick1988], [Triglav], [Tul], [Ursca], "
                   "[Vincenzo], [Vlejundo], [Vornne], [WilliamBerne], "
                   "[yellowmosquito], [Yoshiboy], [xenoargh], [Zimke Zlovoljni]" ))
              .arg("<br>[amade], [Swyter]")
              .arg(QString::fromLatin1(
                   "<br> [foxyman] <i>(%1)</i><br>"
                   " [Swyter] <i>(Español)</i><br>"
                   " [Vlejundo] and [Roemerboy] <i>(Deutsche)</i>")
              .arg(tr("additional code and Chinese")))
              .arg(applVersion)
                 +QString("<p><i>%1</i><br>%2</p>").arg(tr("With detailed info about flag meanings provided by:")).arg("[cmpxchg8b]")
            ,QMessageBox::Ok, this);
 msg.setLocale(QLocale::system());
 //msg.layout()->addWidget(&lab);

 msg.setIconPixmap( windowIcon().pixmap(128,128) );

 QSpacerItem* horizontalSpacer = new QSpacerItem(550, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
 QGridLayout* layout = (QGridLayout*)msg.layout();
 layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

 msg.exec();
}


void MainWindow::optionAutoFixTextureShowInfo(){
   QMessageBox::about(this, "OpenBRF",
            tr("<b>What is this autofix DXT texture option?</b><br>"
               "<p>"
               "Many DDS texture creation programs/plugins around will output DXT1 textures "
               "with a minor error in the header.</p><p>"
               "This error confuses me (OpenBRF) but not Mount and Blade (or many other programs).</p><p>"
               "(When I cannot read a texture for this or any other problem, I display a chekerboard pattern instead).</p><p>"
               "If you want, I can silently fix this error every time I encounter it "
               "(I'll shamelessly write on the texture dss files on disk).</p>"
               ));

}

bool MainWindow::askIfUseOpenGL2(bool e){
    if (e) QMessageBox::information(this,"OpenBrf",tr("Activating preview Shaders\n(can be disabled under Settings)"));
	return true;
	/*
 return (QMessageBox::question(this,"OpenBrf",tr(
    "<b>Activate OpenGL2.0?</b>"
    "<p>OpenGL2.0 is needed to preview<br>"
    "bumpmaps, \"iron\" shader, specular maps...<br>"
    "but it has been reported to crash a few computers,<br>"
    "with (older?) ATI or Intel graphic cards.<br><br>%1"
    ).arg(e?tr("<i>(later you can set this option under [Settings])"):""),QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes);
		*/
}

void MainWindow::aboutColorAssignment(){
	QString info = tr(
		"<p><b>Info on mesh recoloring:</b><br/>when activating any tool which assigns<br/>new vertex-colors to an entire mesh: "
		"<ul>"
		"<li> keep <b>Shift</b> pressed:<br/>new colors will <i>multiply</i> existing ones</li>"
		"</ul>"
		"(by <b>default</b>, new colors <i>replace</i> existing ones).</p>"
	);

	/* info += tr(
		"<p>This applies to the following tools:<br/>[%1],<br/>[%2],<br/>[%3],<br/>[%4],<br/></p>"
	 ).arg(editPasteVertColorsAct->text())
	  .arg(selector->meshComputeAoAct->text())
	  .arg(selector->meshColorWithTextureAct->text())
	  .arg(selector->meshRecolorAct->text())
	 ); */

	QMessageBox::information(this,"OpenBrf", info );
}

void MainWindow::aboutCurrentShader(){
	QString log  = glWidget->getCurrentShaderLog();
	QString extraText  = tr("<br><p><i>This info has been copyed to clipboard</i>");
	bool errors = true;
	if (log.isEmpty()) {
		log = tr("ok");
		extraText = "";
		errors = false;
	};
	QString text = tr(
		"<p><i>Currently used preview Shader:</i><br />%1</p>"
		"<p><br /><i>Shader status:</i><br />%2</p>"
	).arg(glWidget->getCurrentShaderDescriptor()).arg(log);

	QMessageBox::about(this,tr("OpenBrf - Preview Shader info"),text+extraText);
	if (errors)
	QApplication::clipboard()->setText(
		text.replace("<br />","\n").replace("<p>","\n").replace("</p>","\n").replace("<i>","").replace("</i>","")
	);
}

QString MainWindow::hitboxExplaination(){
  return tr("<i><br/><br/>(info: skeleton metadata, including hitboxes, are kept in the separate file \"data/skeleton_bodies.xml\")</i>");
}


void MainWindow::aboutCheckboard(){
  int t = glWidget->lastMatErr.type;
  QString mot;
  QString longmot;
  QString cure;
  //bool ingame;
  switch (t){
  case 1:
    mot = QString(tr("I don't know what the material <i>\"%1\"</i> is.")).arg(glWidget->lastMatErr.matName);
    longmot = QString(tr("I've scanned in all file \"%1\" and didn't find a "
      "<i>load_mod_resource</i> or <i>load_resource</i> command that pointed me to a brf file "
      "that contained any such material")).arg(mabPath+"/Modules/"+modName+"/module.ini");
    cure = QString(tr("<br>- double check material name of the mesh<br>"
                   "<b>or</b><br>"
                   "- find the brf-file with the material, or create one<br>"
                   "- add a line <i>load_<b>mod</b>_resource</i> in module.ini, with a text editor,<br>"
                   "- (note the <i>mod</i> part)!<br>"
                   "- save module.ini<br>"
                   "- come back, and refresh Module [f5]"));
    //ingame=true;
    break;
  case 2:
    mot = QString(tr("I cannot find the file \"%1\" on disk.")).arg(glWidget->lastMatErr.texName);
    longmot = QString(
      tr("I've looked in folders <br>%1<br> and <br>%2<br> and <br>%3<br> but it wasn't there...<br>"
         "Maybe it is was tiff texture? (I don't understand them).")
      ).arg(
        glWidget->texturePath[0]).arg(glWidget->texturePath[1]).arg(glWidget->texturePath[2]
      );
    cure = QString(tr("<br>- double check DiffuesA texture name of the material<br>"
                   "- (hint: remember you can navigate with ctrl-left/right)<br>"
									 "<b>or</b><br>"
                   "<br>- make sure the missing texture file in mod texture folder!"
                   "<br>- put it there if it is missing")
                   );
    //ingame=true;
    break;
  case 3:
    mot = QString(tr("I cannot understand the texture format of  file \"%1\".")).arg(glWidget->lastMatErr.texName);
    longmot = QString(tr("I'm supposed to understand .dds textures of formats DXT1 (maybe), DXT3, and DXT5.<br>"
                      "But some kinds of DXT1 texture confuse me, and too big textures too.<br>Also, if graphic drivers are not up to date, I might ignore how to intepret DXT formats."
                      ));
    cure = QString(tr("Try updating the drivers. Else, maybe just accept the fact... it should still show the texture in game."));
    //ingame=false;
  }
  //if (t!=0)
	QMessageBox::about(this, "OpenBrf",
     QString(tr("<i>I could not display the real texture because:</i><br><b>%1</b><br><br>%2<br><br><b>Cure: </b>%3")).arg(mot).arg(longmot).arg(cure));
}




