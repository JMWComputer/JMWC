#include <JMWC.h>

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <string>
#include <optional>
#include <unordered_set>
#include <variant>
#include <map>
#include <sstream>
#include <optional>

namespace fs = std::filesystem;

std::ofstream _outFile{ SOURCE_ROOT "/out.o" };

size_t COUNTER = 0;

x69::CompileContext CONTEXT{};

using Token = x69::Token;

bool try_match(const std::string& _str, const std::string& _token, size_t _offset)
{
	size_t i = 0;
	for (; i + _offset < _str.size() && i < _token.size(); ++i)
	{
		if (_str[i + _offset] != _token[i])
		{
			return false;
		};
	};
	return true;
};

std::string& remove_leading_whitespace(std::string& _str)
{
	size_t i = 0;
	for (; i < _str.size(); ++i)
	{
		if (std::isspace(_str[i]))
		{
			continue;
		}
		else
		{
			_str.erase(0, i);
			return _str;
		};
	};
	return _str;
};
std::string& remove_trailing_whitespace(std::string& _str)
{
	int32_t i = _str.size() - 1;
	for (; i >= 0; --i)
	{
		if (std::isspace(_str[i]))
		{
			continue;
		}
		else
		{
			_str.erase(i, _str.size() - i);
			return _str;
		};
	};
	return _str;
};
std::string& remove_outer_whitespace(std::string& _str)
{
	return remove_trailing_whitespace(remove_leading_whitespace(_str));
};

template <typename _FwdIter, typename _Op>
_FwdIter rfind_if(const _FwdIter _begin, const _FwdIter _end, _Op _op)
{
	if (_end == _begin)
	{
		return _end;
	};

	auto it = _end - 1;
	for (; it != _begin; --it)
	{
		if (std::invoke(_op, *it))
		{
			return it;
		};
	};

	return (std::invoke(_op, *it)) ? it : _end;

};

template <typename _FwdIter, typename _Op>
std::vector<_FwdIter> find_all_if(_FwdIter _begin, const _FwdIter _end, _Op _op)
{
	auto _it = _begin;
	std::vector<_FwdIter> _out{};
	while ((_it = std::find_if(_it, _end, _op)) != _end)
	{
		_out.push_back(_it);
		++_it;
	};
	return _out;
};

std::pair<size_t, size_t> find_any_of(const std::string& _str, const std::vector<std::string>& _tokens, size_t _offset = 0)
{
	std::pair<size_t, size_t> _out{ std::string::npos, _tokens.size() };
	for (auto i = _offset; i < _str.size(); ++i)
	{
		for (auto t = 0; t < _tokens.size(); ++t)
		{
			if (try_match(_str, _tokens[t], i))
			{
				_out.first = i;
				_out.second = t;
				return _out;
			};
		};
	};
	return _out;
}; 

bool is_any_of(char _c, const std::vector<char>& _of)
{
	return std::find(_of.begin(), _of.end(), _c) != _of.end();
};

size_t find_nested_close(const std::string& _str, size_t _openPos, char _openToken, char _closeToken, size_t _tcount = 0)
{
	for (auto i = _openPos + 1; i < _str.size(); ++i)
	{
		if (_str[i] == _openToken)
		{
			i = find_nested_close(_str, i, _openToken, _closeToken, _tcount + 1);
		}
		else if (_str[i] == _closeToken)
		{
			return i;
		};
	};
	return std::string::npos;
};

