// Joel Byers
// Assignment 3
// Due 11/21/21

#include <iostream>
#include <list>
#include <random>
#include <algorithm>

using namespace std;

void parseLine(string in, string* out);
int memoryManager(int memSize, int frameSize);
int allocate(int allocSize, int pid);
int deallocate(int pid);
void printMemory();
int write(int pid, int logicalAddress);
int read(int pid, int logicalAddress);
void dbg(string s);
void dbgProcess();

struct Process
{
    int pid = -1;
    list<int> pages;
};

int nextFrameNum = 0;
list<int> freeFrameList;
list<Process> processList;
list<int> memory;

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
            dbgProcess();
        }
        else if(command[0] == "A")
        {
            int success = allocate(stoi(command[1]), stoi(command[2]));

            if(success == -1)
            {
                cout << "Unable to allocate memory: Not enough free frames" << endl;
            }
        }
        else if(command[0] == "D")
        {
            int success = deallocate(stoi(command[1]));

            if(success == -1)
            {
                cout << "Unable to deallocate memory: Process not found" << endl;
            }
        }
        else if(command[0] == "W")
        {
            int success = write(stoi(command[1]), stoi(command[2]));

            if(success == -1)
            {
                cout << "Unable to write to memory" << endl;
            }
        }
        else if(command[0] == "R")
        {
            int success = read(stoi(command[1]), stoi(command[2]));

            if(success == -1)
            {
                cout << "Unable to read memory" << endl;
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

    for(int i = 0; i < in.length(); i++)    // for every char in input string
    {
        if(in.at(i) == ' ')                 // if the char is a space
        {
            listOut.push_back(nextString);  // add the compiled string to the output list
            nextString = "";                // reset compiled string
            i++;                            // skip space
        }

        nextString += in.at(i);             // add char to compiled string
    }

    listOut.push_back(nextString);          // add last string
    listItr = listOut.begin();

    for(int i = 0; i < listOut.size(); i++) //convert to array
    {
        out[i] = *listItr;
        advance(listItr, 1);
    }
}

int memoryManager(int memSize, int frameSize)
{
    int success = 1;

    // add frames to free frame list
    for(int i = 0; i < memSize; i++)
    {
        freeFrameList.push_back(nextFrameNum);
        memory.push_back(0);
        nextFrameNum++;
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
    uniform_int_distribution<> distributor(0, nextFrameNum - 1); // define the range based on total number of frames

    // verify there are enough free frames
    //if(freeFrameList.size() - pageTable.size() >= allocSize)
    if(freeFrameList.size() >= allocSize)
    {
        int frameNumber;
        list<int>::iterator listItr;

        // for every frame we're trying to add
        for(int i = 0; i < allocSize; i)
        {
            frameNumber = distributor(seed);                                            // get random frame
            listItr = find(freeFrameList.begin(), freeFrameList.end(), frameNumber);    // search for it in free frames
            
            // if frame is in free frame list
            if(listItr != freeFrameList.end())
            {  
                //cout << "FRAME " << frameNumber << " PAGE " << pageTable.size() << endl;
                //pageTable.push_back(frameNumber);                   // add frame to page table
                newProcess.pages.push_back(frameNumber);               // add frame to process
                freeFrameList.erase(listItr);                       // remove frame from free frame list
                i++;                                                // move on to next frame
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

int deallocate(int pid)
{
    int returnValue = 1;
    list<Process>::iterator pListIter;
    list<int>::iterator iListIter2;
    list<int>::iterator pageTableIter;

    // search each process for pid
    pListIter = find_if(processList.begin(), processList.end(),
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    // if pid found
    if(pListIter != processList.end())
    {
        // for every page in process
        for(int page : pListIter->pages)
        {
            freeFrameList.push_back(page);
            //pageTable.erase(page);     // remove page from table
        }

        processList.erase(pListIter);
    }
    else
    {
        returnValue = -1;
    }

    return returnValue;
}

int write(int pid, int logicalAddress)
{
    list<Process>::iterator pListIter;
    list<int>::iterator iListIter;
    list<list<int>::iterator>::iterator iterListIter;
    int returnValue = 1;

    pListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    // if process exists
    if(pListIter != processList.end())
    {
        if(logicalAddress < pListIter->pages.size())
        {
            int frame;

            iListIter = pListIter->pages.begin();
            advance(iListIter, logicalAddress);
            frame = *iListIter;
            iListIter = memory.begin();
            advance(iListIter, frame);
            *iListIter = 1;

            /*
            iterListIter = pListIter->pages.begin();    // get page list
            advance(iterListIter, logicalAddress);      // get pointer to page table element
            iListIter = freeFrameList.begin();          // get frame list
            advance(iListIter, **iterListIter);         // go to frame specified in page table
            *iListIter = 1;
            */
        }
        else
        {
            returnValue = -1;
        }
    }
    else
    {
        returnValue = -1;
    }

    return returnValue;
}

int read(int pid, int logicalAddress)
{
    list<Process>::iterator pListIter;
    list<int>::iterator iListIter;
    list<list<int>::iterator>::iterator iterListIter;
    int returnValue = 1;

    pListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    // if process exists
    if(pListIter != processList.end())
    {
        if(logicalAddress < pListIter->pages.size())
        {
            int frame;

            iListIter = pListIter->pages.begin();
            advance(iListIter, logicalAddress);
            frame = *iListIter;
            iListIter = memory.begin();
            advance(iListIter, frame);
            cout << *iListIter << endl;

            /*
            iterListIter = pListIter->pages.begin();    // get page list
            advance(iterListIter, logicalAddress);      // get pointer to page table element
            iListIter = freeFrameList.begin();          // get frame list
            advance(iListIter, **iterListIter);         // go to frame specified in page table
            *iListIter = 1;
            */
        }
        else
        {
            returnValue = -1;
        }
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
    list<int>::iterator iListIter;

    cout << "----------------" << endl << "Free Frames    : ";
    
    iListIter = freeFrameList.begin();

    for(int i = 0; i < freeFrameList.size(); i++)   // for all free frames
    {
        cout << *iListIter << ",";  
        iListIter++;  
    }

    cout << endl << "----------------" <<  endl << "Processes" << endl << "PID            |";

    pListItr = processList.begin();

    for(int i = 0; i < processList.size(); i++)     // for every process, list PID
    {
        cout << pListItr->pid << "|";
        advance(pListItr, 1);
    }

    cout << endl << "Number of Pages|";
    
    pListItr = processList.begin();

    for(int i = 0; i < processList.size(); i++)     // for every process, list number of pages
    {
        cout << pListItr->pages.size() << "|";
        advance(pListItr, 1);
    }

    cout << endl << "----------------" << endl;
}


// DEBUG FUNCTIONS
void dbgProcess()
{
    list<list<int>::iterator>::iterator iter;

    cout << "Memory -> ";

    for(int frame : memory)
    {
        cout << frame << ",";
    }

    cout << endl;

    for(Process p : processList)
    {
        cout << "PID " << p.pid << " ->";

        for(int page : p.pages)
        {
            cout << page << ",";
        }

        cout << endl;
    }
}

void dbg(string s)
{
    cout << s << endl;
}