#ifndef CARRYPOSITION_H
#define CARRYPOSITION_H

class CarryPosition{
public:
	char name[255];
	char boneName[255];
	vcg::Matrix44f matPre, matPost;
	bool needExtraTrasl;

	bool Load(const char* line);
};


#endif // CARRYPOSITION_H
