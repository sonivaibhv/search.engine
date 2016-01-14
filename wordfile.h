#ifndef __WORD_FILE_H__
#define __WORD_FILE_H__

#include <string>
#include "itoa.h"
#include "lib/list.h"

using namespace std;

class Int
{
	int n;
	public:
	Int()
	{
	}
	Int(int n)
	{
		this->n = n;
	}
	int getVal()
	{
		return n;
	}
	friend ostream & operator << (ostream & out, Int & i);
	friend istream & operator >> (istream & in, Int & i);
};

ostream & operator << (ostream & out, Int & i)
{
	out << itoa(i.n);
	return out;
}

istream & operator >> (istream & in, Int & i)
{
	void * v;
	in >> v;
	i.n = (int)v;
	return in;
}

class WordFile
{
	int fnum;
	int count;
	List<Int> positions;
	
	public:
	WordFile(int fnum = 0, int count = 0)
	{
		this->fnum = fnum;
		this->count = count;
	}
	~WordFile()
	{
		List<Int>::Node * pi = positions.getFirst();
		while(pi != NULL)
		{
			if(pi->val != NULL)
				delete pi->val;
			pi = pi->next;
		}
	}
/*	WordFile(int fnum, List<int> positions, int count = 0)
	{
		this->fnum = fnum;
		this->positions = positions;
		this->count = count;
	}
	
	int getNum()
	{
		return fnum;
	}
*/	
	int getCount()
	{
		if(count != 0)
			return count;
		return positions.count;
	}
	
	List<Int>& getPositions()
	{
		return positions;
	}
	
	void add_pos(int pos)
	{
		Int * i = new Int(pos);
		positions.insert(i);
	}
	
	int getNum()
	{
		return fnum;
	}

	bool operator > (const WordFile& wf)
	{
		int count1 = 0, count2 = 0;
		
		if(this->count == 0)
			count1 = this->positions.count;
		else
			count1 = this->count;

		if(wf.count == 0)
			count2 = wf.positions.count;
		else
			count2 = wf.count;
		
		if(count1 > count2)
			return true;
		return false;
	}
	
	bool operator == (const WordFile & wf)
	{
		if(fnum == wf.fnum)
			return true;
		return false;
	}
	
	friend istream & operator >> (istream & in, WordFile & wf);
	friend ostream & operator << (ostream & out, WordFile & wf);
};

istream & operator >> (istream & in, WordFile & wf)
{
	void * v;
	in >> v;
	wf.fnum = (int)v;
	wf.positions.load(in);
	return in;
}

ostream & operator << (ostream & out, WordFile & wf)
{
	out << itoa(wf.fnum);
	out << endl;
	wf.positions.save(out);
/*	List<int>::Node * pn = wf.positions.getFirst();
	out << wf.positions.count << " ";
	while(pn != NULL)
	{
		out << *(pn->val) << " ";
		cout << *(pn->val) << " ";
		pn = pn->next;
	}
	cout << endl;
*/	return out;
}

#endif
