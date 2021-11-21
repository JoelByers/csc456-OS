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
            //dbgProcess();
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
    uniform_int_distribution<> distributor(0, freeFrameList.size() - 1); // define the range

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
                //cout << "FRAME " << frameNumber << " PAGE " << pageTable.size() << endl;
                pageTable.push_back(frameNumber);                   // add frame to page table
                newProcess.pages.push_back(pageTable.size() - 1);      // add page to process
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
            pageTableIter = pageTable.begin();  // set iterator to start of page table
            advance(pageTableIter, page);   // go the page
            pageTable.erase(pageTableIter);     // remove from page from table

            // fix all process pages
            for(Process p : processList)        // for every process
            {
                iListIter2 = p.pages.begin();   // for all pages in process

                for(int i = 0; i < p.pages.size(); i++) 
                {
                    if(*iListIter2 >= page)  // if the page is greater than the page being removed
                    {
                        //cout << "shift " << p.pages.size() << " " << *iListIter2 << endl;
                        (*iListIter2)--;          // shift it down by one to compensate for the removed page
                    }

                    advance(iListIter2, 1);
                }
            }
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
    list<int>::iterator iListIter2;
    int returnValue = 1;

    pListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    // if process exists
    if(pListIter != processList.end())
    {
        iListIter = find(pListIter->pages.begin(), pListIter->pages.end(), logicalAddress); // find the page

        if(iListIter != pListIter->pages.end()) // verify the page exists
        {
            iListIter2 = pageTable.begin();
            advance(iListIter2, *iListIter);    // go to page table index
            iListIter = freeFrameList.begin();
            advance(iListIter, *iListIter2);    // go to frame
            *iListIter = 1;                     // set to one
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
    list<int>::iterator iListIter2;
    int returnValue = 1;

    pListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    // if process exists
    if(pListIter != processList.end())
    {
        iListIter = find(pListIter->pages.begin(), pListIter->pages.end(), logicalAddress); // find the page

        if(iListIter != pListIter->pages.end())
        {
            iListIter2 = pageTable.begin();
            advance(iListIter2, *iListIter);    // go to page table index
            iListIter = freeFrameList.begin();
            advance(iListIter, *iListIter2);    // go to frame
            cout << *iListIter << endl;
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
    list<int>::iterator pageTableItr;

    cout << "----------------" << endl << "Free Frames    : ";
    
    for(int i = 0; i < freeFrameList.size(); i++)   // for all free frames
    {
        pageTableItr = find(pageTable.begin(), pageTable.end(), i);

        if(pageTableItr == pageTable.end())         // frame isn't in page table
        {
            cout << i << ", ";                      // list it as free
        }
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
    list<int>::iterator iter;

    cout << "pageTable -> ";

    for(int frame : pageTable)
    {
        cout << frame << ",";
    }

    cout << endl;

    for(Process p : processList)
    {
        cout << "PID " << p.pid << " ->";

        for(int page : p.pages)
        {
            iter = pageTable.begin();
            advance(iter, page);
            cout << "p" << page << "=f" << *iter << ",";
        }

        cout << endl;
    }
}

void dbg(string s)
{
    cout << s << endl;
}