std::vector<Token> tokenize(const std::string& _str, size_t _offset = 0)
{
	std::vector<Token> _out{};
	size_t _lastStart = 0;
	bool _readingToken = false;

	for (auto i = _offset; i < _str.size(); ++i)
	{
		if (!_readingToken)
		{
			if (!std::isspace(_str[i]))
			{
				_readingToken = true;
				_lastStart = i;
			}
			else
			{
				continue;
			};
		};
		
		if (is_any_of(_str[i], x69::FORCEFUL_TOKENS))
		{
			if (_lastStart != i)
			{
				auto _tstr = _str.substr(_lastStart, (i - _lastStart));
				auto _tokenOpt = CONTEXT.parse_token(_tstr);
				_out.push_back({ _tokenOpt, std::move(_tstr) });
			};

			auto _forceFull = std::string{ _str[i] };
			if (i + 1 < _str.size() && is_any_of(_str[i + 1], x69::FORCEFUL_TOKENS))
			{
				_forceFull.push_back(_str[i + 1]);
				if (CONTEXT.parse_token(_forceFull))
				{
					++i;
				}
				else
				{
					_forceFull.pop_back();
				};
			};
			auto _ffOpt = CONTEXT.parse_token(_forceFull);
			_out.push_back({ _ffOpt, std::move(_forceFull) });
				
			_readingToken = false;
		}
		else if (std::isspace(_str[i]))
		{
			auto _tstr = _str.substr(_lastStart, i - _lastStart);
			auto _tokenOpt = CONTEXT.parse_token(_tstr);
			_out.push_back({ _tokenOpt, std::move(_tstr) });
			_readingToken = false;
		};

	};

	if (_readingToken)
	{
		auto _tstr = _str.substr(_lastStart, _str.size() - _lastStart);
		auto _tokenOpt = CONTEXT.parse_token(_tstr);
		_out.push_back({ _tokenOpt, std::move(_tstr) });
	};

	return _out;
};

size_t find_eol(const std::string& _str, size_t _offset = 0)
{
	return _str.find(';', _offset);
};

uint16_t TOTAL_MEMOFFSET = 0;

size_t get_memory_base_address()
{
	return 0;
};



void handle_expression(const std::vector<Token>& _tokens)
{
	std::cout << "Parsing expression :\n";

	for (auto& t : _tokens)
	{
		std::cout << "'" << t.str << "'\n";
	};



};

void handle_variable_declaration(const std::vector<Token>& _tokens)
{
	x69::Variable _v{ .offset = TOTAL_MEMOFFSET, .name = _tokens[1].str, .type = _tokens.front().str };
	TOTAL_MEMOFFSET += CONTEXT.TYPES.at(_tokens.front().str).size;


	auto _opIt = std::find_if(_tokens.begin(), _tokens.end(), [](const auto& a)
		{
			return a.type && (*a.type == x69::TOKEN_TYPE::OPERATOR || *a.type == x69::TOKEN_TYPE::SCOPE);
		});

	if (_opIt != _tokens.end())
	{
		if (_opIt->str == "=")
		{
			// Variable has an assignment
			_v.assignment.assign(_opIt, _tokens.end());
		}
		else
		{
			abort();
		};
	};

	CONTEXT.push_variable(_v);

};

void handle_function_declaration(const std::vector<Token>& _tokens)
{
	std::cout << "Parsing function declaration :\n";

	auto _fname = _tokens[1].str;

	auto _bodyStart = std::find_if(_tokens.begin(), _tokens.end(), [](const auto& t) {
		return (bool)t.type && *t.type == x69::TOKEN_TYPE::SCOPE;
		});
	auto _bodyEnd = rfind_if(_tokens.begin(), _tokens.end(), [](const auto& t) {
		return (bool)t.type && *t.type == x69::TOKEN_TYPE::SCOPE;
		});

	auto _parametersStart = std::find_if(_tokens.begin(), _tokens.end(), [](const auto& t) {
		return (bool)t.type && *t.type == x69::TOKEN_TYPE::EXPRESSION;
		});
	auto _parametersEnd = rfind_if(_tokens.begin(), _tokens.end(), [](const auto& t) {
		return (bool)t.type && *t.type == x69::TOKEN_TYPE::EXPRESSION;
		});

	assert(_tokens.begin() + 2 == _parametersStart);
	
	x69::Function _func{ .name = _tokens.begin()[1].str, .return_type = _tokens.begin()[0].str };
	for (auto it = _parametersStart + 1; it < _parametersEnd; it += 2)
	{
		_func.parameters.push_back({ it[0].str, it[1].str });
	};

	if(_bodyStart + 1 < _bodyEnd)
		_func.body.assign(_bodyStart + 1, _bodyEnd);

	CONTEXT.push_function(_func);


};




std::ostream& cgen_set_memaddress(uint16_t _addr, uint8_t _lreg, uint8_t _hreg, std::ostream& _ostr)
{
	_ostr << "set r" << (int)_lreg << ", 0x" << std::hex << (_addr & 0x00FF) << std::dec << '\n';
	_ostr << "set r" << (int)_hreg << ", 0x" << std::hex << ((_addr & 0xFF00) >> 16) << std::dec << '\n';

	return _ostr;
};

