#include "Directory.h"

//Default constructor
Directory::Directory()
{
	//Nothing
}

//Constructor
Directory::Directory(const std::string & name, Directory * parent)
{
	_name = name;
	_parent = parent;
}

//Destructor
Directory::~Directory()
{
	for (unsigned int i = 0; i < _directories.size(); i++)
	{
		delete _directories[i];
	}
	for (unsigned int i = 0; i < _files.size(); i++)
	{
		delete _files[i];
	}
}

//Get directory name
std::string Directory::getName()
{
	return _name;
}

//Returns parent of current directory
Directory * Directory::getParent() const
{
	return _parent;
}

//Get info about only this directory
std::string Directory::toString() const
{
	std::string tempName = ".";
	tempName.replace(0, 1, 24, ' ');
	int l = int(fminf(float(_name.length()), 22.f));
	tempName.replace(0, l, _name.substr(0, l));

	return tempName + std::to_string(_directories.size()) + "\t"
		+ std::to_string(_files.size()) + "\n";
}

//Get info about what is inside this directory
std::string Directory::getInfoString() const
{
	std::string output = "\nDirectory name: " + _name + "\n\nDirectories: " + std::to_string(_directories.size())
		+ "\nName:                   Dirs:\tFiles:\n";
	for (unsigned int i = 0; i < _directories.size(); i++)
	{
		output.append(_directories[i]->toString());
	}
	output += "\nFiles: " + std::to_string(_files.size()) + "\nName:                   Size:\tDisk Size:\n";
	for (unsigned int i = 0; i < _files.size(); i++)
	{
		output.append(_files[i]->getFileInfo());
	}
	return output + "\n";
}

//Returns path to current directory
std::string Directory::getPath() const
{
	if (_parent == nullptr)
		return "";
	else
		return _parent->getPath() + "/" + _name;
}

//Returns child directory based on index
Directory * Directory::getDirectory(const unsigned int & index)
{
	if (index > _directories.size() - 1)
		return nullptr;
	else
		return _directories[index];
}

//Returns child file based on index
File* Directory::getFile(const unsigned int & index)
{
	if (index > _files.size() - 1)
		return nullptr;
	else
		return _files[index];
}

//Returns child file based on name
File * Directory::getFile(const std::string & name)
{
	File* output = nullptr;
	for (unsigned int i = 0; i < _files.size(); i++)
		if (_files[i]->getName() == name)
			output = _files[i];
	return output;
}

//Stores all data from given param name in given param data
bool Directory::getFileData(const std::string & name, std::string & data) const
{
	int index = -1;
	for (unsigned int i = 0; i < _files.size() && index == -1; i++)
	{
		if (name == _files[i]->getName())
			index = i;
	}
	if (index != -1)
	{
		data = _files[index]->getData();
		return true;
	}
	else
		return false;
}

//Stores number of children in param array children
void Directory::getChildren(int* children)
{
	children[0] = _directories.size();
	children[1] = _files.size();
}

//Calculates the index of a new file with the name 'name'. Returns -1 if name is already used
int Directory::newFileIndex(const std::string & name)
{
	//Special case if it�s the first one created
	if (_files.size() == 0)
	{
		return 0;
	}
	//Normal case
	else
	{
		unsigned int i = 0;
		//Loop while 'name' comes (alphabetically) after the directory-names tested 
		while (i < _files.size() && strcmp(name.c_str(), _files[i]->getName().c_str()) > 0)
		{
			if (name == _files[i]->getName())
				return -1;
			i++;
		}
		return i;
	}
}

//Processes the path-string given
Directory * Directory::processPath(const std::string & path)
{
	if (path != "")  //Seems stupid but is needed
	{
		if (path == "..")  //Checks if the only thing left is to go up one level
			return _parent;
		else if (path.substr(0, 3) == "../")  //Checks if it should go up one level (and possibly more, unlike the first check)
			return _parent->processPath(path.substr(3));
		else if (path.substr(0, 2) == "./")  //Checks if it should work in the current directory
			return this->processPath(path.substr(2));
		//Extracts the next directory name in the path
		unsigned int end = path.find_first_of("/");
		bool lastPart = false;
		if (end >= path.length() - 1)  //Checks if this is the last part of the path, aka the final directory
			lastPart = true;
		std::string testString = path.substr(0, end);
		//Searches for a child directory with a matching name in this directory
		for (unsigned int i = 0; i < _directories.size(); i++)
		{
			if (testString == _directories[i]->getName())
			{
				if (lastPart)  //If lastPart is true nothing remains of path and the recursion is done
					return _directories[i];
				else
					return _directories[i]->processPath(path.substr(end + 1));
			}
		}
		//No matching name found
		return nullptr;
	}
	else  //Just trust me, this is needed
	{
		return this;
	}
}

//Add a child directory
std::string Directory::addDirectory(const std::string & name)
{
	//Special case if it�s the first one created
	if (_directories.size() == 0)
	{
		_directories.push_back(new Directory(name, this));
		return "Directory creation successful!\n";
	}
	//Normal case
	else
	{
		unsigned int i = 0;
		//Loop while 'name' comes (alphabetically) after the directory-names tested 
		while (i < _directories.size() && strcmp(name.c_str(), _directories[i]->getName().c_str()) > 0)
		{
			if (name == _directories[i]->getName())
				return "Name already used.\n";
			i++;
		}
		_directories.insert(_directories.begin() + i, new Directory(name, this));
		return "Directory creation successful!\n";
	}
}

//Add a child file
void Directory::addFile(int index, const std::string & name, int accessRights, int size
	, const std::vector<Block*>& blocks)
{
	_files.insert(_files.begin() + index, new File(name, accessRights, size, blocks));
}

//Rename file
std::string Directory::renameFile(const std::string & prevName, const std::string & newName)
{
	std::string output = "Invalid file name.\n";
	for (unsigned int i = 0; i < _files.size(); i++)
		if (_files[i]->getName() == prevName)
		{
			_files[i]->setName(newName);
			output = "Rename successful.\n";
			break;
		}

	return output;
}

//Remove file
bool Directory::removeFile(const std::string & name, std::vector<Block*>& usedIndexes)
{
	int index = -1;
	for (unsigned int i = 0; i < _files.size() && index == -1; i++)
	{
		if (name == _files[i]->getName())
			index = i;
	}
	if (index != -1)
	{
		usedIndexes = _files[index]->getUsedIndexes();
		delete _files[index];
		_files.erase(_files.begin() + index);
		return true;
	}
	else
		return false;
}
