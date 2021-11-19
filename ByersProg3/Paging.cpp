#include <iostream>
#include <list>

using namespace std;

void parseLine(string in, string* out);
void dbg(string s);

int main()
{
    string command[3] {};

    parseLine("a b c", command);
    parseLine("d e f", command);

    cout << command[0] << endl;
    

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
    //out = NULL;
    //out = new string[listOut.size()];
    //out = (string*)malloc(sizeof(string[listOut.size()]));
    listItr = listOut.begin();

    for(int i = 0; i < listOut.size(); i++)
    {
        out[i] = *listItr;
        advance(listItr, 1);
    }
}

void dbg(string s)
{
    cout << s << endl;
}