std::ostream& cgen_variable_declaration(const x69::Variable& _v, std::ostream& _ostr)
{
	if (_v.assignment.empty())
	{
		return _ostr;
	};

	cgen_set_memaddress(get_memory_base_address() + _v.offset, 0, 1, _ostr);

	_ostr << "set r2, 0x0F\n";
	_ostr << "str r2, r0, r1\n";
	
	return _ostr;

};





void parse_token(const std::string& _str, size_t& _at)
{
	std::string _lineStr{};

	auto _lineStart = _at;
	auto _eolPos = find_eol(_str, _at);
	auto _curlyPos = _str.find('{', _at);

	if (_curlyPos < _eolPos)
	{
		// Find closing curly
		auto _curlyEnd = find_nested_close(_str, _curlyPos, '{', '}');
		assert(_curlyEnd != std::string::npos);

		if (_str[_curlyEnd + 1] == ';')
		{
			++_at;
			_eolPos = _curlyEnd + 1;
		}
		else
		{
			_eolPos = _curlyEnd;
		};
	}
	else if (_eolPos == std::string::npos)
	{
		_at = _eolPos;
		return;
	};

	_at = _eolPos;
	_lineStr = _str.substr(_lineStart, _eolPos - _lineStart);

	auto _tkns = tokenize(_lineStr);

	if (_tkns.size() == 0)
	{
		return;
	};

	if (_tkns.front().type == x69::TOKEN_TYPE::TYPENAME)
	{
		// This is any of...
		//	- Variable declaration and/or definition
		//  - Function declaration and/or definition

		// This is a function if parenthesis are given followed by a curly brace block

		auto _parenPos = _str.find('(', _lineStart);
		if (_parenPos < _eolPos)
		{
			auto _curlyPos = _str.find('{', _parenPos);
			if (_curlyPos < _eolPos && _parenPos < _eolPos)
			{
				auto _closingParenPos = _str.rfind(')', _curlyPos);
				assert(_closingParenPos > _parenPos && _closingParenPos < _curlyPos);

				// Likely a function declaration
				handle_function_declaration(_tkns);



				return;
			}

		}		
	
		// Likely a variable declaration


		handle_variable_declaration(_tkns);





	}
	else if (_tkns.front().type == x69::TOKEN_TYPE::VARIABLE)
	{
		// This is any of...
		//	- Expression

		handle_expression(_tkns);

	}






	return;

};

void skip_block_comment(const std::string& _str, size_t& _at)
{
	auto _got = _str.find("*/", _at);
	_at = _got;
};
void skip_line_comment(const std::string& _str, size_t& _at)
{
	auto _got = _str.find('\n', _at);
	_at = _got;
};

void parse_code_stream(const std::string& _str)
{
	size_t _at = 0;
	size_t _len = 0;

	constexpr char _commentChar = '/';

	for (; _at < _str.size(); ++_at)
	{
		switch (_str[_at])
		{
		case _commentChar:
			
			switch(_str[_at + 1])
			{
			case '/':
				// single line comment
				skip_line_comment(_str, _at);
				break;
			case '*':
				// block comment
				skip_block_comment(_str, _at);
				break;
			default:
				break;
			};

			break;

		case ' ': 
			[[fallthrough]];
		case '\t': 
			[[fallthrough]];
		case '\n':
			break;
		
		default:
			parse_token(_str, _at);
			break;
		};
	};



};




