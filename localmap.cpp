

#include "../utils.h"
#include "../debug.h"
#include "localmap.h"


void TestHash()
{
#if 1
	HASHINT m[HASHLEVELS];
	uint64_t t = GetTicks();

	//for(int times=0; times<50; ++times)
	{
		memset(m, 0, HASHLEVELS * sizeof(HASHINT));

		struct Map
		{
			HASHINT i;
			HASHINT o;
		};

		std::list<Map> map;

		uint32_t r = GetTicks();
		
		//for(HASHINT o=0; o<pow(2,HASHBITS)-1; ++o)
		//for(HASHINT o=0; o<pow(2,HASHBITS); ++o)
		//for(HASHINT o=0; o<pow(2,HASHBITS-2)-1; ++o)
		//for(HASHINT o=0; o<HASHBITS-1; ++o)
		for(HASHINT o=0; o<HASHVALS; ++o)
		{
			while(true)
			{
				r *= 0x12345;
				r += 98765;

				//HASHINT i = 1024-o;
				HASHINT i = (1024-o) ^ r;
				//uint32_t i = o;
				//uint32_t i = o+1;
				//HASHINT i = o * MAXJUMP;

				//i &= ((1<<1) | (1<<3) | (1<<4) | (1<<5));

				//i = (3-o)&(4-1);

				Map add;
				add.i = HASHCROP(i);
				add.o = HASHCROP(o);

				//if(o>1)
				//	add.o = HASHCROP( -1 );

				bool have = false;

				for(auto mit=map.begin(); mit!=map.end(); mit++)
				{
					if(mit->i != add.i)
						continue;

					have = true;
					break;
				}

				if(have)
					continue;

				map.push_back(add);
				break;
			}
		}

		auto mit = map.begin();

		while(mit != map.end())
		{
			HASHINT old[HASHLEVELS];
			memcpy(old, m, sizeof(HASHINT) * HASHLEVELS);

			if(Try12(m, mit->i, mit->o))
				;//InfoMess("s","s");
			else
			{
				//InfoMess("f","f");
				
				uint64_t p = GetTicks() - t;

				char ms[123];
				sprintf(ms, "fail %llu on %d", p, (int)mit->o);
				MessageBox(NULL, ms, ms, NULL);

				return;
			}

			//break;

			if((Ahead(old, m) || Ahead(m, old)) && mit != map.begin())
				mit = map.begin();
			else
				mit++;
		}
	}

	uint64_t p = GetTicks() - t;

	char ms[123];
	sprintf(ms, "pass %llu", p);
	MessageBox(NULL, ms, ms, NULL);
	//InfoMess(ms,ms);
#endif
}

#if 1

HASHINT Hash(HASHINT mval[HASHLEVELS], HASHINT oval, HASHADDR level)
{
	if(level >= HASHLEVELS)
		return oval;

	HASHINT shifto = HASHROTL(oval,1);
	const HASHINT ovalup = HASHCROP( shifto ^ ~(oval & mval[level]) );

	Log("level%d up%u", (int)level, ovalup);

	return Hash(mval, ovalup, level+1);
}

void Flush(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], SHASHADDR level)
{
	HASHINT oval = ovals[level];

	for(; level<HASHLEVELS; ++level)
	{
		const HASHINT shifto = HASHROTL(oval,1);
		oval = HASHCROP( shifto ^ ~(oval & mval[level]) );

		//Log("level %d", (int)level);
		//g_applog.flush();

		if(ovals[level+1] == oval)
			break;

		ovals[level+1] = oval;
	}
}

void FullFlush(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], SHASHADDR level)
{
	HASHINT oval = ovals[level];

	for(; level<HASHLEVELS; ++level)
	{
		const HASHINT shifto = HASHROTL(oval,1);
		oval = HASHCROP( shifto ^ ~(oval & mval[level]) );
		ovals[level+1] = oval;
	}
}

bool Check(HASHINT mval[HASHLEVELS], HASHINT in, HASHINT out, HASHADDR checkbin)
{
	HASHINT oval = in;

	for(HASHADDR i=0; i<HASHLEVELS; ++i)
	{
		HASHINT shifto = HASHROTL(oval,1);
		oval = HASHCROP( shifto ^ ~(oval & mval[i]) );
	}

	HASHINT checkbm = 1 << checkbin;

	return ((oval & checkbm) == (out & checkbm));
}

bool UpBuffer(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], HASHADDR checkbit, HASHADDR bin)
{
	HASHADDR decline = bin - checkbit;

	if(bin < checkbit)
		decline = ((HASHBITS-1)-checkbit)+bin;
	
	SHASHADDR height = (HASHLEVELS-1) - decline;

	height = imax(0, height);

	HASHINT bm = 1 << bin;

	for(SHASHADDR l=(HASHLEVELS-1); l>=height; --l)
	{
		if(mval[l] & bm)
			continue;

		mval[l] |= bm;

		for(HASHADDR l2=l+1; l2<HASHLEVELS; ++l2)
			mval[l2] &= HASHCROP( ~bm );

		Flush(mval, ovals, l);

		return true;
	}

	//InfoMess("!upb","!upb");

	return false;
}

