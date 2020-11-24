#ifndef JMWCLib_H
#define JMWCLib_H

#include <unordered_map>
#include <string>
#include <cassert>
#include <map>
#include <optional>

namespace x69
{
	enum class TOKEN_TYPE
	{
		TYPENAME,
		VARIABLE,
		OPERATOR,
		EXPRESSION,
		FUNCTION,
		SCOPE,
		EOL
	};

	struct LexedToken
	{
		std::string str;
	};
	
	
	
	
	struct Token
	{
		std::optional<TOKEN_TYPE> type;
		std::string str;
	};

	struct Type
	{
		size_t size = 0;
		std::string name = "";
	};

	struct Variable
	{
		uint16_t offset = 0;
		std::string name = "";
		std::string type = "";
		std::vector<Token> assignment{};
	};

	struct Function
	{
		std::string name = "";
		std::string return_type = "";
		std::vector<std::pair<std::string, std::string>> parameters;
		std::vector<Token> body;
	};

	struct ParseTree
	{
	private:
		struct Node
		{
			friend inline bool operator==(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator!=(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator<(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator>(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator<=(const Node& _lhs, const Node& _rhs) = default;
			friend inline bool operator>=(const Node& _lhs, const Node& _rhs) = default;

			friend inline bool operator==(const Node& _lhs, char _rhs)
			{
				return _lhs.c == _rhs;
			};
			friend inline bool operator!=(const Node& _lhs, char _rhs)
			{
				return _lhs.c != _rhs;
			};
			friend inline bool operator==(char _lhs, const Node& _rhs)
			{
				return _rhs == _lhs;
			};
			friend inline bool operator!=(char _lhs, const Node& _rhs)
			{
				return _rhs != _lhs;
			};

			char c;
			std::optional<TOKEN_TYPE> type;
			std::map<char, Node> children;

		};

		Node* find_node(const std::string& _str)
		{
			auto _at = &this->root_;
			assert(_at);

			for (size_t i = 0; i < _str.size(); ++i)
			{
				if (_at->children.contains(_str[i]))
				{
					_at = &_at->children.at(_str[i]);
				}
				else
				{
					return nullptr;
				};
			};

			return _at;
		};
		const Node* find_node(const std::string& _str) const
		{
			auto _at = &this->root_;
			assert(_at);

			for (size_t i = 0; i < _str.size(); ++i)
			{
				if (_at->children.contains(_str[i]))
				{
					_at = &_at->children.at(_str[i]);
				}
				else
				{
					return nullptr;
				};
			};

			return _at;
		};

	public:

		bool insert(const std::string& _str, TOKEN_TYPE _token)
		{
			Node* _at = &this->root_;

			for (size_t i = 0; i < _str.size(); ++i)
			{
				assert(_at);

				if (!_at->children.contains(_str[i]))
				{
					// Branch ends, extend it
					Node _newNode{};
					_newNode.c = _str[i];
					_at->children.insert({ _str[i], std::move(_newNode) });
				};

				_at = &_at->children.at(_str[i]);

			};

			assert(_at);
			if (_at->type)
			{
				return false;
			}
			else
			{
				_at->type = _token;
				return true;
			};

		};

		bool contains(const std::string& _str) const
		{
			return this->find_node(_str) != nullptr;
		};

		std::optional<TOKEN_TYPE> find(const std::string& _str)
		{
			auto _ptr = find_node(_str);
			if (_ptr)
			{
				return _ptr->type;
			}
			else
			{
				return std::nullopt;
			};
		};

		void clear()
		{
			this->root_.children.clear();
			this->root_.type = std::nullopt;
		};

	private:
		Node root_;

	};

	static inline const std::vector<char> FORCEFUL_TOKENS
	{
		'+', '=', '-', '<', '>', '!', '(', ')', '{', '}', ';', ','
	};

	struct IntegralTypes
	{
		static inline const Type t_i8{ 1, "i8" };
		static inline const Type t_i16{ 2, "i16" };

		static inline const Type t_u8{ 1, "u8" };
		static inline const Type t_u16{ 2, "u16" };
	};

	struct CompileContext
	{

		void push_variable(Variable _v)
		{
			this->ptree_.insert(_v.name, TOKEN_TYPE::VARIABLE);
			this->VARIABLES.insert({ _v.name, _v });
		};
		void push_type(Type _t)
		{
			this->ptree_.insert(_t.name, TOKEN_TYPE::TYPENAME);
			this->TYPES.insert({ _t.name, _t });
		};
		void push_function(Function _f)
		{
			this->ptree_.insert(_f.name, TOKEN_TYPE::FUNCTION);
			this->FUNCTIONS.insert({ _f.name, _f });
		};

		auto parse_token(std::string& _str)
		{
			return this->ptree_.find(_str);
		};

		ParseTree ptree_;

		std::unordered_map<std::string, Variable> VARIABLES
		{

		};

		std::unordered_map<std::string, Type> TYPES
		{

		};

		std::unordered_map<std::string, Function> FUNCTIONS
		{

		};


	};





}


#endif