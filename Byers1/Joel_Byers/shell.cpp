// Joel Byers
// Assignment 1
// Due 9/13/21

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include <vector>

using namespace std;

void createChild(string command);
vector<string> parseCommand(string command);

int main()
{
    string command = "";

    while(command != "exit")    // until user types exit
    {
        cout << "Byers>";
        getline(cin, command);  // Read user input
        createChild(command);   // Create child process based on user input
    }
}

void createChild(string command)
{
    pid_t pid = fork();

    if (pid < 0)                                        // If fork fails
    {
        cout << "ERROR: Child not created" << endl;
    }
    else if (pid == 0)                                  // Create child process
    {
        vector<string> args = parseCommand(command);
        
        vector<char*> c_args;
        for(int i = 0; i < args.size(); i++)
        {
            c_args.push_back(&args[i][0]);              // Populate new char* array from args
        }

        c_args.push_back(NULL);                         // Add null for execvp

        execvp(args[0].c_str(), &c_args[0]);            // System Call
    }
    else                                                // Parent waits for child
    {
        wait(NULL);
    }
}


vector<string> parseCommand(string command)
{
    vector<string> output;
    int argNum = 0;

    string nextArg = "";
    for(char i : command)               // for each char in command
    {
        if(i != ' ')                    // until there is a space
        {
            nextArg.push_back(i);       // create a string for a single arg
        }
        else
        {
            output.push_back(nextArg);  // add new arg to output vector
            argNum++;                   // increment for next arg
            nextArg = "";               // reset nextArg
        }      
    }

    output.push_back(nextArg);          // add new arg to output vecotr

    return output;
}
