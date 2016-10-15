#pragma once

#include <vector>

#include "block.h"

class File
{
private:
	std::vector<Block*>		_blocks;
	std::string				_name;
	int						_realSize;

public:
	File();
	File(const std::string & fileName, int realSize, const std::vector<Block*> & blocks);
	virtual ~File();
	std::string getName() const;
	std::string getFileInfo() const;
	std::string getData() const;
	int getSize();
};

