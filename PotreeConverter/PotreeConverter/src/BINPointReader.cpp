

#include <fstream>
#include <iostream>
#include <vector>

#include "boost/filesystem.hpp"
#include <boost/algorithm/string.hpp>

#include "BINPointReader.hpp"

namespace fs = boost::filesystem;

using std::ifstream;
using std::cout;
using std::endl;
using std::vector;
using boost::iequals;
using std::ios;


BINPointReader::BINPointReader(string path,  AABB aabb, double scale){
	this->path = path;
	this->aabb = aabb;
	this->scale = scale;
	
	if(fs::is_directory(path)){
		// if directory is specified, find all las and laz files inside directory

		for(fs::directory_iterator it(path); it != fs::directory_iterator(); it++){
			fs::path filepath = it->path();
			if(fs::is_regular_file(filepath)){
				files.push_back(filepath.string());
			}
		}
	}else{
		files.push_back(path);
	}

	currentFile = files.begin();
	reader = new ifstream(*currentFile, ios::in | ios::binary);

	attributes.add(PointAttribute::POSITION_CARTESIAN);
	attributes.add(PointAttribute::COLOR_PACKED);
}

BINPointReader::~BINPointReader(){
		close();
}

void BINPointReader::close(){
	if(reader != NULL){
		reader->close();
		delete reader;
		reader = NULL;
	}
}

long BINPointReader::numPoints(){
	//TODO

	return 0;
}

bool BINPointReader::readNextPoint(){
	bool hasPoints = reader->good();

	if(!hasPoints){
		// try to open next file, if available
		reader->close();
		delete reader;
		reader = NULL;
		currentFile++;

		if(currentFile != files.end()){
			reader = new ifstream(*currentFile, ios::in | ios::binary);
			hasPoints = reader->good();
		}
	}

	if(hasPoints){
		point = Point();
		char* buffer = new char[attributes.byteSize];
		reader->read(buffer, attributes.byteSize);

		if(!reader->good()){
			return false;
		}
		
		int offset = 0;
		for(int i = 0; i < attributes.size(); i++){
			const PointAttribute attribute = attributes[i];
			if(attribute == PointAttribute::POSITION_CARTESIAN){
				int* iBuffer = reinterpret_cast<int*>(buffer+offset);
				point.x = (iBuffer[0] * scale) + aabb.min.x;
				point.y = (iBuffer[1] * scale) + aabb.min.y;
				point.z = (iBuffer[2] * scale) + aabb.min.z;
			}else if(attribute == PointAttribute::COLOR_PACKED){
				unsigned char* ucBuffer = reinterpret_cast<unsigned char*>(buffer+offset);
				point.r = ucBuffer[0];
				point.g = ucBuffer[1];
				point.b = ucBuffer[2];
				point.a = 255;
			}
			offset += attribute.byteSize;
		}
		
		delete [] buffer;
	}

	return hasPoints;
}

Point BINPointReader::getPoint(){
	return point;
}

AABB BINPointReader::getAABB(){
	AABB aabb;
	//TODO

	return aabb;
}








