#include <iostream>
#include <cstring>

#include <trasvc.h>
#include <trasvc_private.h>

using namespace std;

int main()
{
	int flag;
	string tmp;

	while(1)
	{
		// Get user input
		cout << "Enter a command: ";
		getline(cin, tmp);

		// Parse cmd
		flag = trasvc_cmd_parse((char*)tmp.c_str());
		if(flag < 0)
		{
			cout << "trasvc_cmd_parse() failed with error: " << flag << endl;
			cout << endl;
			continue;
		}

		// Show flag
		cout << "Have HEAD flag: " << ((flag & TRASVC_CMD_HEAD_FLAG) > 0) << endl;
		cout << "Have APPEND flag: " << ((flag & TRASVC_CMD_APPEND_FLAG) > 0) << endl;
		cout << "Have OK flag: " << ((flag & TRASVC_CMD_OK_FLAG) > 0) << endl;
		cout << endl;
	}

	return 0;
}
