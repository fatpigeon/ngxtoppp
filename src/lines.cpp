#include "lines.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

std::string getline(std::FILE *fp, bool is_listen){
	std::string str;
	
	while(true){
		char ch = std::fgetc(fp);
		if(std::feof(fp)){
			if(is_listen){
				std::fseek(fp, 0, SEEK_CUR);
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			else{
				return str;
			}
		}
		else if(!ch) {
			std::cout << "ch is none wait one second\n";
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}
		else if(ch == '\n'){
			return str;
		}
		str += ch;
	}
}

//==========
//  Lines
//==========
Lines::Lines(){

}
Lines::~Lines(){

}
std::string Lines::get_line()
{
	return std::string();
}


//==========
//  StdinRead
//==========
StdinRead::StdinRead()
{

}

std::string StdinRead::get_line(){
	return std::move(getline(stdin, true));
}

bool StdinRead::is_end() {
	return false;
}

//============
//   FileRead
//===========
FileRead::FileRead(const char* filename)
:filename(filename),
 fp(std::fopen(filename, "r")){
	if(fp == NULL){
		//error;
	}

}
FileRead::~FileRead(){
	std::fclose(fp);
	fp = NULL;
	std::cout << "file closed\n";
}

//============
//   FileStream
//============
FileStream::FileStream(const char* filename)
:FileRead(filename){
	std::fseek(fp, 0, SEEK_SET);
}

std::string FileStream::get_line(){
	return std::move(getline(fp, false));
}

bool FileStream::is_end() {
	return std::feof(fp);
}

//==============
//    FileListener
//==============
FileListener::FileListener(const char* filename)
:FileRead(filename){
	std::fseek(fp, 0, SEEK_END);
}

std::string FileListener::get_line(){
	return std::move(getline(fp, true));
}

bool FileListener::is_end() {
	return false;
}