//increment side until change happens effect
//stay in column
bool UpSide2(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], HASHINT out, HASHADDR checkbit, HASHADDR bin)
{
	HASHADDR decline = bin - checkbit;

	if(bin < checkbit)
		decline = ((HASHBITS-1)-checkbit)+bin;
	
	SHASHADDR height = (HASHLEVELS-1) - decline;

	height = imax(0, height);

	HASHINT bm = 1 << bin;

	for(SHASHADDR l=height; l>=0; --l)
	{
		if(mval[l] & bm)
		{
			mval[l] &= HASHCROP( ~bm );
			Flush(mval, ovals, l);
			continue;
		}

		mval[l] |= bm;

		HASHINT oval = ovals[l];
		HASHINT shifto = HASHROTL(oval,1);
		oval = shifto ^ ~(oval & mval[l]);

		HASHADDR stretch = (HASHLEVELS-1) - l;
		HASHINT effect = 0;
		effect |= ((1<<(checkbit+stretch+1))-1);
		effect &= ~((1<<checkbit)-1);
		effect |= ((1<<(((checkbit+stretch)&(HASHBITS-1))+1))-1);
		effect = HASHCROP(effect);

		if( (oval & effect) != (ovals[l+1] & effect) )
		{
			Flush(mval, ovals, l);
			return true;
		}

		mval[l] &= HASHCROP( ~bm );
	}

	//InfoMess("!ups","!ups");

	Flush(mval, ovals, 0);
	return false;
}

//reset unused 'buffer' bits
void ResetBuffer(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], HASHINT out, HASHADDR checkbit, HASHADDR bin)
{
	HASHINT bm = 1 << bin;

	HASHADDR decline = bin - checkbit;

	if(bin < checkbit)
		decline = ((HASHBITS-1)-checkbit)+bin;

	SHASHADDR height = (HASHLEVELS-1) - decline;

	height = imax(0, height);

	//Log("hl%d,   h %d    d %d     b%d    c%d", (int)HASHLEVELS, (int)height, (int)decline, (int)bin, (int)checkbit);
	//g_applog.flush();

	for(SHASHADDR l=(HASHLEVELS-1); l>height; --l)
		mval[l] &= HASHCROP( ~bm );

	Flush(mval, ovals, height);
}

bool MatchBit2(HASHINT mval[HASHLEVELS], HASHINT ovals[HASHLEVELS+1], HASHINT out, HASHADDR checkbit)
{
	uint64_t full = 0;
	HASHINT cbm = 1 << checkbit;

	HASHADDR bin=0;
	ResetBuffer(mval, ovals, out, checkbit, bin);

	//send pulse
	while(true)
	{
		HASHINT bm = 1 << bin;

		if(UpSide2(mval, ovals, out, checkbit, bin))
		{
			if( (ovals[HASHLEVELS] & cbm) == (out & cbm) )
				return true;
			
			bin = 0;

			continue;
		}

		
		//if(GetTicks() % 1000 == 0)
		if(0)
		{
			for(SHASHADDR x=0; x<HASHLEVELS; ++x)
			{
				Log("\t mask[%d] = %u", (int)x, mval[x]);
			}

			Log("matching bit%d %u-%u=%u", (int)checkbit, ovals[HASHLEVELS], out, ovals[HASHLEVELS]-out);
		}

		if(bin+1 >= HASHBITS)
		{
			//MessageBox(NULL, "!>hb", "!>hb", NULL);

#if 0
			for(SHASHADDR x=0; x<HASHLEVELS; ++x)
			{
				Log("\t >mask[%d] = %u", (int)x, mval[x]);
			}

			Log("no bit%d %u-%u=%u", (int)checkbit, ovals[HASHLEVELS], out, ovals[HASHLEVELS]-out);
#endif

			return false;
		}

		if( !(full & bm) && UpBuffer(mval, ovals, checkbit, bin+1) )
		{
			full |= bm;
			Flush(mval, ovals, 0);
			bin = 0;
			continue;
		}

		++bin;

		ResetBuffer(mval, ovals, out, checkbit, bin);
	}
}

//opt 7
bool Try12(HASHINT mval[HASHLEVELS], HASHINT in, HASHINT out)
{
	HASHINT ovals[HASHLEVELS+1];
	ovals[0] = in;
	HASHINT oval = ovals[0];

	for(HASHADDR l=0; l<HASHLEVELS; ++l)
	{
		HASHINT shifto = HASHROTL(oval,1);
		oval = HASHCROP( shifto ^ ~(oval & mval[l]) );
		ovals[l+1] = oval;
	}

	//Log("start");

#if 1
	for(SHASHADDR bin=(HASHBITS-1); bin>=0; --bin)
	{
		//Log("bin%d", (int)bin);

		HASHINT bm = 1 << bin;

		if( (ovals[HASHLEVELS] & bm) == (out & bm) )
		{
			//Log("skip match %u-%u=%u", ovals[32], out, ovals[32]-out);
			continue;
		}

		if(!MatchBit2(mval, ovals, out, bin))
		{
			//Log("matched bit%d %u-%u=%u", (int)bin, ovals[32], out, ovals[32]-out);
			return false;
		}

		//if(GetTicks() % 1000 == 0)
		if(0)
		{
			for(SHASHADDR x=0; x<HASHLEVELS; ++x)
			{
				Log("\t mask[%d] = %u", (int)x, mval[x]);
			}

			Log("matched bit%d %u-%u=%u", (int)bin, ovals[HASHLEVELS], out, ovals[HASHLEVELS]-out);
		}

		bin = (HASHBITS-1);
	}
#else

	for(HASHADDR bin=0; bin<HASHBITS; ++bin)
	{
		//Log("bin%d", (int)bin);

		HASHINT bm = 1 << bin;

		if( (ovals[HASHLEVELS] & bm) == (out & bm) )
		{
			//Log("match %u-%u=%u", ovals[32], out, ovals[32]-out);
			continue;
		}

		if(!MatchBit2(mval, ovals, out, bin))
			return false;

		//Log("matched bit%d %u-%u=%u", (int)bin, ovals[32], out, ovals[32]-out);

		bin = 0;
	}
#endif

	//Log("fin");

	return true;
}

#endif