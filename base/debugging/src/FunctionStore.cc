// ---------------------------------------------------------------------------------------------------
// File: FunctionStore.cc
// Date: 28/8/2003
// Part of StackedIt V2.0
// By: Joel Fenwick, Mi-Pal, Griffith University
//
// This program is copyrighted to the author and released under the GPL (V2.0).
// There is no warantee associated with this code. Especially not for use
// in nuclear plants, aircraft control systems or hedgehogs. Use at your
// own risk.
// See license.txt for details
// ----------------------------------------------------------------------------------------------------

#include "FunctionStore.h"

using namespace std;

FunctionStore::FunctionStore()
{
	fp=0;
}

FunctionStore::FunctionStore(istream& is)
{
	fp=0;
	char buffer[500];
	while (is.getline(buffer,500),is.good())
	{
		Process(buffer);
	}
}


FunctionStore::~FunctionStore()
{
	for (vector<Function*>::iterator i=fns.begin();i!=fns.end();++i)
	{
		delete *i;
	}
}

// process a line of dissassembly
void FunctionStore::Process(char* line)
{
	if (Function::IsHeader(line))
	{
		fp=new Function(line);
		fns.push_back(fp);	
	}
	else if (fp!=0)
	{
		if (!fp->Process(line))
		{
			// Not sure why the function would return false at this point
			// Maybe it shouldn't return anything???
		}
	}
}

// Return the function which contains the address
// This assumes that everything that appears between the heading of A and the heading of B
// belongs to A. This may not be a good assumption
Function* FunctionStore::ContainsAddress(unsigned int addr)
{
	int index=BinarySearch(0,fns.size()-1, addr);
	if (index==-1)
	{
		return 0;
	}
	return fns[index];
}

// find an index x such that val comes after [x] but before [x+1] if it
// exists (return -1 on failure)
// This function is a bit hard to read because 'start' refers to the start address of the
// function not the 'begin'ing of the search range
int FunctionStore::BinarySearch(int begin, int end, unsigned int val)
{
	if (begin>end)
	{
		return -1;
	}
	if (fns[begin]->start==val)
	{
		return begin;
	}
	if (begin==end)
	{
		return -1;		//since val!=[begin]
	}
	if (fns[begin]->start>val)
	{
		return -1;
	}
	if (fns[end]->start<=val)
	{
		return end;
	}
	if ((end-begin==1) && (fns[end]->start>val))
	{
		return begin;		
	}
	if (fns[(begin+end)/2]->start<val)
	{
		return BinarySearch((begin+end)/2,end,val);
	}
	else
	{
		return BinarySearch(begin,(begin+end)/2,val);
	}
}

// return the range of addresses covered by functions in this store
void FunctionStore::GetAddressRange(unsigned int& lower, unsigned int& upper)
{
	if (fns.size()==0)
	{
		lower=0;
		upper=0;
	}
	else
	{
		lower=fns[0]->start;
		upper=fns[fns.size()-1]->start;	// since the last function is .fini this is good enough
	}
}

// For debug only 
void FunctionStore::Dump(ostream& os)
{
	for (vector<Function*>::iterator i=fns.begin();i!=fns.end();++i)
	{
		os << hex <<(*i)->start << " " << (*i)->name << " " << (*i)->stacksize << endl;
	}
}
