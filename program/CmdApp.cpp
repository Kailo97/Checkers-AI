#include "CmdApp.h"
#include <iostream>

void CmdApp::run(std::initializer_list<std::pair<std::string_view, CommandHandler>> il, std::string_view quit_cmd)
{
	std::string input;
	for (;;)
	{
		std::cout << "> ";
		std::cin >> input;

		auto cmd = std::find_if(il.begin(), il.end(), [&input](const decltype(il)::value_type& entry) { return entry.first == input; });
		if (cmd != il.end())
			cmd->second();
		else if (input != quit_cmd)
			std::cout << "unknown command " << input << std::endl;

		if (input == quit_cmd)
			break;
	}
}
