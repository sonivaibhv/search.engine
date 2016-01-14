#ifndef __TRIE_H__
#define __TRIE_H__

#include <cstring>
#include <iostream>
#include <fstream>
#include "itoa.h"
#include "word.h"

#define ALPHABET_SIZE	40

using namespace std;

class Trie
{
	public:
	
	class Node
	{
		class Pointer
		{
			public:
			bool file;
			union
			{
				int offset;
				Node * node;
			}p;
			Pointer()
			{
				file = false;
				p.node = false;
			}
			~Pointer()
			{
				if(file == false && p.node != NULL)
					delete p.node;
			}
		};
		
		static int count;
		
		char c;
		Word * data;
		int data_offset;
	//	int data_size;
		Pointer * arr;
		bool modified;
		int offset;
		
		public:
		
		void makeDirty()
		{
			modified = true;
		}
		Node(int offset = -1)
		{
			c = '!';
			data = NULL;
			data_offset = -1;
			arr = new Pointer[count];
			modified = false;
			this->offset = offset;
		}
		~Node()
		{
			if(arr != NULL)
				delete [] arr;
			
			if(data != NULL)
				delete data;
		}
		Word * insert(const char * key, Word & data, fstream & dbIndexFile, fstream & dbFile)
		{
			if(key[0] == '\0')
			{
				this->data = new Word(data);
				this->modified = true;
				return this->data;
			}
			else
			{
				Node * child = this->findChild(key[0], dbIndexFile, dbFile);
				if(child == NULL)
				{
					child = new Node();
					this->insertChild(key[0], child, dbIndexFile, dbFile);
					child->c = key[0];
				}
				return child->insert(key + 1, data, dbIndexFile,  dbFile);
			}
		}
		Node * findNode(const char * key, fstream & dbIndexFile, fstream & dbFile)
		{
			int i = 0;
			Node * curr = this;
			int key_len = strlen(key);
			for(i=0; i<key_len; i++)
			{
				curr = curr->findChild(key[i], dbIndexFile, dbFile);
				if(curr == NULL)
					return NULL;
			}
			return curr;
		}
		Word * find(const char * key, fstream & dbIndexFile, fstream & dbFile)
		{
			Node * node = findNode(key, dbIndexFile, dbFile);
			if(node != NULL)
			{
				node->loadData(dbIndexFile, dbFile);
				return node->data;
			}
			else
				return NULL;
		}
		Node * findChild(char c, fstream & dbIndexFile, fstream & dbFile)
		{
			if(this->arr[getNum(c)].file == false)
				return this->arr[getNum(c)].p.node;
			else if(this->arr[getNum(c)].p.offset != -1)
			{
				dbIndexFile.seekg(this->arr[getNum(c)].p.offset);
				Node * child = new Node();
				dbIndexFile >> *child;
				this->insertChild(c, child, dbIndexFile, dbFile);
				child->c = c;
				return child;
			}
			return NULL;
		}
		void insertChild(char c, Node * child, fstream & dbIndexFile, fstream & dbFile)
		{
			this->arr[getNum(c)].p.node = child;
			this->arr[getNum(c)].file = false;
			this->modified = true;
		}
		int getNum(char c)
		{
			int i;
		       	if(c >= 'a' && c <= 'z')
				i = (c-'a');
			else if(c >= '0' && c <= '9')
				i = ('z'-'a') + 1 + (c-'0');
			else
				i = 39;
			return i;
		}
		char getChar(int i)
		{
			char c;
			if(i >= 0 && i <= 25)
				c = 'a'+i;
			else
				c = '0' + (i-26);
			return c;
		}
		void save(fstream & dbIndexFile, fstream & dbFile)
		{
			for(int i = 0 ; i < ALPHABET_SIZE ; i++)
			{
				if(this->arr[i].file == false && this->arr[i].p.node != NULL)
					this->arr[i].p.node->save(dbIndexFile, dbFile) ;
			}
			if(this->modified == true)
			{
				if(this->data != NULL)
				{
					if(this->data_offset == -1)
					{
						dbFile.seekp(0, ios::end);
						this->data_offset = dbFile.tellp();
					}
					else
					{
						dbFile.seekp(this->data_offset);
					}
					dbFile << *(this->data) << endl;
				}
				
				if(this->offset == -1)
				{
					dbIndexFile.seekp(0, ios::end) ;
				}
				else
				{
					dbIndexFile.seekp(this->offset);
				}
				dbIndexFile << *this << endl ;
				this->modified = false ;
			}
		}
		void load(fstream & dbIndexFile, fstream & dbFile)
		{
			dbIndexFile >> *this ;
		}
		
