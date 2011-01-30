//Notes:
//CJH - 01/28/2011    This header was entirely written by Igor.

#ifndef TOOLS_H_
#define TOOLS_H_

#include <string>

namespace tools
{
std::string my_trim_r(const std::string &str, const std::string &tr)
{
	std::string ret = str;
	if (!ret.empty())
		ret.erase(ret.find_last_not_of(tr)+1);

	return ret;
}

std::string my_trim_l(const std::string &str, const std::string &tr)
{
	std::string ret = str;

	if (!ret.empty())
		ret.erase(0, ret.find_first_not_of(tr));

	return ret;
}

std::string my_trim(const std::string &str, const std::string &tr)
{
	return (my_trim_r(my_trim_l(str,tr),tr));
}

std::vector<std::string> split_string(std::string src, char delimiter, bool skip_empty)
{
	std::vector<std::string> res;

	if (src.length() < 3)
	{
		res.push_back(src);
		return res;
	}

	int t_pos = 0;
	int t_opos = 0;

	while( (t_pos = src.find_first_of(delimiter,t_pos)) != -1 )
	{
		if(t_pos > 0)
		{
			if (!skip_empty || (skip_empty && ((t_pos-t_opos) > 0)))
				res.push_back(std::string(src.substr(t_opos,t_pos-t_opos)));
			t_pos++;
			t_opos = t_pos;
		}
	}

	if(t_opos > 0)
	{
		res.push_back(std::string(src.substr(t_opos,src.size() - t_opos)));
	}

	return res;
}
}

#endif /* TOOLS_H_ */
