#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
using namespace std;

int ProcessInput(int input)
{
    int output;
    output = input + 2;
    return output;
}

int main(int argc, char* argv[]){

	if(argc < 3)
	{
		cout << "not enough arguments"<<endl;
	}

    ifstream inputFile (argv[1]);
    ofstream outputFile(argv[2]);
    if (! inputFile.is_open() | !outputFile.is_open())
    {
        cout<< "Error opening output file or";
    	cout << "Error opening input file";
    	exit(1);
    }

    //vector<string> buffers;
    string buffer;
    while(inputFile >> buffer)
    {
        cout << buffer << endl;
        outputFile << ProcessInput(atoi(buffer.c_str())) << endl;
        //buffers.push_back(buffer);
    }

	return 0;
}



/*void DetectFileIsOpen(string method)
{
    if(method == "in")
    {
        ifstream file (argv[1]);
    }
}*/

//input --> translation -->output