std::vector<x69::LexedToken> find_all_tokens(const std::string& _str)
{
	std::vector<x69::LexedToken> _out{};
	size_t _lastStart = 0;
	bool _readingToken = false;

	for (auto i = 0; i < _str.size(); ++i)
	{
		if (!_readingToken)
		{
			if (!std::isspace(_str[i]))
			{
				_readingToken = true;
				_lastStart = i;
			}
			else
			{
				continue;
			};
		};

		if (is_any_of(_str[i], x69::FORCEFUL_TOKENS))
		{
			if (_lastStart != i)
			{
				auto _tstr = _str.substr(_lastStart, (i - _lastStart));
				auto _tokenOpt = CONTEXT.parse_token(_tstr);
				_out.push_back({ std::move(_tstr) });
			};

			auto _forceFull = std::string{ _str[i] };
			if (i + 1 < _str.size() && is_any_of(_str[i + 1], x69::FORCEFUL_TOKENS))
			{
				_forceFull.push_back(_str[i + 1]);
				if (CONTEXT.parse_token(_forceFull))
				{
					++i;
				}
				else
				{
					_forceFull.pop_back();
				};
			};
			auto _ffOpt = CONTEXT.parse_token(_forceFull);
			_out.push_back({ std::move(_forceFull) });

			_readingToken = false;
		}
		else if (std::isspace(_str[i]))
		{
			auto _tstr = _str.substr(_lastStart, i - _lastStart);
			auto _tokenOpt = CONTEXT.parse_token(_tstr);
			_out.push_back({std::move(_tstr) });
			_readingToken = false;
		};

	};

	if (_readingToken)
	{
		auto _tstr = _str.substr(_lastStart, _str.size() - _lastStart);
		auto _tokenOpt = CONTEXT.parse_token(_tstr);
		_out.push_back({ std::move(_tstr) });
	};

	return _out;

};





int main()
{
	CONTEXT.push_type(x69::IntegralTypes::t_i8);
	CONTEXT.push_type(x69::IntegralTypes::t_i16);

	CONTEXT.push_type(x69::IntegralTypes::t_u8);
	CONTEXT.push_type(x69::IntegralTypes::t_u16);

	CONTEXT.ptree_.insert("=", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert("+", x69::TOKEN_TYPE::OPERATOR);
	CONTEXT.ptree_.insert("+=", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert("-", x69::TOKEN_TYPE::OPERATOR);
	CONTEXT.ptree_.insert("-=", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert(">", x69::TOKEN_TYPE::OPERATOR);
	CONTEXT.ptree_.insert(">=", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert("<", x69::TOKEN_TYPE::OPERATOR);
	CONTEXT.ptree_.insert("<=", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert("!=", x69::TOKEN_TYPE::OPERATOR);
	CONTEXT.ptree_.insert("==", x69::TOKEN_TYPE::OPERATOR);

	CONTEXT.ptree_.insert("!", x69::TOKEN_TYPE::OPERATOR);


	CONTEXT.ptree_.insert(",", x69::TOKEN_TYPE::OPERATOR);


	CONTEXT.ptree_.insert("(", x69::TOKEN_TYPE::EXPRESSION);
	CONTEXT.ptree_.insert(")", x69::TOKEN_TYPE::EXPRESSION);

	CONTEXT.ptree_.insert("{", x69::TOKEN_TYPE::SCOPE);
	CONTEXT.ptree_.insert("}", x69::TOKEN_TYPE::SCOPE);


	CONTEXT.ptree_.insert(";", x69::TOKEN_TYPE::EOL);


	fs::path _srcPath = SOURCE_ROOT;
	_srcPath.append("sample.jmw");

	assert(fs::exists(_srcPath));

	std::fstream _fstr(_srcPath);
	assert(_fstr.is_open());

	char _buff[256]{ 0 };
	std::string _str{};

	while (!_fstr.eof())
	{
		_fstr.read(_buff, sizeof(_buff));
		_str.append(_buff, _fstr.gcount());
	};

	auto _tks = find_all_tokens(_str);
	for (auto& t : _tks)
	{
		std::cout << "'" << t.str << "'\n";
	};



	parse_code_stream(_str);

	std::ofstream _ofstr{ SOURCE_ROOT "/out.x69" };

	for (auto& v : CONTEXT.VARIABLES)
	{
		std::cout << "variable : " << v.second.type << " " << v.second.name;
		if (!v.second.assignment.empty())
		{
			cgen_variable_declaration(v.second, _ofstr);
			_ofstr << '\n';

			for (auto& t : v.second.assignment)
			{
				std::cout << ' ' << t.str;
			};
		
		};

		std::cout << '\n';
	};
	for (auto& f : CONTEXT.FUNCTIONS)
	{
		std::cout << "functions : " << f.second.return_type << " " << f.second.name << "()\n{\n\t";
		for (auto& t : f.second.body)
		{
			std::cout << t.str << ' ';
			if (t.type == x69::TOKEN_TYPE::EOL)
			{
				std::cout << "\t\n";
			}
		};
		std::cout << "};\n";
	};




	return 0;
}

