// Joel Byers
// Assignment 4  
// Due 12/4/21

#include <iostream>
#include <string>
#include <list>
#include <random>
#include <algorithm>

using namespace std;

struct Page
{
    int num;
    bool valid = true;
};

struct Process
{
    int pid = -1;
    list<Page> pages;
};

struct LRUData
{
    int pid;
    int page;
};

void parseLine(string in, string* out);
int memoryManager(int memSize, int frameSize);
int allocate(int allocSize, int pid);
int deallocate(int pid);
int getVictom();
Page givePageToProcess(int frameNumber, int pid, Process &newProcess);
void updateLRUStack(int pid, int page);
void printMemory();
int write(int pid, int logicalAddress);
int read(int pid, int logicalAddress);
void dbg(string s);
void dbgProcess();

list<int> freeFrameList;
list<Process> processList;
list<int> memory;
list<LRUData> LRUStack;

int nextFrameNum = 0;

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

    int frameNumber;
    list<int>::iterator freeFrameIter;

        // for every frame we're trying to add
    for(int i = 0; i < allocSize; i)
    {
        Page newPage;

        // if there are free frames
        if(freeFrameList.size() > 0)
        {
            frameNumber = distributor(seed);                                                // get random frame
            freeFrameIter = find(freeFrameList.begin(), freeFrameList.end(), frameNumber);  // search for it in free frames
                    
            // if frame is in free frame list
            if(freeFrameIter != freeFrameList.end())
            {  
                freeFrameList.erase(freeFrameIter);                 // remove frame from free frame list
                givePageToProcess(frameNumber, pid, newProcess);    // fill page table and LRU stack
                updateLRUStack(pid, frameNumber);
                i++;                                                // move on to next frame
            }
        }
        else    // page replacement
        {
            frameNumber = getVictom();                                          // get victom frame and set valid bit 
            givePageToProcess(frameNumber, pid, newProcess);     // fill page table and LRU stack
            updateLRUStack(pid, frameNumber);
            i++;                                                                // move on to next frame
        }

    }

    processList.push_front(newProcess);     // add process to process list

    return returnValue;
}

int deallocate(int pid)
{
    int returnValue = 1;
    int originalStackSize;
    list<Process>::iterator pListIter;
    list<int>::iterator iListIter2;
    list<int>::iterator pageTableIter;
    list<LRUData>::iterator lruStackIter;
    list<LRUData>::iterator tempStackIter;

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
        for(Page page : pListIter->pages)   // check each process for frame being used. if it is,
        {                                   //  dont' add it to free frames
            bool frameNotFound = true;

            for(Process pro : processList)
            {
                for(Page proPage : pro.pages)
                {
                    if(page.num == proPage.num && pro.pid != pid)
                    {
                        frameNotFound = false;
                        break;
                    }
                }
            }

            if(frameNotFound)
            {
                freeFrameList.push_back(page.num);
            }
        }

        processList.erase(pListIter);

        // remove all process pages from stack
        lruStackIter = LRUStack.begin();
        originalStackSize = LRUStack.size();

        for(int i = 0; i < originalStackSize; i++)
        {
            if(lruStackIter->pid == pid)            // if page has pid being deallocated
            {
                tempStackIter = lruStackIter;       // get temp iterator to element
                tempStackIter++;                    // increment temp
                LRUStack.erase(lruStackIter);       // erase original element
                lruStackIter = tempStackIter;       // resore original variable
            }
            else
            {
                lruStackIter++;                     // move to next element
            }
        }
    }
    else
    {
        returnValue = -1;
    }

    return returnValue;
}

