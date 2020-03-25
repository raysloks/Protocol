#include "Token.h"

const std::string control = "()[]{};,.";
const std::string equal_ops = "|<>!&=+-*/";
const std::string double_ops = "|&+-";

std::string extract_token(std::istringstream& ss)
{
	std::string lexeme;

	int type = -1;
	char ssc;
	while (true)
	{
		char c;
		ss.get(c);
		if (ss.eof())
			break;
		if (type == -1)
		{
			if (!isspace(c))
			{
				lexeme.append(1, c);
				if (isalpha((unsigned char)c) || c == '_')
					type = 0;
				if (isdigit((unsigned char)c))
					type = 1;
				if (c == '"' || c == '\'')
				{
					type = 2;
					ssc = c;
				}
				if (equal_ops.find(c) != std::string::npos)
					type = 4;
				if (double_ops.find(c) != std::string::npos)
				{
					type = 5;
					ssc = c;
				}
				if (control.find(c) != std::string::npos)
					break;
			}
		}
		else
		{
			if (type == 0)
			{
				if (isalpha((unsigned char)c) || isdigit((unsigned char)c) || c == '_')
				{
					lexeme.append(1, c);
				}
				else
				{
					ss.unget();
					break;
				}
			}
			if (type == 1)
			{
				if (isdigit((unsigned char)c) || c == '.')
				{
					lexeme.append(1, c);
				}
				else
				{
					ss.unget();
					break;
				}
			}
			if (type == 2)
			{
				if (c == '\\')
				{
					type = 3;
				}
				else
				{
					if (c != ssc)
						lexeme.append(1, c);
					else
						break;
				}
			}
			if (type == 3)
			{
				lexeme.append(1, c);
				type = 2;
			}
			if (type == 4)
			{
				if (c == '=')
					lexeme.append(1, c);
				else
					ss.unget();
				break;
			}
			if (type == 5)
			{
				if (c == ssc)
					lexeme.append(1, c);
				else
					ss.unget();
				break;
			}
		}
	}

	return lexeme;
}

std::vector<std::string> extract_tokens(std::istringstream& ss)
{
	std::vector<std::string> tokens;

	auto token = extract_token(ss);
	while (token.size())
	{
		tokens.push_back(token);
		token = extract_token(ss);
	}

	return tokens;
}
