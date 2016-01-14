#ifndef __WORD_H__
#define __WORD_H__

#include <string>
#include "wordfile.h"
#include "lib/list.h"

using namespace std;

class Word
{
	string word;
	List<WordFile> files;

public:
	Word()
	{
		word = "";
	}
	Word(string word)
	{
		this->word = word;
	}
	~Word()
	{
		List<WordFile>::Node * wfn = files.getFirst();
		while(wfn != NULL)
		{
			if(wfn->val != NULL)
				delete wfn->val;
			wfn = wfn->next;
		}
	}
	
	void add_file(int fnum, int count = 0)
	{
		List<WordFile>::Node * wfn = files.find(WordFile(fnum));
		if(wfn == NULL)
		{
			WordFile * wf = new WordFile(fnum, count);
			files.insert(wf);
		}
	}
	
	void add_pos(int fnum, int pos)
	{
		List<WordFile>::Node * wfn = files.find(WordFile(fnum));
		if(wfn != NULL)
			wfn->val->add_pos(pos);
	}
	
	string getWord()
	{
		return word;
	}
	
	List<WordFile> & getFiles()
	{
		return files;
	}
/*	
	List<string>& getFilePaths()
	{
		files.sort();
		List<string> *paths = new List<string>();
		List<WordFile>::Node * wfn = files.getFirst();
		while(wfn != NULL)
		{
			string *s = new string( File::resolvePath(wfn->val->getNum()) );
			paths->insert(s);
			wfn = wfn->next;
		}
		return *paths;
	}
*/	
	friend Word * or_them(Word & w1, Word & w2);
	friend Word * and_them(Word & w1, Word & w2);
	friend Word * phrase_match(Word & w1, Word & w2);
	friend ostream & operator << (ostream & out, Word & w);
	friend istream & operator >> (istream & in, Word & w);
};

Word * or_them(Word & w1, Word & w2)
{
	Word * word = new Word(w1.word + " OR " + w2.word);
//	List<Word2File> *files = new List<Word2File>();
	List<WordFile>::Node *n1 = w1.files.getFirst();
	List<WordFile>::Node *n2 = w2.files.getFirst();
	
	while(n1 != NULL)
	{
		List<WordFile>::Node *n = w2.files.find(WordFile(n1->val->getNum()));
		int count = 0;
		if(n != NULL)
			count = n->val->getCount();
//		Word2File *wf = new Word2File(n1->val->getNum(), n1->val->getCount(), count);
//		files->insert(wf);
		word->add_file(n1->val->getNum(), n1->val->getCount() + count);
		n1 = n1->next;
	}
	
	while(n2 != NULL)
	{
		List<WordFile>::Node *n = w1.files.find(WordFile(n2->val->getNum()));
		if(n == NULL)
		{
//			Word2File *wf = new Word2File(n2->val->getNum(), 0, n2->val->getCount());
//			files->insert(wf);
			word->add_file(n2->val->getNum(), n2->val->getCount());
		}
		n2 = n2->next;
	}
	
	return word;
}

Word * and_them(Word & w1, Word & w2)
{
	Word * word = new Word(w1.word + " AND " + w2.word);
	List<WordFile>::Node *n1 = w1.files.getFirst();
	
	while(n1 != NULL)
	{
		List<WordFile>::Node *n = w2.files.find(WordFile(n1->val->getNum()));
		if(n!=NULL)
		{
			word->add_file(n1->val->getNum(), (n1->val->getCount() + n->val->getCount()));
		}
		n1 = n1->next;
	}
	return word;
}

Word * phrase_match(Word & w1, Word & w2)
{
	Word * word = new Word("\"" + w1.word + " " + w2.word + "\"");
	List<WordFile>::Node *n1 = w1.files.getFirst();
	
	while(n1 != NULL)
	{
		List<WordFile>::Node *n = w2.files.find(WordFile(n1->val->getNum()));
		if(n!=NULL)
		{
			List<Int>::Node *p= n1->val->getPositions().getFirst();
			while(p!=NULL)
			{
				List<Int>::Node *q= n->val->getPositions().getFirst();
				while(q!=NULL)
				{
					if(p->val->getVal()==(q->val->getVal()-1))
					{
						word->add_file(n1->val->getNum());
						word->add_pos(n1->val->getNum(), q->val->getVal());
					}
					q=q->next;
				}
				p=p->next;
			}
		}
		n1=n1->next;
	}
	return word;
}

ostream & operator << (ostream & out, Word & w)
{
	out << w.word;
	out << endl;
	w.files.save(out);
	return out;
}

istream & operator >> (istream & in, Word & w)
{
	getline(in, w.word);
	w.files.load(in);
	return in;
}

#endif
