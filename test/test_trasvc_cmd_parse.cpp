#include <iostream>
#include <cstring>
#include <cstdio>

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
		for(int i = 0; trasvc_flag_list[i].flag > 0; i++)
		{
			printf("Have %s flag: %d\n", trasvc_flag_list[i].str, (flag & trasvc_flag_list[i].flag) > 0);
		}
		cout << endl;
	}

	return 0;
}
