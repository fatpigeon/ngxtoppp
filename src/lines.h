#ifndef __LINES_H_
#define __LINES_H_
#include <cstdio>
#include <string>

std::string getline(std::FILE *fp, bool is_listen);

class Lines{
public:
	Lines();
	virtual ~Lines();
	virtual std::string get_line();
	virtual bool is_end()=0;
protected:

};

class StdinRead: public Lines{
public:
	StdinRead();
	virtual std::string get_line();
	virtual bool is_end();
};

class FileRead: public Lines {
public:
	FileRead(const char* filename);
	~FileRead();
protected:
	const char* filename;
	std::FILE *fp;
};

class FileStream : public FileRead {
public:
	FileStream(const char* filename);
	virtual std::string get_line();
	virtual bool is_end();
};


class FileListener: public FileRead{
public:
	FileListener(const char* filename);
	virtual std::string get_line();
	virtual bool is_end();
};
#endif
