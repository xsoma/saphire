#include "Kit_parser.h"
#include <Windows.h>
#include "includes.hpp"
#include <algorithm>
#include <deque>
#include "nSkinz/SkinChanger.h"


std::vector<game_data::paint_kit> game_data::skin_kits;
std::vector<game_data::paint_kit> game_data::glove_kits;
std::vector<game_data::paint_kit> game_data::sticker_kits;


class CCStrike15ItemSchema;
class CCStrike15ItemSystem;

template <typename Key, typename Value>
struct Node_t
{
	int previous_id;		//0x0000
	int next_id;			//0x0004
	void* _unknown_ptr;		//0x0008
	int _unknown;			//0x000C
	Key key;				//0x0010
	Value value;			//0x0014
};

template <typename Key, typename Value>
struct Head_t
{
	Node_t<Key, Value>* memory;		//0x0000
	int allocation_count;			//0x0004
	int grow_size;					//0x0008
	int start_element;				//0x000C
	int next_available;				//0x0010
	int _unknown;					//0x0014
	int last_element;				//0x0018
}; //Size=0x001C

// could use CUtlString but this is just easier and CUtlString isn't needed anywhere else
struct String_t
{
	char* buffer;	//0x0000
	int capacity;	//0x0004
	int grow_size;	//0x0008
	int length;		//0x000C
}; //Size=0x0010

struct CPaintKit
{
	int id;						//0x0000

	String_t name;				//0x0004
	String_t description;		//0x0014
	String_t item_name;			//0x0024
	String_t material_name;		//0x0034
	String_t image_inventory;	//0x0044

	char pad_0x0054[0x8C];		//0x0054
}; //Size=0x00E0

struct CStickerKit
{
	int id;

	int item_rarity;

	String_t name;
	String_t description;
	String_t item_name;
	String_t material_name;
	String_t image_inventory;

	int tournament_event_id;
	int tournament_team_id;
	int tournament_player_id;
	bool is_custom_sticker_material;

	float rotate_end;
	float rotate_start;

	float scale_min;
	float scale_max;

	float wear_min;
	float wear_max;

	String_t image_inventory2;
	String_t image_inventory_large;

	std::uint32_t pad0[4];
};

int GetWeaponRarity(std::string rarity)
{
	if (rarity == "default")
		return 0;
	else if (rarity == "common")
		return 1;
	else if (rarity == "uncommon")
		return 2;
	else if (rarity == "rare")
		return 3;
	else if (rarity == "mythical")
		return 4;
	else if (rarity == "legendary")
		return 5;
	else if (rarity == "ancient")
		return 6;
	else if (rarity == "immortal")
		return 7;
	else if (rarity == "unusual")
		return 99;

	return 0;
}

namespace valve_parser
{
	class Document;
	class KeyValue;
	class Object;

	enum ENCODING
	{
		UTF8,
		UTF16_LE,
		UTF16_BE,
		UTF32_LE,
		UTF32_BE,
		MAX
	};

	enum
	{
		STRING = '"',
		OBJECT_OPEN = '{',
		OBJECT_CLOSE = '}',
		COMMENT = '/',
	};


	class Str
	{
	public:
		char32_t* _start = 0;
		char32_t* _end = 0;
	public:
		std::string toString()
		{
			if (!_start || !_end)
				return "";

			//convert utf32 to utf16
			std::vector<char16_t> utf16;
			utf16.reserve(200);

			for (char32_t* text = _start; text != _end + 1; text++)
			{
				if ((*text >= 0 && *text <= 0xD7FF) ||
					*text >= 0xE000 && *text <= 0xFFFF)
				{
					utf16.push_back(*text);
				}
				if (*text >= 0x10000 && *text <= 0x10FFFF)
				{
					char32_t offset = *text - 0x10000;
					char16_t hi = (offset & 0xFFC00) >> 10;
					char16_t lo = offset & 0x3FF;
					hi += 0xD800;
					lo += 0xDC00;
					utf16.push_back(hi);
					utf16.push_back(lo);
				}
			}
			return std::string(utf16.begin(), utf16.end());
		}

		//warning: no exception handling
		int toInt()
		{
			return std::stoi(toString());
		}

		//checks whether the next read character equals expectedTag, if not 0 is returned
		static char32_t* ParseTextExpectedTag(char32_t* p, char32_t expectedTag, bool IGNORE_SPACE_TAB_LF_CR)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n'; p++);
				}