int write(int pid, int logicalAddress)
{
    list<Process>::iterator proListIter;
    list<Page>::iterator pgeListIter;
    list<int>::iterator memoryIter;
    int returnValue = 1;

    proListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    if(proListIter != processList.end())                        // if process exists
    {
        if(logicalAddress < proListIter->pages.size())          // if the page exists
        {
            int frame;

            pgeListIter = proListIter->pages.begin();
            advance(pgeListIter, logicalAddress);               // find page      

            if(!pgeListIter->valid)                             // if invalid
            {
                pgeListIter->num = getVictom();                 // get victom, LRU Stack is updated 
                pgeListIter->valid = true;                      // set to valid
            }
            
            updateLRUStack(pid, pgeListIter->num);          // update LRU Stack

            frame = pgeListIter->num;                           // get frame number
            memoryIter = memory.begin();
            advance(memoryIter, frame);                         // find frame in memory
            *memoryIter = 1;                                    // write 1
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
    list<Process>::iterator proListIter;
    list<Page>::iterator pgeListIter;
    list<int>::iterator memoryIter;
    int returnValue = 1;

    proListIter = find_if(processList.begin(), processList.end(), // find the process
    [pid] (const Process& p) 
    {
        return p.pid == pid;
    });

    if(proListIter != processList.end())                        // if process exists
    {
        if(logicalAddress < proListIter->pages.size())          // if the page exists
        {
            int frame;

            pgeListIter = proListIter->pages.begin();
            advance(pgeListIter, logicalAddress);               // find page      

            if(!pgeListIter->valid)                             // if invalid
            {
                pgeListIter->num = getVictom();                 // get victom, LRU Stack is updated
                pgeListIter->valid = true;                      // set to valid
            }

            updateLRUStack(pid, pgeListIter->num);          // update LRU Stack

            frame = pgeListIter->num;                           // get frame number
            memoryIter = memory.begin();
            advance(memoryIter, frame);                         // find frame in memory
            cout << *memoryIter << endl;                        // print contents
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

int getVictom()
{
    // return frame from victom,
    // also sets victom page to invalid

    list<Process>::iterator proListIter;
    list<LRUData>::iterator lastPageIter;
    list<Page>::iterator pgeListIter;
    int victomFrameNum;
    
    lastPageIter = --LRUStack.end();        // get last element
    victomFrameNum = lastPageIter->page;    // get page number

    // assume pid and page exist, find them
    // find process
    proListIter = find_if(processList.begin(), processList.end(), 
    [lastPageIter] (const Process& pro)
    {
        return pro.pid == lastPageIter->pid;
    });

    //find page
    pgeListIter = find_if(proListIter->pages.begin(), proListIter->pages.end(), 
    [victomFrameNum] (const Page& pge)
    {
        return pge.num == victomFrameNum;
    });

    pgeListIter->valid = false;         // set previous owner page to invalid
    LRUStack.erase(lastPageIter);       // remove last element from LRU stack

    return victomFrameNum;
}

Page givePageToProcess(int frameNumber, int pid, Process &newProcess)
{
    Page newPage;
    LRUData newData;
    newPage.num = frameNumber;                      // add frame number to page
    newData.page = frameNumber;                     // add frame number to LRU data
    newData.pid = pid;                              // add PID to LRU data
    newProcess.pages.push_back(newPage);            // add frame to process
    LRUStack.push_front(newData);                   // push page and PID to LRU Stack
    
    return newPage;
}

void updateLRUStack(int pid, int page)
{
    // moves used element to front

    list<LRUData>::iterator lruIter;
    LRUData newData;
    newData.pid = pid;
    newData.page = page;

    lruIter = find_if(LRUStack.begin(), LRUStack.end(), // find element that matches pid and page
    [pid, page] (LRUData const& data)
    {
        return data.pid == pid && data.page == page;
    });

    LRUStack.push_front(newData);                       // push data to front

    if(lruIter != LRUStack.end())
    {
        LRUStack.erase(lruIter);                        // pop data off back
    }
}

void printMemory()
{
    list<Process>::iterator pListItr;
    list<int>::iterator iListIter;

    cout << "----------------" << endl << "Memory" << endl << "Address        |";
    
    for(int i = 0; i < memory.size(); i++)  // print memory address
    {
        cout << i << "|";
    }

    cout << endl << "Value          |";     // print value

    for(int bit : memory)
    {
        cout << bit << "|";
    }

    cout << endl <<"----------------" << endl << "Free Frames    :";
    
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

    for(Process p : processList)
    {
        cout << "PID " << p.pid << " ->";

        for(Page page : p.pages)
        {
            cout << page.num << (page.valid ? "T" : "F"); 
            cout << ",";
        }

        cout << endl;
    }

    cout << "LRU Stack" << endl;
    cout << "PID :";

    for(LRUData d : LRUStack)
    {
        cout << d.pid << "|";
    } 

    cout << endl << "Page:";

    for(LRUData d : LRUStack)
    {
        cout << d.page << "|";
    } 

    cout << endl;
}

void dbg(string s)
{
    cout << s << endl;
}