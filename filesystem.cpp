#include "filesystem.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <errno.h>

//Extracts and returns the last part of a path, aka the name, and removes it from the referenced path
std::string FileSystem::extractNameFromPath(std::string & path)
{
	int lastSlash = path.find_last_of("/");
	std::string name = "";
	if (lastSlash == -1)  //Found no '/'
	{
		name = path;
		path = "";
	}
	else
	{
		name = path.substr(lastSlash + 1);
		path = path.substr(0, lastSlash + 1);
	}
	return name;
}

//Basically preprocessing for processing paths
Directory * FileSystem::startPathProcessing(const std::string & path)
{
	std::string p = path;
	//Remove '/' at the end of path for consistency
	if (p.length() > 3 && p[p.length() - 1] == '/')
		p = p.substr(0, p.length() - 1);

	if (p[0] == '/')  //Relative path
		return _root->processPath(p.substr(1));
	else  //Absolute path
		return _currentDir->processPath(p);
}

//Write data to file
std::string FileSystem::writeToFile(Directory* dir, const std::string & name, const std::string & data
	, const unsigned int accessRights)
{
	if ((data.length() + 511) / 512 < _freeBlocks.size())  //Check if there is enough space in our unused memory
	{
		int index = dir->newFileIndex(name);  //Asks the directory that will be used what index the file will get (in the files-array the directory has). 
		if (index == -1)
			return "Name already used";
		else
		{
			std::vector<Block*> blocks;
			unsigned int i = 0;
			//Takes a full block (512 bytes) from 'data' at a time and puts it in a block if it can (/has to).
			while (i + 512 < data.length())
			{
				//_memBlockDevice.writeBlock(_freeBlocks.front()->, data.substr(i, 512));  //Puts 512 bytes of data in a free block.
				_freeBlocks.front()->writeBlock(data.substr(i, 512));
				blocks.push_back(_freeBlocks.front());  //Stores a pointer to the block that was just written to.
				_freeBlocks.pop_front();  //Deletes the index just used from the list of usable indexes.
				i += 512;
			}
			int left = data.length() - i;
			if (left != 0)  //Checks if there is any data left to store.
			{
				std::string last = ".";
				last.replace(0, 1, 512, '*');  //Creates a 512 byte long string full of '*'
				last.replace(0, left, data.substr(i, left));  //Relpaces as much as needed with actual data. The string is still 512 bytes long!
															  //Same as in the while-loop above...
				//_memBlockDevice.writeBlock(_freeBlocks.front(), last);
				_freeBlocks.front()->writeBlock(last);
				blocks.push_back(_freeBlocks.front());
				_freeBlocks.pop_front();
			}
			dir->addFile(index, name, accessRights, i + left, blocks);  //Sends all the data to the directory which in turn actually creates and stores a file-object.
			return "File successfully written.\n";
		}
	}
	else
	{
		return "Not enough storage left.\n";
	}
}

//Stores virtual disk on hard drive
std::string FileSystem::saveToFile(Directory & directory, std::ofstream & saveFile)
{
	std::string output;
	int children[2] = { 0, 0 };
	directory.getChildren(children);
	saveFile << directory.getName() << "\n" << std::to_string(children[0]) << "\n" << std::to_string(children[1]) << "\n";
	for (int i = 0; i < children[0]; i++)
	{
		Directory* dir = directory.getDirectory(i);
		if (dir != nullptr) //Checks whether the path exists
			saveToFile(*directory.getDirectory(i), saveFile);
		else

			output = "Directory " + std::to_string(i) + " in " + directory.getName() + " is non-existant.\n";
	}
	for (int i = 0; i < children[1]; i++)
	{
		File* file = directory.getFile(i);
		if (file != nullptr) //Checks whether the path exists
			saveFile << file->getName() << "\n" << file->getSize() << "\n" << file->getAccessRights() << "\n" << file->getData() << "\n";
		else
			output = "File " + std::to_string(i) + " in " + directory.getName() + " is non-existant.\n";


	}
	output = "Save successful.\n";
	return output;
}

