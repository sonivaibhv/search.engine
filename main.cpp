#include <iostream>
#include <string>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "lib/list.h"
#include "file.h"
#include "word.h"
#include "trie.h"
#include "lib/getopt.h"

extern int yyparse();
extern void makelower(char *);

using namespace std;

extern Trie * t;

bool is_valid(string & path)
{
	size_t pos = path.find_first_not_of("./");
	if(pos != string::npos && pos > 0)
	{
		string p = path.substr(pos);
		if(path.compare(pos-1, 4, "/dev") == 0 ||
				path.compare(pos-1, 4, "/bin") == 0 ||
				path.compare(pos-1, 8, "/usr/bin") == 0 ||
				path.compare(pos-1, 4, "/usr/local/bin") == 0 ||
				path.compare(pos-1, 4, "/sbin") == 0 ||
				path.compare(pos-1, 4, "/usr/X11R6/bin/") == 0 ||
				path.compare(pos-1, 4, "/usr/local/sbin") == 0 ||
				path.compare(pos-1, 4, "/proc") == 0)
		{
			return false;
		}
	}
	return true;
}

int file_count = 0;

void traverse(string & fn)
{
	DIR *dir;
	struct dirent *entry;
	string path;
	struct stat info;
	
	if( ! is_valid(fn))
		return;
	
	
	if ((dir = opendir(fn.c_str())) == NULL)
	{
		return;
	}
	
	while ((entry = readdir(dir)) != NULL) 
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			path = fn;
			if(path.find_last_of("/") != path.size()-1)
				path = path + "/";
			path = path + entry->d_name;
			if(stat(path.c_str(), &info) != 0 || S_ISLNK(info.st_mode))
				continue;
			if (S_ISDIR(info.st_mode))
				traverse(path);
			else
			{
				size_t pos = path.find_last_of(".");
				if(pos != string::npos && (path.substr(pos) == ".txt" || path.substr(pos) == ".htm" || path.substr(pos) == ".html"))
				{
					file_count++;
					File::parse(path);
				}
			}
		}
	}
	closedir(dir);
}

bool give_count = false;

void print(Word & w)
{
	cerr << "Searched for: " << w.getWord() << endl;
	List<WordFile> & wf = w.getFiles();
	wf.sort();
	List<WordFile>::Node * wfn = wf.getFirst();
	while(wfn != NULL)
	{
		int fnum = wfn->val->getNum();
		if(give_count)
			cout << "[" << wfn->val->getCount() << "] ";
		cout << File::resolvePath(fnum) << endl;
		wfn = wfn->next;
	}
}

Word * sres = NULL;

void stem_fn(Word * w)
{
	if(sres != NULL)
	{
		if(w != NULL)
			sres = or_them(*sres, *w);
	}
	else
		sres = w;
}

Word * s(string & line, Trie & t)
{
	Word * res = NULL;
	while(line != "")
	{
		size_t pos = line.find_first_of(" \t");
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
			Word * w1 = NULL;
			if(word.find_last_of("*") != string::npos)
			{
				word = word.substr(0, word.size()-1);
				sres = NULL;
				t.traverse((char*)word.c_str(), stem_fn);
				w1 = sres;
				sres = NULL;
			}
			else
				w1 = t.find(word.c_str());
			
			if(w1 == NULL)
				w1 = new Word(word);
			
			if(res != NULL)
			{
				if(w1 == NULL)
					res = NULL;
				else
					res = phrase_match(*res, *w1);
			}
			else
				res = w1;
		}
	}
	return res;
}

