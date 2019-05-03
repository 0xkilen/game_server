#define WINDOWS_LEAN_AND_MEAN

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <shlwapi.h>

using namespace std;

bool ciCharLessThan(char c1, char c2) {
	return tolower(static_cast<unsigned char>(c1)) 
		< tolower(static_cast<unsigned char>(c2));
}

bool compareNoCase(const string& s1, const string& s2) {
	return lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), ciCharLessThan);
}

bool filesOnly = false;
vector<string> output;
string specs("*.*");

void scan(string dir)
{
	if (!dir.empty()) dir += "/";

	WIN32_FIND_DATAA data;
	string wc( dir + "*" );
	HANDLE f = FindFirstFileA(wc.c_str(), &data);
	if (f != INVALID_HANDLE_VALUE) {
		do {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
				continue;
			}
			bool isDir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			if (isDir && (data.cFileName[0] == '.')) {
				continue;
			}

			string name(dir + data.cFileName);
			if ((!isDir || !filesOnly) && (PathMatchSpecExA(data.cFileName, specs.c_str(), PMSF_MULTIPLE) == S_OK)) {
				output.push_back(name);
			}
			if (isDir) {
				(void) scan(name);
			}
		} while (FindNextFileA(f, &data));
		FindClose(f);
	}
}

int main(int argc, char *argv[])
{
	vector<string> paths(1, ".");

	int pos = 0;
	for (int i = 1; i < argc; ++i) {
		string arg(argv[i]);
		if (arg == "--help" || arg == "/?") {
			cerr << "usage: wfind [/f] [paths] [specs]" << endl;
			cerr << "  /f    files only" << endl;
			cerr << "  paths  semicolon separated list of directories" << endl;
			cerr << "  specs  semicolon separated list of filename wildcards" << endl;
			exit(2);
		}
		if (arg == "/f") {
			filesOnly = true;
			continue;
		}
		if (argv[i][0] != '/') {
			if (pos == 0) {
				paths.clear();
				while (true) {
					size_t end = arg.find(";");
					if (end != 0) {
						paths.push_back(arg.substr(0, end));
					}
					if (end >= arg.size()) break;
					arg = arg.substr(end + 1, string::npos);
				}
				++pos;
				continue;
			}
			else if (pos == 1) {
				specs = arg;
				++pos;
				continue;
			}
		}
		cerr << "unrecognized option '" << arg << "'" << endl;
		exit(1);
	}

	for (vector<string>::const_iterator i = paths.begin();
		i != paths.end(); ++i) {
		scan(*i);
	}
	sort(output.begin(), output.end(), compareNoCase);
	for (vector<string>::const_iterator i = output.begin();
		i != output.end(); ++i)
	{
		cout << *i << endl;
	}

	return 0;
}