//Loads virtual disk from real storage
std::string FileSystem::loadFromFile(Directory & directory, FILE* loadFile)
{
	std::string name, data, children[2], size, accessRights;
	char read;

	//reads directory's number of children, 0: directories, 1: files
	for (int i = 0; i < 2; i++)
	{
		while (1)
		{
			fscanf(loadFile, "%c", &read);
			if (read != '\n')
			{
				children[i] += read;
			}
			else
				break;
		}
	}

	//reads and stores the subderectories for the current directory
	for (int i = 0; i < std::stoi(children[0]); i++) //std::stoi converts string to int
	{
		//reads and stores directory name
		while (1)
		{
			fscanf(loadFile, "%c", &read);
			if (read != '\n')
				name += read;
			else
				break;
		}
		directory.addDirectory(name);
		loadFromFile(*directory.getDirectory(i), loadFile);
		name = "";
	}

	//reads and stores files
	for (int i = 0; i < std::stoi(children[1]); i++) //std::stoi converts string to int
	{
		//reads and stores file name
		while (1)
		{
			fscanf(loadFile, "%c", &read);
			if (read != '\n')
				name += read;
			else
				break;
		}

		//reads and stores file size
		while (1)
		{
			fscanf(loadFile, "%c", &read);
			if (read != '\n')
			{
				size += read;
			}
			else
				break;
		}
		//Reads and stores file access rights
		while (1)
		{
			fscanf(loadFile, "%c", &read);
			if (read != '\n')
			{
				accessRights += read;
			}
			else
				break;
		}

		//reads and stores file data
		for (int j = 0; j < std::stoi(size); j++)
		{
			fscanf(loadFile, "%c", &read);
			data += read;
		}
		fscanf(loadFile, "%c", &read); //reads the line feed after the data
		writeToFile(&directory, name, data, std::stoi(accessRights));
		name = "";
		size = "";
		accessRights = "";
		data = "";
	}

	return "Load successful.\n";
}


//-------Public stuff-----

//Default constructor
FileSystem::FileSystem()
{
	_memBlockDevice = MemBlockDevice();
	_root = new Directory("root", nullptr);
	_currentDir = _root;

	for (unsigned int i = 0; i < 250; i++)
	{
		_freeBlocks.push_back(&_memBlockDevice[i]);
	}
}

//Destructor
FileSystem::~FileSystem()
{
	delete _root;
}

//Resets the whole system
std::string FileSystem::format()
{
	delete _root;
	_memBlockDevice.reset();
	_root = new Directory("root", nullptr);
	_currentDir = _root;

	_freeBlocks.clear();
	for (unsigned int i = 0; i < 250; i++)
	{
		_freeBlocks.push_back(&_memBlockDevice[i]);
	}
	return "Format successful.\n";
}

//Lists all directories and files in the current directory
std::string FileSystem::getDirectoryInfo(const std::string & path)
{
	if (path != "") //Checks whether the path refers to current directory
	{
		Directory* dir = startPathProcessing(path);
		if (dir != nullptr) //Checks whether the path exists
			return dir->getInfoString();
		else
			return "Invalid path.\n";
	}
	else
	{
		return _currentDir->getInfoString();
	}
}

//Create a new file
std::string FileSystem::createFile(const std::string & path, const std::string & data)
{
	std::string p = path;
	std::string name = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether the path exists
	{
		return writeToFile(dir, name, data, 0);
	}
	else
		return "Invalid path.\n";
}

//Returns string containing all data in file
std::string FileSystem::getFileData(const std::string & path)
{
	std::string p = path;
	std::string name = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether the path exists
	{
		File* file = dir->getFile(name);
		if (file != nullptr) //Checks whether the file exists
		{
			if (file->getAccessRights() < 2) //Checks access rights for reading file
			{
				std::string data = "";
				if (dir->getFileData(name, data)) //Checks whether the file exists while getting data if it does
					return "Data in file \"" + name + "\":\n" + data + "\n";
				else
					return "Invalid file name.\n";
			}
			else
				return "Access violation reading file \'" + name + "\'.\n";
		}
		else
			return "Invalid file name.\n";
	}
	else
		return "Invalid path.\n";
}

//Initializes storing virtual disk on real storage
std::string FileSystem::createImage(const std::string & path) //real path
{
	std::string output;
	std::ofstream saveFile;
	saveFile.open(path);
	if (saveFile.is_open())
	{
		output = saveToFile(*_root, saveFile);
		saveFile.close();
	}
	else
	{
		output = "Real file active or invalid path name.\n";
	}

	return output;
}

//Initializes restoring virtual disk from real storage
std::string FileSystem::restoreImage(const std::string & path)
{
	format();
	std::string output;
	char read = 'a';
	FILE* loadFile = NULL;
	if ((loadFile = fopen(path.c_str(), "r")) != NULL)
	{
		while (read != '\n')
		{
			fscanf(loadFile, "%c", &read);
		}
		output = loadFromFile(*_root, loadFile);
		fclose(loadFile);
	}
	else
	{
		output = "Invalid path name.\n";
	}

	return output;
}

//Removes a file from the system
std::string FileSystem::removeFile(const std::string & path)
{
	std::string p = path;
	std::string name = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether the path to the file exists
	{
		std::vector<Block*> usedIndexes;
		if (dir->removeFile(name, usedIndexes)) //Checks whether the file exists
		{
			_freeBlocks.insert(std::end(_freeBlocks), std::begin(usedIndexes), std::end(usedIndexes));
			return "Removal successful.\n";
		}
		else
			return "Invalid name.\n";
	}
	else
		return "Invalid path.\n";
}

