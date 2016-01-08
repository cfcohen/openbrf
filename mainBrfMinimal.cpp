
#include "brfData.h"

typedef unsigned int uint;

#include "polynomials/polyMinimizer.h"
#include "polynomials/vecOfPoly.h"


/* HACK: define declared functions which we don't really need (just to make compiler happy) */
unsigned int tuneColor(unsigned int col, int c, int h, int s, int b){ return col; }
void BrfBodyPart::MakeQuadDominant(){}
void setGotNormals(bool){}
void setGotMaterialName(bool){}
void setGotTexture(bool){}
void setGotColor(bool){}
/* HACK: end */


PolyMinimizer<2> energy;

void addFrameEnergy( const BrfMesh &m , const BrfFrame &target, const std::vector< Matrix44f > &mat ){
	int nvert = (int)m.frame[0].pos.size();
	for (int i=0, k=0; i<nvert; i++) { // i index of vertex

		VecOfPoly<3,0> targetPos = fromVec3( target.pos[i] );
		VecOfPoly<3,1> lbsPos;
		vcg::Point3f restPos = m.frame[0].pos[i];

		for (int j=0; j<(int)mat.size(); j++, k++) { // j index of bone, k index of variable
			VecOfPoly<3,0> bonePos = fromVec3( mat[j] * restPos );
			Polynomial<1> poly;
			poly.term(k) = 1.0;
			lbsPos +=  bonePos * poly;
		}

		VecOfPoly<3,1> diff = lbsPos; diff -= targetPos;
		//printf("%d linear term, const term = %f\n", diff.x().countTermsOfDeg(1), diff.x().term());

		//energy.addEnergyTerm( squaredDistance( lbsPos, targetPos ) );
		energy.addEnergyTerm( dot( diff, diff ) );
	}
}

void testFrame( BrfMesh &m , const BrfFrame &target, const std::vector< Matrix44f > &mat ){
	int nvert = (int)m.frame[0].pos.size();
	int nbone = (int)mat.size();
	for (int i=0, k=0; i<nvert; i++) { // i index of vertex

		VecOfPoly<3,0> targetPos = fromVec3( target.pos[i] );
		VecOfPoly<3,1> lbsPos;
		vcg::Point3f restPos = m.frame[0].pos[i];

		for (int j=0; j<nbone; j++, k++) { // j index of bone, k index of variable
			VecOfPoly<3,0> bonePos = fromVec3( mat[j] * restPos );
			Polynomial<1> poly;
			poly.term(k) = 1.0;
			lbsPos +=  bonePos * poly;
		}

		VecOfPoly<3,1> diff = lbsPos; diff -= targetPos;

		int j = m.rigging[i].boneIndex[0];

		m.frame[0].pos[i].X() = diff.x().term() + diff.x().term(i*nbone + j);
		m.frame[0].pos[i].Y() = diff.y().term() + diff.y().term(i*nbone + j);
		m.frame[0].pos[i].Z() = diff.z().term() + diff.z().term(i*nbone + j);

	}
}



void makeRiggingFromVariables(BrfMesh &m, std::vector<float> weights, int nbones){

	int nonZero = 0;for (int i=0; i<(int)weights.size(); i++ ) if (weights[i]!=0) nonZero++;
	printf("debug: %d non zero vars on %d\n", nonZero, weights.size());

	int nvert = m.frame[0].pos.size();
	m.rigging.resize( nvert );

	int stats[5] = {0,0,0,0,0};

	for (int i=0, k=0; i<nvert; i++, k+=nbones){
		// find 4 maximal weights
		int nDone = 0;
		for (int r=0; r<4; r++) {
			int maxj = 0;
			int maxki = 0;
			float maxv = 0;

			for (int j=0, ki=k; j<nbones; j++,ki++) {
				float val = fabs(weights[ki]);
				if (val>maxv) {
					maxv = val;
					maxki = ki;
					maxj = j;
				}
			}
			if (maxv>0.0001) {
				m.rigging[i].boneIndex[r] = maxj;
				m.rigging[i].boneWeight[r] = weights[ maxki ];
				weights[ maxki ] = 0;
				nDone ++;
			} else {
				m.rigging[i].boneIndex[r] = -1;
				m.rigging[i].boneWeight[r] = 0;
			}
		}

		stats[nDone]++;
	}

	printf("Done rigging!\n");
	for (int i=0; i<5; i++)
		if (stats[i]) printf("  assigned %d vertex to %d bones\n", stats[i], i);

}

void computeRigging(BrfMesh &m, const BrfMesh &v, const BrfAnimation &a, const BrfSkeleton &s){
	printf("Compute energy: " );
	energy.clear();
	int nbones = s.bone.size();

	energy.setNumberOfVars( m.frame[0].pos.size() * nbones );
	for (int i=0; i<(int)v.frame.size(); i++) {

		std::vector< Matrix44f > matr = s.GetBoneMatrices( a.frame[i] );
		//matr.resize(nbones);

		addFrameEnergy ( m , v.frame[i], matr );
		printf("*");

		//if (i==v.frame.size()-1) testFrame( m, v.frame[i], matr );


	}

	printf("\nComputed mega energy with %d variables (%d terms) \n",energy.numberOfVars(), energy.countTerms() );

	printf("Minimize energy: " );

	energy.updateDoubleDerivatives();



	std::vector<float> weights( energy.numberOfVars() , 0.0 );
	for (int i=0; i<10000; i++) {
		energy.optimizeStep( weights );
		if (i%1000==0) printf("*");
	}
	printf("\nDone energy minimization!\n" );

	makeRiggingFromVariables( m, weights, nbones );

	// minimize
}

int main(int argc, char** argv){
	BrfData d;
	FILE *f = fopen("release/testAni.brf","rb");
	d.Load(f);
	printf("Found %d meshes!\n",d.mesh.size());

	if (!d.mesh.size()) { printf("No mesh!!!\n"); return 0; }
	if (!d.animation.size()) { printf("No ani!!!\n"); return 0; }
	if (!d.skeleton.size()) { printf("No skel!!!\n"); return 0; }

	BrfMesh &riggedMesh (d.mesh[0]);
	BrfSkeleton &skeleton (d.skeleton[0]);
	BrfAnimation &skelAni (d.animation[1]);

	printf("Loaded rigged-mesh, skel, skelAni\n");

	printf("rigged mesh is called \"%s\",\n",riggedMesh.name);
	printf("and it %s rigged!\n", (riggedMesh.IsRigged())?"IS":"ISN'T" );

	//skelAni.ResampleOneEvery(2);
	printf("Downsampled animation to %d frames\n", skelAni.frame.size() );

	// make vertex ani
	BrfMesh vertexAni = riggedMesh;
	vertexAni.RiggedToVertexAni(skeleton, skelAni );
	vertexAni.DiscardRigging();
	printf("Produced vertex ani\n");

	BrfMesh reriggedMesh = riggedMesh;
	//reriggedMesh.DiscardRigging();

	printf("Computing rigging from vertex ani\n");
	computeRigging(reriggedMesh, vertexAni, skelAni, skeleton );

	// save result
	sprintf( vertexAni.name, "vertex_animation" );
	sprintf( riggedMesh.name, "rigged_mesh" );
	sprintf( reriggedMesh.name, "rerigged_mesh");

	BrfData output;
	output.mesh.push_back( vertexAni );
	output.mesh.push_back( riggedMesh );
	output.mesh.push_back( reriggedMesh );
	FILE *fo = fopen("release/outputAni.brf","wb");
	output.Save(fo);

	return 0;
}


