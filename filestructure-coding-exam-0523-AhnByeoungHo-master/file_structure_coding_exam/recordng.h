// recordng.h

#ifndef RECORDING_H
#define RECORDING_H

#include <iostream>
#include "iobuffer.h"

using namespace std;
// class person with fixed field buffer

class Recording
// a recording with a composite key
{public:
	Recording ();
	Recording (const char * label, const char * idNum, const char * title, 
		const char * composer, const char * artist);
	char IdNum[7]; char Title [30]; char Composer[30];
	char Artist[30]; char Label[7];
	char * Key () const; // return key of object
	int Unpack (IOBuffer &);
	int Pack (IOBuffer &) const;
	void Print (ostream &, char * label = 0) const;
	friend ostream & operator << (ostream &, Recording &);
};

ostream & operator << (ostream & os , Recording & r) {
	os << r.Label << " " << r.IdNum << " " << r.Title << " " << r.Composer << " " << r.Artist << endl;
}
inline ostream & operator << (ostream & stream, Recording & rec)
{	rec.Print(stream); return stream;}

#endif
