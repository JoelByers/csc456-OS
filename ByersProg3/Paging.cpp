#include <iostream>
#include <list>
#include <random>
#include <algorithm>

using namespace std;

void parseLine(string in, string* out);
int memoryManager(int memSize, int frameSize);
int allocate(int allocSize, int pid);
void printMemory();
void dbg(string s);

struct Process
{
    int pid = -1;
    list<int> pages;
};

list<int> freeFrameList;
list<Process> processList;
list<int> pageTable;

int main()
{
    string command[3] {};
    string userInput;

    do
    {
        // reset array to empty strings
        for(int i = 0; i < sizeof(command)/sizeof(command[0]); i++)
        {
            command[i] = "";
        }

        userInput = "";
        getline(cin, userInput);
        parseLine(userInput, command);

        if(command[0] == "M")
        {
            memoryManager(stoi(command[1]), 1);
        }
        else if(command[0] == "P")
        {
            printMemory();
        }
        else if(command[0] == "A")
        {
            int success = allocate(stoi(command[1]), stoi(command[2]));

            if(!success)
            {
                cout << "Unable to allocate memory: Not enough free frames" << endl;
            }
        }
        else if (userInput != "quit")
        {
            cout << "Unknown Command" << endl;
        }
    } 
    while(userInput != "quit");    

    return 0;
}

void parseLine(string in, string* out)
{
    list<string> listOut;
    string nextString = "";
    list<string>::iterator listItr;

    for(int i = 0; i < in.length(); i++)
    {
        if(in.at(i) == ' ')
        {
            listOut.push_back(nextString);
            nextString = "";
            i++;
        }

        nextString += in.at(i);
    }

    listOut.push_back(nextString);
    listItr = listOut.begin();

    for(int i = 0; i < listOut.size(); i++)
    {
        out[i] = *listItr;
        advance(listItr, 1);
    }
}

int memoryManager(int memSize, int frameSize)
{
    int success = 1;

    for(int i = 0; i < memSize; i++)
    {
        freeFrameList.push_back(0);
    }

    return success; 
}

int allocate(int allocSize, int pid)
{
    int returnValue = 1;

    Process newProcess;
    newProcess.pid = pid;

    random_device random; // obtain a random number from hardware
    mt19937 seed(random()); // seed the generator
    uniform_int_distribution<> distributor(0, freeFrameList.size()); // define the range

    // verify there are enough free frames
    if(freeFrameList.size() - pageTable.size() >= allocSize)
    {
        int frameNumber;
        list<int>::iterator listItr;

        // for every frame we're trying to add
        for(int i = 0; i < allocSize; i)
        {
            frameNumber = distributor(seed);                                    // get random frame
            listItr = find(pageTable.begin(), pageTable.end(), frameNumber);    // search for it in page table
            
            // if frame is not in page table
            if(listItr == pageTable.end())
            {
                pageTable.push_back(frameNumber);               // add frame to page table
                newProcess.pages.push_front(pageTable.size());  // add page to process
                i++;                                            // move on to next frame
            }
        }

        processList.push_front(newProcess);
    }
    else
    {
        returnValue = -1;
    }

    return returnValue;
}

void printMemory()
{
    list<Process>::iterator pListItr;
    list<int>::iterator iListItr;

    cout << endl << "Free Frames    :";

    for(int i = 0; i < freeFrameList.size(); i++)
    {
        iListItr = find(pageTable.begin(), pageTable.end(), i);

        if(iListItr == pageTable.end())
        {
            cout << i << ", ";
        }
    }

    cout << endl << "----------------" <<  endl << "Processes" << endl << "PID            |";

    pListItr = processList.begin();

    for(int i = 0; i < processList.size(); i++)
    {
        cout << pListItr->pid << "|";
        advance(pListItr, 1);
    }

    cout << endl << "Number of Pages|";
    
    pListItr = processList.begin();

    for(int i = 0; i < processList.size(); i++)
    {
        cout << pListItr->pages.size() << "|";
        advance(pListItr, 1);
    }

    cout << endl << "----------------" << endl;
}

void dbg(string s)
{
    cout << s << endl;
}