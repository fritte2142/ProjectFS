#include <iostream>
#include <sstream>
#include <crtdbg.h>

#include "filesystem.h"

const int MAXCOMMANDS = 8;
const int NUMAVAILABLECOMMANDS = 15;

std::string availableCommands[NUMAVAILABLECOMMANDS] = {
    "quit","format","ls","create","cat","createImage","restoreImage",
    "rm","cp","append","mv","mkdir","cd","pwd","help"
};

/* Takes usercommand from input and returns number of commands, commands are stored in strArr[] */
int parseCommandString(const std::string &userCommand, std::string strArr[]);
int findCommand(std::string &command);
std::string help();

int main(void)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	FileSystem fileSystem;
	std::string userCommand, commandArr[MAXCOMMANDS];
	std::string user = "user@DV1492";    // Change this if you want another user to be displayed
	std::string currentDir = "/root/";    // current directory, used for output

    bool bRun = true;

    do {
        std::cout << user << ":" << currentDir << "$ ";
        getline(std::cin, userCommand);

        int nrOfCommands = parseCommandString(userCommand, commandArr);
        if (nrOfCommands > 0)
		{
            int cIndex = findCommand(commandArr[0]);
            switch(cIndex) {

            case 0: // quit
                bRun = false;
                std::cout << "Exiting\n";
                break;
            case 1: // format
                // Call fileSystem.format()
                break;
            case 2: // ls
				std::cout << fileSystem.ls(commandArr[1]);
                break;
            case 3: // create
			{
				std::cout << "Enter file content below:\n";
				std::string data = "";
				getline(std::cin, data);
				std::cout << fileSystem.createFile(commandArr[1], data);
				break;
			}
            case 4: // cat
				std::cout << fileSystem.getFileData(commandArr[1]);
                break;
            case 5: // createImage
				std::cout << fileSystem.createImage(commandArr[1]);
                break;
            case 6: // restoreImage
				std::cout << fileSystem.restoreImage(commandArr[1]);
				break;
            case 7: // rm
                break;
            case 8: // cp
                break;
            case 9: // append
                break;
            case 10: // mv
                break;
            case 11: // mkdir
				std::cout << fileSystem.makeDir(commandArr[1]);
                break;
            case 12: // cd
				std::cout << fileSystem.goToFolder(commandArr[1], currentDir);
                break;
            case 13: // pwd
				std::cout << fileSystem.getFullPath() + "\n";
                break;
            case 14: // help
                std::cout << help() << std::endl;
                break;
            default:
                std::cout << "Unknown command: " << commandArr[0] << std::endl;
            }
			for (unsigned int i = 0; i < nrOfCommands; i++)
			{
				commandArr[i] = "";
			}
        }
    } while (bRun == true);

    return 0;
}

int parseCommandString(const std::string &userCommand, std::string strArr[]) {
    std::stringstream ssin(userCommand);
    int counter = 0;
    while (ssin.good() && counter < MAXCOMMANDS) {
        ssin >> strArr[counter];
        counter++;
    }
    if (strArr[0] == "") {
        counter = 0;
    }
    return counter;
}
int findCommand(std::string &command) {
    int index = -1;
    for (int i = 0; i < NUMAVAILABLECOMMANDS && index == -1; ++i) {
        if (command == availableCommands[i]) {
            index = i;
        }
    }
    return index;
}

std::string help() {
    std::string helpStr;
    helpStr += "OSD Disk Tool .oO Help Screen Oo.\n";
    helpStr += "-----------------------------------------------------------------------------------\n" ;
    helpStr += "* quit:                             Quit OSD Disk Tool\n";  //Done
    helpStr += "* format;                           Formats disk\n";  //Started...
    helpStr += "* ls     <path>:                    Lists contents of <path>.\n";  //Done
    helpStr += "* create <path>:                    Creates a file and stores contents in <path>\n";
    helpStr += "* cat    <path>:                    Dumps contents of <file>.\n";
    helpStr += "* createImage  <real-file>:         Saves disk to <real-file>\n";
    helpStr += "* restoreImage <real-file>:         Reads <real-file> onto disk\n";
    helpStr += "* rm     <file>:                    Removes <file>\n";
    helpStr += "* cp     <source> <destination>:    Copy <source> to <destination>\n";
    helpStr += "* append <source> <destination>:    Appends contents of <source> to <destination>\n";
    helpStr += "* mv     <old-file> <new-file>:     Renames <old-file> to <new-file>\n";
    helpStr += "* mkdir  <directory>:               Creates a new directory called <directory>\n";  //Done
    helpStr += "* cd     <directory>:               Changes current working directory to <directory>\n";  //Done
    helpStr += "* pwd:                              Get current working directory\n";  //Done
    helpStr += "* help:                             Prints this help screen\n";  //Done
    return helpStr;
}
