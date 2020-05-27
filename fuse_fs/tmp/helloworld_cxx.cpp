#include <string>

extern "C" const char *help_info_cxx()
{
	static std::string s(
		"File-system specific options:\n"
		"    --name=<s>          Name of the \"hello\" file\n"
		"                        (default: \"hello\")\n"
		"    --contents=<s>      Contents \"hello\" file\n"
		"                        (default \"Hello, World!\\n\")\n"
		"\n");
	return s.c_str();
}