//Copies a file
std::string FileSystem::copyFile(const std::string & path1, const std::string & path2)
{
	std::string output, data;
	File* file;
	std::string p = path1;
	std::string fileName = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if ((file = dir->getFile(fileName)) != nullptr) //Checks whether the file exists
	{
		if (file->getAccessRights() < 2) //Checks access rights for reading the file
		{
			dir->getFileData(fileName, data);
			p = path2;
			fileName = extractNameFromPath(p);
			if (p != "") //Checks whether the file is to be copied to the same path
			{
				dir = startPathProcessing(p);
				if (dir != nullptr) //Checks whether the path to the file exists
				{
					writeToFile(dir, fileName, data, file->getAccessRights());
					output = "File copy successful.\n";
				}
				else
					output = "Invalid path name.\n";
			}
			else
			{
				writeToFile(_currentDir, fileName, data, file->getAccessRights());
				output = "File copy successful.\n";
			}
		}
	}
	else
		output = "Invalid file name.\n";

	return output;
}

//Adds the content of one file to another
std::string FileSystem::appendFile(const std::string & path1, const std::string & path2)
{
	std::string output, data, data1, data2, fileName, appendFileName, p = path1;
	File* file;
	fileName = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether path 1 exists
	{
		if ((file = dir->getFile(fileName)) != nullptr) //Checks whether file 1 exists
		{
			if (file->getAccessRights() < 2) //Checks access rights for reading file 1
			{
				dir->getFileData(fileName, data1);
				p = path2;
				appendFileName = extractNameFromPath(p);
				dir = startPathProcessing(p);
				if (dir != nullptr) //Checks whether path 2 exists
				{
					if ((file = dir->getFile(appendFileName)) != nullptr) //Checks whether file 2 exists
					{
						if (file->getAccessRights() == 0 || file->getAccessRights() == 2) //Checks access rights for writing in file 2
						{
							dir->getFileData(appendFileName, data2);
							std::vector<Block*> usedIndexes;
							dir->removeFile(appendFileName, usedIndexes);
							_freeBlocks.insert(std::end(_freeBlocks), std::begin(usedIndexes), std::end(usedIndexes));
							data = data2 + data1;
							writeToFile(dir, appendFileName, data, file->getAccessRights());
							output = "File successfully appended.\n";
						}
						else
							output = "Access violation writing in '" + appendFileName + "'.\n";
					}
					else
						output = "Invalid file 2 name.\n";
				}
				else
					output = "Invalid '" + appendFileName + "' path.\n";
			}
			else
				output = "Access violation reading '" + fileName + "'.\n";
		}
		else
			output = "Invalid file 1 name.\n";
	}
	else
		output = "Invalid path name.\n";

	return output;
}

//Rename and/or move a file. mv command in shell
std::string FileSystem::renameFile(const std::string & path1, const std::string & path2)
{
	std::string output, prevName, newName, p;
	p = path1;
	prevName = extractNameFromPath(p);
	Directory* prevDir = startPathProcessing(p), *newDir;
	if (prevDir != nullptr) //Checks whether path 1 exists
	{
		File* file = prevDir->getFile(prevName);
		if (file != nullptr)
		{
			if (file->getAccessRights() == 0 || file->getAccessRights() == 2) //Checks access rights for writing in file
			{
				p = path2;
				newName = extractNameFromPath(p);
				newDir = startPathProcessing(p);
				if (prevDir == newDir) //Checks whether the file is to be renamed only
					output = prevDir->renameFile(prevName, newName);
				else
				{
					copyFile(path1, path2);
					std::vector<Block*> usedIndexes;
					prevDir->removeFile(prevName, usedIndexes);
					_freeBlocks.insert(std::end(_freeBlocks), std::begin(usedIndexes), std::end(usedIndexes));
					output = "File successfully moved.\n";
				}
			}
			else
				output = "Access violation writing in'" + prevName + ".\n";
		}
		else
			output = "Invalid file 1 name.\n";
	}
	else
		output = "Invalid '" + prevName + "' path.\n";

	return output;
}

//Create a directory
std::string FileSystem::makeDir(const std::string & path)
{
	std::string p = path;
	std::string name = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether the path exists
		return dir->addDirectory(name);
	else
		return "Invalid path.\n";
}

//Sets new working directory
std::string FileSystem::goToFolder(const std::string & path, std::string & fullPath)
{
	Directory* dir = startPathProcessing(path);
	if (dir != nullptr) //Checks whether the path exists
	{
		_currentDir = dir;
		fullPath = getFullPath();
		return "";
	}
	else
		return "Invalid path.\n";
}

//Get the full file path from root to current working directory
std::string FileSystem::getFullPath()
{
	return _currentDir->getPath() + "/";
}

//Changes acces rights for a file
std::string FileSystem::accessRights(const std::string & accessRights, const std::string & path)
{
	std::string output, name, p;
	p = path;
	name = extractNameFromPath(p);
	Directory* dir = startPathProcessing(p);
	if (dir != nullptr) //Checks whether the path to the file exists
	{
		File* file = dir->getFile(name);
		if (file != nullptr) //Checks whether the file exists
		{
			file->setAccessRights(std::stoi(accessRights));
		}
		else
			output = "Invalid file name.\n";
	}
	else
		output = "Invalid path name.\n";
	return output;
}