int main(int argc, char **argv)
{
	int errflag, option;
	string f1;
	string f2;
	string f3;
	string f4;
	string dir = "";
	bool search = true;
	char *home;
	string mss_dir;
	struct stat mss_stat;
	string query_string = "";
	string prev = "";
	errflag=0;
	string index_name = "";
	string query[10];
	int qnum = 0;
	
	while(((option = my_getopt(argc,argv,"hnc:i:")) != NONOPT) || optarg != NULL)
	{
		switch(option)
		{
			case 'i':
				index_name = optarg;
				cerr <<"Using non-default index: " << optarg <<endl;
				break;
			case 'c':
				cerr << "Indexing directory: " << optarg << endl;
				search = false;
				dir = optarg;
				break;
			case 'n':
				give_count = true;
				break;
			case 'h':
				cerr << argv[0] << ": Index and search for words in files. Version 1.0" << endl << endl
					<< "usage: "
					<< argv[0] << " [-i index-name] -c directory     \t\tcreate index for 'directory'; use index 'index-name'" << endl
					<< "   or: "
					<< argv[0] << " [-i index-name] [-n] query-string\t\tsearch for 'query-string'; use index 'index-name'" << endl << endl
					<< "Arguments:" <<endl
					<< "  -c (with directory)\t create index" << endl
					<< "  -i (with name)\t use non-default index" << endl
					<< "  -n\t\t\t display number of occurrances in search results" << endl
					<< endl
					<< "EXAMPLES:" << endl
					<< "To index '/usr/share/doc', use" << endl
					<< "\t" << argv[0] << " -c /usr/share/doc" << endl << endl
					<< "To index '/home/tom' with index-name 'home', use" << endl
					<< "\t" << argv[0] << " -c /home/tom -i home" << endl << endl
					<< "Now, to search '/usr/share/doc' for 'linux AND perl', use" << endl
					<< "\t" << argv[0] << " linux AND perl" << endl << endl
					<< "Now, to search '/home/tom' for 'perl OR php', use" << endl
					<< "\t" << argv[0] << " perl OR php -i 'home'" << endl << endl
					<< "To search '/usr/share/doc' for the phrase \"apache php module\", use" << endl
					<< "\t" << argv[0] << " \"apache php module\"" << endl << endl
					<< "To search '/usr/share/doc' for 'lin*', use (observe the single quotes)" << endl
					<< "\t" << argv[0] << " 'lin*'" << endl;
				exit(0);
				break;
			case '?':
				cerr << "Type " << argv[0] << " -h for help" << endl;
				exit(1);
				break;
			case NONOPT:
//				cout<< "Word " << optarg <<endl;
				makelower(optarg);
				if(prev != "")
				{
					query_string = query_string + " ";
					if(prev != "and" && prev != "or" && strcmp(optarg, "and") != 0 && strcmp(optarg, "or") != 0)
					{
						query_string = query_string + "and ";
						query[qnum] = "and";
						qnum++;
					}
				}
				string q = "";
				if(strcmp(optarg, "and") != 0 && strcmp(optarg, "or") != 0)
				{
					string s = optarg;
					size_t p = s.find_first_of(" \t");
					if(s.find_first_of(" \t") != string::npos && s.find_first_of("*") != string::npos)
					{
						cerr << "Ignoring phrase (wildcards used): \"" << optarg << "\"" << endl;
						continue;
					}
					if(s.find_first_of("*") < s.size()-1)
					{
						cerr << "Ignoring word (wildcard '*' at wrong position): " << optarg << endl;
						continue;
					}
					if(strlen(optarg) <= 2 && s.at(s.size()-1) != '*')
					{
						cerr << "Ignoring word (too short): " << optarg << endl;
						continue;
					}
					if(s.find_first_of(" \t") != s.npos)
					{
						while(s != "")
						{
							size_t pos = s.find_first_of(" \t");
							string word;
							if(pos != s.npos)
							{
								word = s.substr(0, pos);
								s = s.substr(pos+1);
							}
							else
							{
								word = s;
								s = "";
							}
							if(word.size() > 2)
							{
								if(q != "")
									q = q + " ";
								q = q + word;
							}
							else
							{
								cerr << "Ignoring word in phrase (too short): " << word << endl;
							}
						}
						if(q == "")
							continue;
					}
					else
						q = s;
				}
				else if(prev == "and" || prev == "or")
				{
					prev = "";
					continue;
				}
				else
					q = optarg;
				query_string = query_string + q;
				query[qnum] = q;
				qnum++;
				prev = q;
				break;
		}
	}
	
	if(search == false && dir == "")
	{
		cout << "USAGE" << endl;		// IMP: Print this properly
		exit(1);
	}
	
	home = getenv("HOME");
	mss_dir = home;
	mss_dir = mss_dir + "/.mss";
	if(stat(mss_dir.c_str(), &mss_stat) == 0)
	{
		if(S_ISDIR(mss_stat.st_mode) == 0)
		{
			cerr << "Error: There is a non-directory: " << mss_dir << endl << " Please delete this to continue using this application" << endl;
			exit(1);
		}
	}
	else
	{
		if(mkdir(mss_dir.c_str() , 0xFFF) == -1)
		{
			cerr << "Error: Could not create directory: " << mss_dir << endl;
			exit(1);
		}
	}
	mss_dir = mss_dir + "/";
	
	f1 = mss_dir;
	f1 = f1 + index_name;
	f1 = f1 + "1.dat";
	
	f2 = mss_dir;
	f2 = f2 + index_name;
	f2 = f2 + "2.dat";
	
	f3 = mss_dir;
	f3 = f3 + index_name;
	f3 = f3 + "3.dat";
	
	f4 = mss_dir;
	f4 = f4 + index_name;
	f4 = f4 + "4.dat";
	
	if(search)
	{
		if(query_string == "")
		{
			cerr << "Nothing to search for!" << endl
				<< "Type " << argv[0] << " -h for help" << endl;
			exit(1);
		}
		if(stat(f1.c_str() , &mss_stat) == -1 || stat(f2.c_str() , &mss_stat) == -1 || 
				stat(f3.c_str() , &mss_stat) == -1 || stat(f4.c_str() , &mss_stat) == -1)
		{
			cerr << "No database exists. Run " << argv[0] <<" with -c option first. Type '" 
				<< argv[0] << " -h' for help" << endl;
			exit(1);
		}
		File::init(f1 , f2);
		t = new Trie(f3, f4);
		
		Word * res = NULL;
		for(int i=0; i<qnum; i++)
		{
			Word * wres = NULL;
			if(query[i] != "and" && query[i] != "or")
			{
				wres = s(query[i], *t);
				if(res != NULL)
				{
					if(query[i-1] == "and")
					{
						if(wres == NULL)
							res = NULL;
						else
							res = and_them(*res, *wres);
					}
					else if(query[i-1] == "or" && wres != NULL)
						res = or_them(*res, *wres);
				}
				else
					res = wres;
			}
		}
		if(res != NULL)
			print(*res);
		else
			cerr << "Not found" << endl;
	}
	else
	{
		ofstream fout;
		fout.open(f1.c_str(), ios::out | ios::binary | ios::trunc);
		fout.close();
		fout.open(f2.c_str(), ios::out | ios::binary | ios::trunc);
		fout.close();
		fout.open(f3.c_str(), ios::out | ios::binary | ios::trunc);
		fout.close();
		fout.open(f4.c_str(), ios::out | ios::binary | ios::trunc);
		fout.close();
		
		File::init(f1 , f2);
		t = new Trie(f3, f4);
		traverse(dir);
		t->save();
		cout << file_count << " files indexed" << endl;
		File::uninit();
	}
	delete t;
}