				//skip options line
				// [§xbox|§360]
				if (*p == '[')
				{
					for (; *p && *p != ']'; p++);
					p++;
				}

				if (IGNORE_SPACE_TAB_LF_CR)
				{
					if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
						continue;
				}

				if (*p == expectedTag)
					return p;
				else
					return 0;
			}
			return 0;
		}

		static char32_t* EndReached(char32_t* p, char32_t expectedTag)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n'; p++);
				}

				//skip options line
				// [§xbox|§360]
				if (*p == '[')
				{
					for (; *p && *p != ']'; p++);
					p++;
				}

				if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
					continue;

				if (*p == expectedTag)
					return p;
				else
					return (char32_t*)-1;
			}
			return 0;
		}

		//reads string until it ecnounters endTag
		static char32_t* ParseTextEndTag(char32_t* p, char32_t endTag)
		{
			for (; *p; p++)
			{
				//skip escape sequence
				//there are also hex escapes
				//\xAA45, no need to handle them tho
				if (*p == '\\')
					p += 2;

				if (*p == endTag)
					return p;
			}
			return 0;
		}
	};

	class Util
	{
	public:
		static bool StrEqu(Str str1, char* str2)
		{
			return str1.toString() == std::string(str2);
		}

		static bool StrEquI(Str str1, char* str2)
		{
			std::string str = str1.toString();
			return str.compare(str2) == std::string::npos;
		}
	};

	class Node
	{
	protected:
		Document* _doc;
	public:
		std::vector<std::shared_ptr<Node>> children;
		virtual bool Parse();

		Node(Document* doc)
		{
			_doc = doc;
		}

		virtual Object* ToObject()
		{
			return 0;
		}

		virtual KeyValue* ToKeyValue()
		{
			return 0;
		}
	};

	class KeyValue : public Node
	{
	public:
		Str Key;
		Str Value;

		KeyValue(Document* doc) : Node(doc) {}

		KeyValue* ToKeyValue()
		{
			return this;
		}
	};

	class Object : public Node
	{
	public:
		Str name;
		Object(Document* doc);
		Object* ToObject();
		bool Parse();
		std::shared_ptr<Node> GetKeyByName(char* name);
	};

	class Document
	{
	private:
		friend class Object;
		friend class Node;

		struct BOM
		{
			char* str;
			size_t len;
		};
		BOM BOMS[ENCODING::MAX];

		char32_t* utf32text = 0;
		char32_t* p;
	public:
		std::shared_ptr<Node> root;

		Document()
		{
			BOMS[UTF8] = { "\xEF\xBB\xBF", 3 };
			BOMS[UTF16_LE] = { "\xFF\xFE", 2 };
			BOMS[UTF16_BE] = { "\xFE\xFF", 2 };
			BOMS[UTF32_LE] = { "\xFF\xFE\x00\x00", 4 };
			BOMS[UTF32_BE] = { "\x00\x00\xFE\xFF", 4 };
		}

		~Document()
		{
			if (utf32text)
				delete[] utf32text;
		}

		std::shared_ptr<Node> BreadthFirstSearch(char* name, bool caseInsensitive = false)
		{
			std::deque<std::shared_ptr<Node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				std::shared_ptr<Node> f = q.front();
				q.pop_front();

				if (f->ToKeyValue())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToKeyValue()->Key, name))
						return f;

					if (caseInsensitive && Util::StrEquI(f->ToKeyValue()->Key, name))
						return f;
				}
				if (f->ToObject())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToObject()->name, name))
						return f;

					if (caseInsensitive && Util::StrEquI(f->ToObject()->name, name))
						return f;
				}

				for (auto& child : f->children)
					q.push_back(child);
			}
			return 0;
		}

		std::vector<std::shared_ptr<Node>> BreadthFirstSearchMultiple(char* name, bool caseInsensitive = false)
		{
			std::vector<std::shared_ptr<Node>> vec;
			std::deque<std::shared_ptr<Node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				std::shared_ptr<Node> f = q.front();
				q.pop_front();

				if (f->ToKeyValue())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToKeyValue()->Key, name))
						vec.push_back(f);

					if (caseInsensitive && Util::StrEquI(f->ToKeyValue()->Key, name))
						vec.push_back(f);
				}
				if (f->ToObject())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToObject()->name, name))
						vec.push_back(f);

					if (caseInsensitive && Util::StrEquI(f->ToObject()->name, name))
						vec.push_back(f);
				}

				for (auto& child : f->children)
					q.push_back(child);
			}
			return vec;
		}

		//returns true when format is correct
		//parse from file
		bool Load(char* path, ENCODING encoding)
		{
			auto f = fopen(path, "rb");
			if (!f)
				return false;

			fseek(f, 0, SEEK_END);
			auto size = ftell(f);
			if (!size)
				return false;

			size_t null_terminator_bytes = 0;
			if (encoding == ENCODING::UTF16_BE ||
				encoding == ENCODING::UTF16_LE)
				null_terminator_bytes = 2;

			if (encoding == ENCODING::UTF8)
				null_terminator_bytes = 1;

			if (encoding == ENCODING::UTF32_BE ||
				encoding == ENCODING::UTF32_LE)
				null_terminator_bytes = 4;

			char* _text = new char[size + null_terminator_bytes];
			fseek(f, 0, SEEK_SET);
			if (fread(_text, 1, size, f) != size)
				return false;

			for (size_t i = 0; i < null_terminator_bytes; i++)
				_text[size + i] = 0;

			fclose(f);

			//convert to utf32
			//which makes everything easy to handle
			//but increases buffer size
			cnv_to_utf32(_text, encoding);
			delete[] _text;

			p = trimSpace(utf32text);
			if (!p)
				return false;
			return ParseDeep();
		}
	private:
		char* skipBOM(char* p, ENCODING encoding)
		{
			if (!memcmp(p, BOMS[encoding].str, BOMS[encoding].len))
				return p + BOMS[encoding].len;
			else
				return p;
		}

		void cnv_to_utf32(char* text, ENCODING encoding)
		{
			if (encoding == ENCODING::UTF16_LE)
			{
				char* bom = skipBOM(text, ENCODING::UTF16_LE);
				cnv_utf16_le(bom);
			}

			if (encoding == ENCODING::UTF16_BE)
			{
				char* bom = skipBOM(text, ENCODING::UTF16_BE);
				cnv_utf16_be(bom);
			}

			if (encoding == ENCODING::UTF8)
			{
				char* bom = skipBOM(text, ENCODING::UTF8);
				cnv_utf8(bom);
			}

			if (encoding == ENCODING::UTF32_LE)
			{
				char* bom = skipBOM(text, ENCODING::UTF32_LE);
				cnv_utf32_le(bom);
			}

			if (encoding == ENCODING::UTF32_BE)
			{
				char* bom = skipBOM(text, ENCODING::UTF32_BE);
				cnv_utf32_be(bom);
			}
		}

		size_t cnv_utf16_be_len(char* text)
		{
			size_t len = 0;
			char16_t* u16text = (char16_t*)text;
			while (*u16text)
			{

				char16_t c = 0;
				char* t = (char*)u16text;
				c |= (char16_t)*(t + 1) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
					u16text++;

				len++;
				u16text++;
			}
			return len;
		}

		size_t cnv_utf16_le_len(char* text)
		{
			size_t len = 0;
			char16_t* u16text = (char16_t*)text;
			while (*u16text)
			{
				if (*u16text >= 0xD800 && *u16text <= 0xDFFF)
					u16text++;

				len++;
				u16text++;
			}
			return len;
		}

		void cnv_utf16_be(char* text)
		{
			auto s = cnv_utf16_be_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char16_t* u16text = (char16_t*)text;
			char32_t k;

			while (*u16text)
			{

				char16_t c = 0;
				char* t = (char*)u16text;
				c |= (char16_t)*(t + 1) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
				{
					char16_t hi = c - 0xD800;

					t = (char*)(u16text + 1);
					c |= (char16_t)*(t + 1) << 8;
					c |= *t;
					char16_t lo = c - 0xDC00;

					k = (char32_t)(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16text++;
				}
				else //2 bytes
				{
					k = c;
				}
				utf32text[i] = k;
				i++;
				u16text++;
			}
		}

		void cnv_utf16_le(char* text)
		{
			auto s = cnv_utf16_le_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char16_t* u16text = (char16_t*)text;
			char32_t k;

			while (*u16text)
			{
				//4 bytes
				if (*u16text >= 0xD800 && *u16text <= 0xDFFF)
				{
					char16_t hi = *u16text - 0xD800;
					char16_t lo = *(u16text + 1) - 0xDC00;
					k = (char32_t)(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16text++;
				}
				else //2 bytes
				{
					k = *u16text;
				}
				utf32text[i] = k;
				i++;
				u16text++;
			}
		}

		size_t cnv_utf8_len(char* text)
		{
			size_t len = 0;
			unsigned char c = (unsigned char)*text;

			while (c)
			{
				if (c >= 0xc0 && c <= 0xdf)
				{
					text++;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					text += 2;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					text += 3;
				}

				len++;
				c = *++text;
			}

			return len;
		}

		void cnv_utf8(char* text)
		{
			auto s = cnv_utf8_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			unsigned char c = (unsigned char)*text;

			while (c)
			{
				char32_t k = 0;
				if (c >= 0 && c <= 0x7f)
				{
					k = c;
				}
				if (c >= 0xc0 && c <= 0xdf)
				{
					k = (char32_t)(c ^ 0xc0) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					k = (char32_t)(c ^ 0xe0) << 12;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					k = (char32_t)(c ^ 0xf0) << 18;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 12;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				utf32text[i] = k;
				i++;
				c = *++text;
			}
		}

		size_t cnv_utf32_len(char* text)
		{
			size_t len = 0;
			char32_t* p = (char32_t*)text;

			while (*p)
			{
				len++;
				p++;
			}

			return len;
		}

		void cnv_utf32_le(char* text)
		{
			auto s = cnv_utf32_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char32_t* p = (char32_t*)text;

			while (*p)
			{
				utf32text[i] = *p;
				i++;
				p++;
			}
		}

		void cnv_utf32_be(char* text)
		{
			auto s = cnv_utf32_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char32_t* p = (char32_t*)text;
			char32_t k;

			while (*p)
			{
				char* t = (char*)p;
				k = (char32_t)*(t + 3) << 24;
				k |= (char32_t)*(t + 2) << 16;
				k |= (char32_t)*(t + 1) << 8;
				k |= *t;

				utf32text[i] = k;
				i++;
				p++;
			}
		}

		char32_t* trimSpace(char32_t* p)
		{
			while (*p)
			{
				if (*p == ' ')
					p++;
				else
					return p;
			}

			return 0;
		}

		bool identify(std::shared_ptr<Node>& node)
		{
			auto string_begin = Str::ParseTextExpectedTag(p, STRING, true);
			if (!string_begin)
				return false;

			auto string_end = Str::ParseTextEndTag(string_begin + 1, STRING);
			if (!string_end)
				return false;


			auto object_open = Str::ParseTextExpectedTag(string_end + 1, OBJECT_OPEN, true);
			auto val_start = Str::ParseTextExpectedTag(string_end + 1, STRING, true);

			if (!object_open && !val_start)
				return false;

			if (object_open)
			{
				std::shared_ptr<Object> obj = std::make_shared<Object>(this);
				obj->name._start = string_begin + 1;
				obj->name._end = string_end - 1;
				node = obj;
				p = object_open + 1;
				return true;
			}

			if (val_start)
			{
				auto val_end = Str::ParseTextEndTag(val_start + 1, STRING);
				if (!val_end)
					return false;

				std::shared_ptr<KeyValue> keyVal = std::make_shared<KeyValue>(this);
				keyVal->Key._start = string_begin + 1;
				keyVal->Key._end = string_end - 1;
				keyVal->Value._start = val_start + 1;
				keyVal->Value._end = val_end - 1;
				node = keyVal;
				p = val_end + 1;
				return true;
			}
			return false;
		}

		bool ParseDeep()
		{
			root = std::make_shared<Node>(this);
			return root->Parse();
		}
	};
}

namespace valve_parser
{
	Object::Object(Document* doc) : Node(doc) {}

	Object* Object::ToObject()
	{
		return this;
	}

	std::shared_ptr<Node> Object::GetKeyByName(char* name)
	{
		for (auto& child : children)
		{
			if (child->ToKeyValue())
			{
				if (Util::StrEqu(child->ToKeyValue()->Key, name))
					return child;
			}
		}

		return 0;
	}

	bool Object::Parse()
	{
		std::shared_ptr<Node> n;

		while (*_doc->p)
		{
			//check for object close
			auto string_begin = Str::ParseTextExpectedTag(_doc->p, STRING, true);
			if (!string_begin)
			{
				auto obj_close = Str::ParseTextExpectedTag(_doc->p, OBJECT_CLOSE, true);
				if (obj_close)
				{
					_doc->p = obj_close + 1;
					return true;
				}
				else
					return false;
			}

			if (!_doc->identify(n))
				return false;

			if (n->ToKeyValue())
			{
				this->children.push_back(n);
			}

			if (n->ToObject())
			{
				this->children.push_back(n);
				Object* obj = n->ToObject();
				if (!obj->Parse())
					return false;
			}
		}
		return false;
	}

	bool Node::Parse()
	{
		std::shared_ptr<Node> n;

		while (*_doc->p)
		{
			if (!_doc->identify(n))
			{
				if (!Str::EndReached(_doc->p, OBJECT_OPEN) &&
					!Str::EndReached(_doc->p, OBJECT_CLOSE) &&
					!Str::EndReached(_doc->p, STRING))
					return true;
				else
					return false;
			}

			if (n->ToKeyValue())
			{
				this->children.push_back(n);
			}

			if (n->ToObject())
			{
				this->children.push_back(n);
				Object* obj = n->ToObject();
				if (!obj->Parse())
					return false;
			}
		}
		return false;
	}
}


auto game_data::initialize_kits() -> void
{
	const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "V_UCS2ToUTF8")));

	// Search the relative calls

	// call    ItemSystem
	// push    dword ptr [esi+0Ch]
	// lea     ecx, [eax+4]
	// call    CEconItemSchema::GetPaintKitDefinition

	const auto sig_address = util::FindSignature("client.dll", "E8 ?? ?? ?? ?? FF 76 0C 8D 48 04 E8");

	// Skip the opcode, read rel32 address
	const auto item_system_offset = *reinterpret_cast<std::int32_t*>(sig_address + 1);

	// Add the offset to the end of the instruction
	const auto item_system_fn = reinterpret_cast<CCStrike15ItemSystem * (*)()>(sig_address + 5 + item_system_offset);

	// Skip VTable, first member variable of ItemSystem is ItemSchema
	const auto item_schema = reinterpret_cast<CCStrike15ItemSchema*>(std::uintptr_t(item_system_fn()) + sizeof(void*));

	// Dump paint kits
	{
		// Skip the instructions between, skip the opcode, read rel32 address
		const auto get_paint_kit_definition_offset = *reinterpret_cast<std::int32_t*>(sig_address + 11 + 1);

		// Add the offset to the end of the instruction
		const auto get_paint_kit_definition_fn = reinterpret_cast<CPaintKit * (__thiscall*)(CCStrike15ItemSchema*, int)>(sig_address + 11 + 5 + get_paint_kit_definition_offset);

		// The last offset is start_element, we need that

		// push    ebp
		// mov     ebp, esp
		// sub     esp, 0Ch
		// mov     eax, [ecx+298h]

		// Skip instructions, skip opcode, read offset
		const auto start_element_offset = *reinterpret_cast<std::intptr_t*>(std::uintptr_t(get_paint_kit_definition_fn) + 8 + 2);

		// Calculate head base from start_element's offset
		const auto head_offset = start_element_offset - 12;

		const auto map_head = reinterpret_cast<Head_t<int, CPaintKit*>*>(std::uintptr_t(item_schema) + head_offset);

		for (auto i = 0; i <= map_head->last_element; ++i)
		{
			const auto paint_kit = map_head->memory[i].value;

			if (paint_kit->id == 9001)
				continue;

			const auto wide_name = m_localize()->Find(paint_kit->item_name.buffer + 1);
			char name[256];
			V_UCS2ToUTF8(wide_name, name, sizeof(name));

			if (paint_kit->id < 10000)
				game_data::skin_kits.push_back({ paint_kit->id, name });
			else
				game_data::glove_kits.push_back({ paint_kit->id, name });
		}

		std::sort(game_data::skin_kits.begin(), game_data::skin_kits.end());
		std::sort(game_data::glove_kits.begin(), game_data::glove_kits.end());
	}

	// Dump sticker kits
	{
		const auto sticker_sig = util::find_pattern("client.dll", "\x53\x8D\x48\x04\xE8\x00\x00\x00\x00\x8B\x4D\x10", "xxxxx????xxx") + 4;
		// Skip the opcode, read rel32 address
		const auto get_sticker_kit_definition_offset = *reinterpret_cast<std::intptr_t*>(sticker_sig + 1);
		// Add the offset to the end of the instruction
		const auto get_sticker_kit_definition_fn = reinterpret_cast<CPaintKit * (__thiscall*)(CCStrike15ItemSchema*, int)>(sticker_sig + 5 + get_sticker_kit_definition_offset);
		// The last offset is head_element, we need that
		//	push    ebp
		//	mov     ebp, esp
		//	push    ebx
		//	push    esi
		//	push    edi
		//	mov     edi, ecx
		//	mov     eax, [edi + 2BCh]
		// Skip instructions, skip opcode, read offset
		const auto start_element_offset = *reinterpret_cast<intptr_t*>(std::uintptr_t(get_sticker_kit_definition_fn) + 8 + 2);
		// Calculate head base from start_element's offset
		const auto head_offset = start_element_offset - 12;
		const auto map_head = reinterpret_cast<Head_t<int, CStickerKit*>*>(std::uintptr_t(item_schema) + head_offset);
		for (auto i = 0; i <= map_head->last_element; ++i)
		{
			const auto sticker_kit = map_head->memory[i].value;
			char sticker_name_if_valve_fucked_up_their_translations[64];
			auto sticker_name_ptr = sticker_kit->item_name.buffer + 1;
			if (strstr(sticker_name_ptr, "StickerKit_dhw2014_dignitas"))
			{
				strcpy_s(sticker_name_if_valve_fucked_up_their_translations, "StickerKit_dhw2014_teamdignitas");
				strcat_s(sticker_name_if_valve_fucked_up_their_translations, sticker_name_ptr + 27);
				sticker_name_ptr = sticker_name_if_valve_fucked_up_their_translations;
			}
			const auto wide_name = m_localize()->Find(sticker_name_ptr);
			char name[256];
			V_UCS2ToUTF8(wide_name, name, sizeof(name));
			std::string szIconPathLarge = sticker_kit->image_inventory_large.buffer;
			int rarity = sticker_kit->item_rarity;
			if (!szIconPathLarge.empty())
				szIconPathLarge = std::string("resource/flash/").append(szIconPathLarge).append(".png");


			game_data::sticker_kits.push_back({ sticker_kit->id, name, szIconPathLarge, rarity });
		}
		std::sort(game_data::sticker_kits.begin(), game_data::sticker_kits.end());
		game_data::sticker_kits.insert(game_data::sticker_kits.begin(), { 0, "None" });
	}
	ParseSkins();
}


