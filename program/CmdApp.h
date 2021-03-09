#pragma once
//#include <initializer_list>
#include <utility>
#include <string_view>

using namespace std::literals;
using CommandHandler = void(*)();

class CmdApp
{
public:
	static void run(std::initializer_list<std::pair<std::string_view, CommandHandler>> list, std::string_view quit_cmd = "quit"sv);
};
