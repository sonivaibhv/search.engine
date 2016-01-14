#ifndef __LIB_LIST_H__
#define __LIB_LIST_H__

#include <ostream>

using namespace std;

template<class T>
class List;

template<class T>
class List
{
	class _Node
	{
		public:
		T* val;
		_Node *next;
		_Node()
		{
			next = NULL;
		}
		
		_Node(T * v)
		{
			val = v;
			next = NULL;
		}
		~_Node()
		{
//			if(val != NULL)
//				delete val;

			if(next != NULL)
				delete next;
		}
	}*first, *last;
	
	public:
	
	int count;
	typedef _Node Node;
	List()
	{
		first = last = NULL;
		count = 0;
	}
	~List()
	{
		if(first != NULL)
			delete first;
		last = NULL;
	}
	int insert(T * val)
	{
		Node* n = new Node(val);
		if(first == NULL)
		{
			first = n;
		}
		else
		{
			last->next = n;
		}
		last = n;
		count++;
		return 0;
	}
	
	Node* find(const T& val)
	{
		Node* n = first;
		while (n!=NULL)
		{
			if(*(n->val) == val)
				return n;
			n = n->next;
		}
		return NULL;
	}
	
	Node* getFirst()
	{
		return first;
	}
	
	void save(ostream & out)
	{
		out << itoa(count);
		Node *n = first;
		while(n != NULL)
		{
			out << endl << *(n->val);
			n = n->next;
		}
	}
	
	void load(istream & in)
	{
		count = 0;
		int num;
		void * v;
		in >> v;
		num = (int)v;
		while(num > 0)
		{
			T * t = new T();
			in >> *t;
			insert(t);
			num--;
		}
	}
	
	void sort()
	{
		Node * second = NULL;
		Node * sec_last = NULL;
		
		while(first != NULL)
		{
			Node * max = first;
			Node * p = first;
			while(p != NULL)
			{
				if(*(p->val) > *(max->val))
					max = p;
				p = p->next;
			}
			
			if(first == max)
				first = max->next;
			else
			{
				p = first;
				while(p != NULL && p->next != max)
					p = p->next;
				if(p != NULL)
					p->next = max->next;
			}
			max->next = NULL;
			
			if(second == NULL)
				second = max;
			else
				sec_last->next = max;
			sec_last = max;
		}
		first = second;
		last = sec_last;
	}
};

/*
template<class T>
ostream & operator << (ostream & out, List<T> & l)
{
	out << itoa(l.count);
	List<T>::Node *n = l.first;
	while(n != NULL)
	{
		out << endl << *(n->val);
		n = n->next;
	}
}

template<class T>
istream & operator << (istream & in, List<T> & l)
{
	in >> l.count;
	List<T>::Node *n = new List<T>::
}
*/
#endif