int game_data::ParseSkins()
{
	valve_parser::Document doc;
	auto r = doc.Load(".\\csgo\\scripts\\items\\items_game.txt", valve_parser::ENCODING::UTF8);
	if (!r)
		return 1;

	valve_parser::Document english;
	r = english.Load(".\\csgo\\resource\\csgo_english.txt", valve_parser::ENCODING::UTF16_LE);
	if (!r)
		return 2;

	auto weaponSkinCombo = doc.BreadthFirstSearch("weapon_icons");
	if (!weaponSkinCombo || !weaponSkinCombo->ToObject())
		return 3;

	auto paintKitsRarity = doc.BreadthFirstSearchMultiple("paint_kits_rarity");
	if (paintKitsRarity.empty())
		return 4;

	auto skinDataVec = doc.BreadthFirstSearchMultiple("paint_kits");
	if (!skinDataVec.size())
		return 5;
	auto PaintKitNames = english.BreadthFirstSearch("Tokens");
	if (!PaintKitNames || !PaintKitNames->ToObject())
		return 6;

	//std::unordered_map<std::string, std::set<std::string>> G::weaponSkins;
	//std::unordered_map<std::string, skinInfo> G::skinMap;
	//std::unordered_map<std::string, std::string> G::skinNames;

	std::array weaponNames = {
		std::string("deagle"),
		std::string("elite"),
		std::string("fiveseven"),
		std::string("glock"),
		std::string("ak47"),
		std::string("aug"),
		std::string("awp"),
		std::string("famas"),
		std::string("g3sg1"),
		std::string("galilar"),
		std::string("m249"),
		std::string("m4a1_silencer"),
		std::string("m4a1"),
		std::string("mac10"),
		std::string("p90"),
		std::string("ump45"),
		std::string("xm1014"),
		std::string("bizon"),
		std::string("mag7"),
		std::string("negev"),
		std::string("sawedoff"),
		std::string("tec9"),
		std::string("hkp2000"),
		std::string("mp5sd"),
		std::string("mp7"),
		std::string("mp9"),
		std::string("nova"),
		std::string("p250"),
		std::string("scar20"),
		std::string("sg556"),
		std::string("ssg08"),
		std::string("usp_silencer"),
		std::string("cz75a"),
		std::string("revolver"),
		std::string("knife_m9_bayonet"),
		std::string("bayonet"),
		std::string("knife_flip"),
		std::string("knife_gut"),
		std::string("knife_css"),
		std::string("knife_cord"),
		std::string("knife_canis"),
		std::string("knife_karambit"),
		std::string("knife_tactical"),
		std::string("knife_outdoor"),
		std::string("knife_skeleton"),
		std::string("knife_falchion"),
		std::string("knife_survival_bowie"),
		std::string("knife_butterfly"),
		std::string("knife_push"),
		std::string("knife_ursus"),
		std::string("knife_gypsy_jackknife"),
		std::string("knife_stiletto"),
		std::string("knife_widowmaker"),
		std::string("studded_bloodhound_gloves"),
		std::string("sporty_gloves"),
		std::string("slick_gloves"),
		std::string("leather_handwraps"),
		std::string("motorcycle_gloves"),
		std::string("specialist_gloves"),
		std::string("studded_hydra_gloves")
	};


	for (auto child : weaponSkinCombo->children)
	{
		if (child->ToObject())
		{
			for (auto weapon : weaponNames)
			{
				auto skinName = child->ToObject()->GetKeyByName("icon_path")->ToKeyValue()->Value.toString();
				auto pos = skinName.find(weapon);

				if (pos != std::string::npos)
				{
					auto pos2 = skinName.find_last_of('_');
					g_SkinData.weaponSkins[weapon].insert(
						skinName.substr(pos + weapon.length() + 1,
							pos2 - pos - weapon.length() - 1)
					);
					break;
				}
			}
		}
	}

	for (auto skinData : skinDataVec)
	{
		if (skinData->ToObject())
		{
			for (auto skin : skinData->children)
			{
				if (skin->ToObject())
				{
					skinInfo si;
					si.paintkit = skin->ToObject()->name.toInt();

					auto skinName = skin->ToObject()->GetKeyByName("name")->ToKeyValue()->Value.toString();
					auto tagNode = skin->ToObject()->GetKeyByName("description_tag");
					if (tagNode)
					{
						std::string tag = tagNode->ToKeyValue()->Value.toString();
						tag = tag.substr(1, std::string::npos); //skip #
						std::transform(tag.begin(), tag.end(), tag.begin(), towlower);
						si.tagName = tag;
					}

					auto keyVal = skin->ToObject()->GetKeyByName("seed");
					if (keyVal != nullptr)
						si.seed = keyVal->ToKeyValue()->Value.toInt();

					g_SkinData.skinMap[skinName] = si;
				}
			}
		}
	}

	for (auto child : PaintKitNames->children)
	{
		if (child->ToKeyValue())
		{
			std::string key = child->ToKeyValue()->Key.toString();
			std::transform(key.begin(), key.end(), key.begin(), towlower);
			if (key.find("paintkit") != std::string::npos &&
				key.find("tag") != std::string::npos)
			{
				g_SkinData.skinNames[key] = child->ToKeyValue()->Value.toString();
			}
		}
	}

	for (const auto& rarity : paintKitsRarity) {
		if (rarity->ToObject()) {
			for (const auto& child : rarity->children) {
				if (child->ToKeyValue()) {
					std::string paint_kit_name = child->ToKeyValue()->Key.toString();
					std::string paint_kit_rarity = child->ToKeyValue()->Value.toString();

					auto skinInfo = &g_SkinData.skinMap[paint_kit_name];

					skinInfo->rarity = GetWeaponRarity(paint_kit_rarity);
				}
			}
		}
	}


	return 0;
}