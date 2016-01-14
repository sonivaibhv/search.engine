#ifndef __FILE_H__
#define __FILE_H__

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "word.h"
#include "trie.h"

using namespace std;

extern int yylex();

void makelower(char * s)
{
	for(int i=0; s[i]!='\0'; i++)
	{
		if(s[i] >= 'A' && s[i] <= 'Z')
		{
			s[i] = s[i] - 'A' + 'a';
		}
	}
}

int f_num = 0;
int wpos = 0;
Trie * t;
int add_word(char * word)
{
	if(strlen(word) > 40 || strlen(word) <= 2)
		return 0;
	
	makelower(word);
	Word * w = t->find(word);
	if(w == NULL)
	{
		Word w1(word);
		w = t->insert(word, w1);
	}
	w->add_file(f_num);
	w->add_pos(f_num, wpos);
	wpos++;
}

class File
{
	static int next_num;
	static fstream dbIndexFile;
	static fstream dbFile;
	
	public:
	
	static void parse(string p)
	{
		if(!dbIndexFile.is_open() || !dbFile.is_open())
		{
			cerr << "File Index files not open" << endl ;
			exit(0);
		}
		
		f_num = next_num;
		wpos = 0;
		extern FILE * yyin;
		FILE * file = fopen(p.c_str(), "r");
		if(file != NULL)
		{
			yyin = file;
			yylex();
			
			dbIndexFile << itoa(next_num) << " " << itoa(dbFile.tellp()) << endl;
//			cout << next_num << " " << p << endl;
			next_num++;
			dbFile << p << endl;
		}
		
/*		ifstream fin(p.c_str());
		if(!fin.is_open())
		{
			cout << "Error: Could not open file " << p <<" for reading\n" ;
			return;
		}
		
		while(fin)
		{
			string line;
			getline(fin, line);
			while(line != "")
			{
				size_t pos = line.find_first_of(delimiters);
				string word;
				if(pos != line.npos)
				{
					word = line.substr(0, pos);
					line = line.substr(pos+1);
				}
				else
				{
					word = line;
					line = "";
				}
				if(word != "")
				{
					// to lower
					Word * w = t.find(word.c_str());
					if(w == NULL)
					{
						w = new Word(word);
						cout << "Inserting into trie: " << word.c_str();
						t.insert(word.c_str(), w);
					}
					cout << "Add position " << wpos << endl;
					w->add_file(num);
					w->add_pos(num, wpos);
					wpos++;
				}
			}
		}
*/	}
	
	static int init(string index_fn, string fn)
	{
		dbIndexFile.open(index_fn.c_str(), ios::out | ios::in | ios::binary);
		
		if(!(dbIndexFile.is_open()))
		{
			cerr << "Error: Could not open File Number Index File\n";
			exit(1);
		}
		
		dbFile.open(fn.c_str(), ios::out | ios::in | ios::binary);
		if(!(dbFile.is_open()))
		{
			cerr << "Error: Could not open File Paths File\n";
			dbIndexFile.close();
			exit(1);
		}
		
		dbIndexFile.seekg(0, ios::end);
		if((int)dbIndexFile.tellg() == 0)
		{
			next_num = 1;
			dbIndexFile << itoa(next_num) << endl;
		}
		else
		{
			dbIndexFile.seekg(0);
			void * v;
			dbIndexFile >> v;
			next_num = (int)v;
		}
	}
	
	static int uninit()
	{
		if(dbIndexFile.is_open())
		{
			dbIndexFile.seekp(0);
			dbIndexFile << itoa(next_num) << endl;
			dbIndexFile.close();
		}
		if(File::dbFile.is_open())
			File::dbFile.close();
	}
	
	static string resolvePath(int num)
	{
		if(!dbIndexFile.is_open() || !dbFile.is_open())
		{
			cerr << "Error: File index files are not open\n";
			exit(1);
		}
		dbIndexFile.seekg(0, ios::beg);
		dbFile.seekg(0, ios::beg);
		int n;
		void * v;
		dbIndexFile >> v;
		n = (int)v;
		while(dbIndexFile)
		{
			int offset;
			void * v;
			dbIndexFile >> v;
			n = (int)v;
			dbIndexFile >> v;
			offset = (int)v;
			if(n == num)
			{
				dbFile.seekg(offset);
				string path;
				getline(dbFile, path);
				return path;
			}
		}
		return "";
	}
};

fstream File::dbIndexFile;
fstream File::dbFile;
int File::next_num;

#endif
