

#ifndef LOCALMAP_H
#define LOCALMAP_H

#include "../platform.h"

//memory localizing hash map with changing hash function cipher key

#define LOWEST0(x)			(~x & (x + 1))
#define LOWEST1(x)			(x & (~x + 1))

#define HASHINT		uint32_t
//#define HASHINT	uint16_t
//#define HASHINT	uint8_t
#define HASHADDR	uint8_t
#define SHASHADDR	int8_t
//#define HASHADDR	uint32_t
//#define SHASHADDR	int32_t
//#define HASHFIELD	uint32_t
//#define HASHCOUNT	uint8_t
//#define LOG2HASHBITS	4	//(log2(HASHBITS))
//#define HASHBITS	21
#define HASHBITS	5
//#define HASHBITS	(1<<LOG2HASHBITS)
#define MAXHASH		((1<<HASHBITS)-1)
#define HASHCROP(x)	((x)&(MAXHASH))
//#define LOW1(x)			((x&(~x+1))&((1<<HASHBITS)-1))	//for extraneous bits
//#define HASHROTL(x,n)	((x<<n)|(x>>(HASHBITS-n)))
//#define HASHROTR(x,n)	((x>>n)|(x<<(HASHBITS-n)))
//for extraneous bits
//#define HASHVALS			(1)
//#define HASHVALS			(HASHBITS>>2)
#define HASHVALS				((1<<(HASHBITS)))
//#define HASHVALS			(HASHBITS-1)
#define HASHROTL(x,n)	HASHCROP((x<<n)|(x>>(HASHBITS-n)))
#define HASHROTR(x,n)	HASHCROP((x>>n)|(x<<(HASHBITS-n)))
//#define HASHLEVELS		HASHBITS
//#define HASHLEVELS		(HASHBITS*(1<<(LOG2HASHBITS*LOG2HASHBITS))/(HASHBITS*HASHBITS))
//#define HASHLEVELS		((1<<((LOG2HASHBITS-1)*(LOG2HASHBITS-1)))/HASHBITS+1)
//#define HASHLEVELS	(HASHBITS)	//2 works for 1 bits
//#define HASHLEVELS	(HASHBITS<<2)	//9 works for 3 bits
//#define HASHLEVELS	((HASHBITS<<1))	//9 works for 3 bits
//#define HASHLEVELS		((1<<HASHBITS)+1)
#define HASHLEVELS	(5)
//#define HASHLEVELS	((HASHBITS<<1)+4)
//#define HASHLEVELS	((HASHBITS*HASHVALS)+HASHVALS)	//works for 3 bits
//#define MAXJUMP			(1<<3)
//#define HASHLEVELS		(7)
//#define HASHLEVELS		(3+HASHBITS)	//log2(MAXJUMP)+HASHBITS
//#define HASHLEVELS	(HASHBITS<<4+1)	//works for 3 bits
//#define HASHLEVELS		(HASHBITS*HASHBITS)
//#define HASHLEVELS		(HASHBITS*HASHBITS*HASHBITS)
//#define HASHLEVELS		((1<<HASHBITS)+1)
//#define HASHLEVELS			((1<<(HASHBITS*HASHBITS))+1)

//if it's too slow, there's not enough levels

inline bool Ahead(HASHINT current[HASHLEVELS], HASHINT test[HASHLEVELS])
{
	for(SHASHADDR i=(HASHLEVELS-1); i>=0; --i)
	//for(uint8_t i=0; i&31; ++i)
	{	
		if(test[i] > current[i])
			return true;
		if(test[i] < current[i])
			return false;
	}

	return false;
}

bool Try12(HASHINT mval[HASHLEVELS], HASHINT in, HASHINT out);
void TestHash();

#endif