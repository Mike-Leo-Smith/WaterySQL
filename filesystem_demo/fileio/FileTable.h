#ifndef FILE_TABLE
#define FILE_TABLE
#include <string>
#include "../utils/MyBitMap.h"
#include <map>
#include <vector>
#include <set>
#include <fstream>

class FileTable {
private:
	std::multiset<std::string> _exists;
	std::multiset<std::string> _opened;
	std::vector<std::string> _file_name;
	std::vector<std::string> _format;
	std::map<std::string, int> _name_to_id;
	std::string* _id_to_name;
	MyBitMap *_ff;
	MyBitMap *_ft;
	int _n;
	void _load() {
		std::ifstream fin("filenames");
		fin >> _n;
		for (int i = 0; i < _n; ++ i) {
			std::string s, a;
			fin >> s;
			_exists.insert(s);
			_file_name.push_back(s);
			fin >> a;
			_format.push_back(a);
		}
		fin.close();
	}
	void _save() {
		std::ofstream fout("filenames");
		fout << _file_name.size() << std::endl;
		for (uint i = 0; i < _file_name.size(); ++ i) {
			fout << _file_name[i] << std::endl;
			fout << _format[i] << std::endl;
		}
		fout.close();
	}
public:
	int newTypeID() {
		int k = _ft->findLeftOne();
		_ft->setBit(k, 0);
		return k;
	}
	int newFileID(const std::string& name) {
		int k = _ff->findLeftOne();
		_ff->setBit(k, 0);
		_name_to_id[name] = k;
		_opened.insert(name);
		_id_to_name[k] = name;
		return k;
	}
	bool ifexist(const std::string& name) {
		return (_exists.find(name) != _exists.end());
	}
	void addFile(const std::string& name, const std::string& fm) {
		_exists.insert(name);
		_file_name.push_back(name);
		_format.push_back(fm);
	}
	int getFileID(const std::string& name) {
		if (_opened.find(name) == _opened.end()) {
			return -1;
		}
		return _name_to_id[name];
	}
	void freeTypeID(int typeID) {
		_ft->setBit(typeID, 1);
	}
	void freeFileID(int fileID) {
		_ff->setBit(fileID, 1);
		std::string name = _id_to_name[fileID];
		_opened.erase(name);
	}
	std::string getFormat(std::string name) {
		for (uint i = 0; i < _file_name.size(); ++ i) {
			if (name == _file_name[i]) {
				return _format[i];
			}
		}
		return "-";
	}
	FileTable(int fn, int tn) {
		_load();
		_ft = new MyBitMap(tn, 1);
		_ff = new MyBitMap(fn, 1);
		_id_to_name = new std::string[fn];
		_opened.clear();
	}
	~FileTable() {
		_save();
	}
};
#endif