		void loadData(fstream & dbIndexFile, fstream & dbFile)
		{
			if(this->data == NULL && this->data_offset != -1)
			{
				this->data = new Word();
				dbFile.seekg(this->data_offset);
				dbFile >> (*(this->data));
			}
		}
		
		void traverse(void (*fn)(Word *), fstream & dbIndexFile, fstream & dbFile)
		{
			char i;
			for(i='a'; i <= 'z'; i++)
			{
				Node * node = findChild(i, dbIndexFile, dbFile);
				if(node != NULL)
					node->traverse(fn, dbIndexFile, dbFile);
			}
			for(i='0'; i <= '9'; i++)
			{
				Node * node = findChild(i, dbIndexFile, dbFile);
				if(node != NULL)
					node->traverse(fn, dbIndexFile, dbFile);
			}
			this->loadData(dbIndexFile, dbFile);
			if(this->data != NULL)
			{
				fn(this->data);
			}
		}
		
		friend ostream & operator << (ostream & out, Node & node);
		friend istream & operator >> (istream & in, Node & node);
	} * root;
	fstream dbIndexFile, dbFile;

public:
	Trie(string index_fn, string fn)
	{
		dbIndexFile.open(index_fn.c_str() , ios::out | ios::in | ios::binary);
		if(dbIndexFile.is_open() == 0)
		{
			cerr << "Error: Could not open dbIndexFile\n";
			exit(1);
		}
		
		dbFile.open(fn.c_str() , ios::out | ios::in | ios::binary);
		if(dbFile.is_open() == 0)
		{
			dbIndexFile.close();
			cerr << "Error: Could not open dbFile\n";
			exit(1);
		}
		
		root = new Node(0);
		
		dbIndexFile.seekp(0, ios::end);
		if((int)dbIndexFile.tellp() == 0)
		{
			dbIndexFile << *root;
		}
		else
		{
			dbIndexFile.seekg(0);
			dbIndexFile >> *root;
		}
	}
	~Trie()
	{
		delete root;
	}
	Word * insert(const char * key, Word & data)
	{
		return root->insert(key, data, dbIndexFile, dbFile);
	}
	Word * find(const char * key)
	{
		return root->find(key, dbIndexFile, dbFile);
	}
	void save()
	{
		root->save(dbIndexFile, dbFile);
	}
	void traverse(const char * key, void (*fn)(Word *))
	{
		Node * node = root->findNode(key, dbIndexFile, dbFile);
		if(node != NULL)
			node->traverse(fn, dbIndexFile, dbFile);
	}
};

int Trie::Node::count = ALPHABET_SIZE;

ostream & operator << (ostream & out, Trie::Node & node)
{
	node.offset = out.tellp();
	
	out << itoa(node.data_offset) << endl;
	
	for(int i = 0 ; i < ALPHABET_SIZE ; i++)
	{
		int offset = -1 ;
		if(node.arr[i].file == true)
			offset = node.arr[i].p.offset ;
		else if(node.arr[i].p.node != NULL)
			offset = node.arr[i].p.node->offset ;
		out << itoa(offset) << " " ;
	}
	
	return out ;
}

istream & operator >> (istream & in, Trie::Node & node)
{
	node.offset = in.tellg();
	
	void * v;
	in >> v;
	node.data_offset = (int)v;
	
	for(int i = 0; i < ALPHABET_SIZE ; i++)
	{
		node.arr[i].file = true ;
		in >> v;
		node.arr[i].p.offset = (int)v;
	}
	
	return in ;
}
/*
int main(int argc, char **argv)
{	
	Trie t("./1.dat", "./2.dat");
	
	int a = 20, b = 80;
	if(argc == 1)
	{
		Word * w = new Word("qwerty");
		w->add_file(7);
		w->add_pos(7, 10);
		w->add_pos(7, 11);
		w->add_file(8);
		w->add_pos(8, 12);
		w->add_pos(8, 13);
		w->add_pos(8, 14);
		t.insert("red", w);
		t.insert("yellow", w);
		t.insert("bma", w);
		t.insert("blue", w);
		t.insert("white", w);
		t.insert("purple", w);
		t.save();
	}
	else
	{
		char * word1 = argv[1] ;
		Word * w1 = t.find(word1) ;
		if(w1 != NULL)
			cout << "Found: " << word1 << " " << *w1 << endl ;
		else
			cout << "Not found: " <<  word1 << endl ;
	}
	return 0;
}
*/

#endif
