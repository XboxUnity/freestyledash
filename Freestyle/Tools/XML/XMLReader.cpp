#include "stdafx.h"

#include "XMLReader.h"
#include "../Fonts/fontchars.h"
#include "../Generic/tools.h"
#include "../Debug/Debug.h"

XMLReader* LoadConfigFile(string configfile, string path)
{

	string fullname = configfile;

	if (path.size() > 0)
		fullname = path + "\\" + configfile;
	//DebugMsg("XmlReader","Load xml %s",fullname.c_str()); 

	/*if (!FileExists(fullname))
	{
		fullname = GetAppPath() + "\\config\\"+configfile;
	}

	if (!FileExists(fullname))
	{
		fullname = "sysconf\\"+configfile;
	}*/

	if (!FileExists(fullname))
	{
		//DebugMsg("XmlReader","XML does not exist"); 
		return NULL;
	}

	byte * buffer;

	FILE* f;
	fopen_s(&f, fullname.c_str(),"rb");
	fseek (f , 0 , SEEK_END);
	int size = ftell(f);
	fseek(f, 0, 0);
	buffer = (byte*)malloc(size+2);
	//buffer = new WCHAR[size+2];
	int offset = 0;
	int count = 0;
	int toread = 0;
	while( size > 0 )
	{
		toread = min(size,4096);
		// Attempt to read in 100 bytes:
		count = (int)fread( buffer+offset, 1, toread, f );
		if( ferror( f ) )      {
			SAFE_DELETE(buffer);
			return NULL;
		}
		offset += count;
		size -= count;
	}
	fclose(f);
	buffer[offset] = 0;
	buffer[offset+1] = 0;

	XMLReader* xml = new XMLReader();
	
	strcpy_s(xml->filename,100,configfile.c_str());
	if (buffer[0] == 0xFF && buffer[1] == 0xFE)
	{
		CHAR * wbuf = (CHAR*)buffer+1;
		xml->LoadXML(wbuf);
	} else {
		// non unicoe, read type from xml head
		char * buffer2 = (char*)buffer;

		// get encoding type
		char temp[250];
		memcpy(&temp,buffer2,250);
		temp[249] = 0;
		string temp2 = make_lowercase(temp);

		// utf-8 type, convert to unicode
		if (temp2.find("utf-8"))
		{
			
			
			xml->LoadXML(buffer2);
		} else {

		}
	}

	return xml;
}

XMLReader* XMLFromString(string & input)
{
	XMLReader * xml = new XMLReader();
	CHAR * chardata = new CHAR[input.size()+1];
	strcpy_s(chardata,input.size()+1,input.c_str());
	xml->LoadXML(chardata);

	return xml;
}

XMLReader::XMLReader()
{
	createSpecialCharacterList();
}

XMLReader::~XMLReader(void)
{
}

//! Constructor
void XMLReader::LoadXML(const CHAR* srcbuf, ETEXT_FORMAT in_SourceFormat, ETEXT_FORMAT in_TargetFormat)
/*	: TextData(0), P(0), TextSize(0), TextBegin(0), CurrentNodeType(EXN_NONE),
	SourceFormat(ETF_ASCII), TargetFormat(ETF_ASCII)*/
{
	TextData = 0;
	P = 0;
	TextSize = 0;
	TextBegin = 0;
	CurrentNodeType = EXN_NONE;
	SourceFormat = in_SourceFormat;
	TargetFormat = in_TargetFormat;

	int size = (int)strlen(srcbuf);
	TextBegin = (CHAR*)srcbuf;
	TextData = (CHAR*)srcbuf;
	TextSize = size;


	// set pointer to text begin
	P = TextBegin;
}


bool XMLReader::read()
{
	// if not end reached, parse the node
	if (P && (unsigned int)(P - TextBegin) < TextSize - 1 && *P != 0)
	{
		parseCurrentNode();
		return true;
	}

	return false;
}

void XMLReader::restart()
{
	P = TextBegin;
}

int XMLReader::GetOffset()
{
	return P - TextBegin;
}

int XMLReader::GetSize()
{
	return TextSize;
}

//! Returns the type of the current XML node.
EXML_NODE XMLReader::getNodeType()
{
	return CurrentNodeType;
}

//! Returns attribute count of the current XML node.
int XMLReader::getAttributeCount()
{
	return (int)Attributes.size();
}


//! Returns name of an attribute.
const CHAR* XMLReader::getAttributeName(int idx)
{
	if (idx < 0 || idx >= (int)Attributes.size())
		return 0;

	return Attributes[idx].Name.c_str();
}


//! Returns the value of an attribute. 
const CHAR* XMLReader::getAttributeValue(int idx)
{
	if (idx < 0 || idx >= (int)Attributes.size())
		return 0;

	return Attributes[idx].Value.c_str();
}


//! Returns the value of an attribute. 
const CHAR* XMLReader::getAttributeValue(const CHAR* name)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return EmptyString.c_str();

	return attr->Value.c_str();
}

//! Returns the value of an attribute. 
const CHAR* XMLReader::getAttributeValue(const CHAR* name, const CHAR * defval)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return defval;

	return attr->Value.c_str();
}

//! Returns the value of an attribute
const CHAR* XMLReader::getAttributeValueSafe(const CHAR* name)
{
	SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return EmptyString.c_str();

	return attr->Value.c_str();
}



//! Returns the value of an attribute as integer. 
int XMLReader::getAttributeValueAsInt(const CHAR* name)
{
	const SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return 0;

	return atoi(attr->Value.c_str());
}

bool XMLReader::getAttributeValueAsBool(const CHAR* name)
{
	return (int)getAttributeValueAsInt(name)!=0;
}


//! Returns the value of an attribute as integer. 
int XMLReader::getAttributeValueAsInt(int idx)
{
	const CHAR* attrvalue = getAttributeValue(idx);
	if (!attrvalue)
		return 0;

	return atoi(attrvalue);
}


//! Returns the value of an attribute as float. 
float XMLReader::getAttributeValueAsFloat(const CHAR* name)
{
	const SAttribute* attr = getAttributeByName(name);
	if (!attr)
		return 0;

	string c = attr->Value.c_str();
	return fast_atof(c.c_str());
}


//! Returns the value of an attribute as float. 
float XMLReader::getAttributeValueAsFloat(int idx)
{
	const CHAR* attrvalue = getAttributeValue(idx);
	if (!attrvalue)
		return 0;

	string c = attrvalue;
	return fast_atof(c.c_str());
}


//! Returns the name of the current node.
const CHAR* XMLReader::getNodeName()
{
	return NodeName.c_str();
}


//! Returns data of the current node.
const CHAR* XMLReader::getNodeData()
{
	return NodeName.c_str();
}

//! Returns data of the current node.
string XMLReader::getSubNodeData()
{
	read();
	if (CurrentNodeType == EXN_TEXT)
		return NodeName;

	return "";
}


//! Returns if an element is an empty element, like <foo />
bool XMLReader::isEmptyElement()
{
	return IsEmptyElement;
}

//! Returns format of the source xml file.
ETEXT_FORMAT XMLReader::getSourceFormat()
{
	return SourceFormat;
}

//! Returns format of the strings returned by the parser.
ETEXT_FORMAT XMLReader::getParserFormat()
{
	return TargetFormat;
}


// Reads the current xml node
void XMLReader::parseCurrentNode()
{
	CHAR* start = P;

	// more forward until '<' found
	while(*P != '<' && *P)
		++P;

	if (!*P)
		return;

	if (P - start > 0)
	{
		// we found some text, store it
		if (setText(start, P))
			return;
	}

	++P;

	// based on current token, parse and report next element
	switch(*P)
	{
	case '/':
		parseClosingXMLElement(); 
		break;
	case '?':
		ignoreDefinition();	
		break;
	case '!':
		if (!parseCDATA())
			parseComment();	
		break;
	default:
		parseOpeningXMLElement();
		break;
	}
}


//! sets the state that text was found. Returns true if set should be set
bool XMLReader::setText(CHAR* start, CHAR* end)
{
	// check if text is more than 2 characters, and if not, check if there is 
	// only white space, so that this text won't be reported
	if (end - start < 3)
	{
		CHAR* p = start;
		for(; p != end; ++p)
			if (!isWhiteSpace(*p))
				break;

		if (p == end)
			return false;
	}

	// set current text to the parsed text, and replace xml special characters
	string s(start, (int)(end - start));
	NodeName = replaceSpecialCharacters(s);

	// current XML node type is text
	CurrentNodeType = EXN_TEXT;

	return true;
}



//! ignores an xml definition like <?xml something />
void XMLReader::ignoreDefinition()
{
	CurrentNodeType = EXN_UNKNOWN;

	// move until end marked with '>' reached
	while(*P != L'>')
		++P;

	++P;
}


//! parses a comment
void XMLReader::parseComment()
{
	CurrentNodeType = EXN_COMMENT;
	P += 1;

	CHAR *pCommentBegin = P;

	int count = 1;

	// move until end of comment reached
	while(count)
	{
		if (*P == L'>')
			--count;
		else
		if (*P == L'<')
			++count;

		++P;
	}

	P -= 3;
	NodeName = string(pCommentBegin+2, (int)(P - pCommentBegin-2));
	P += 3;
}


//! parses an opening xml element and reads attributes
void XMLReader::parseOpeningXMLElement()
{
	CurrentNodeType = EXN_ELEMENT;
	IsEmptyElement = false;
	Attributes.clear();

	// find name
	const CHAR* startName = P;

	// find end of element
	while(*P != '>' && !isWhiteSpace(*P))
		++P;

	const CHAR* endName = P;

	// find Attributes
	while(*P != '>')
	{
		if (isWhiteSpace(*P))
			++P;
		else
		{
			if (*P != '/')
			{
				// we've got an attribute

				// read the attribute names
				const CHAR* attributeNameBegin = P;

				while(!isWhiteSpace(*P) && *P != '=')
					++P;

				const CHAR* attributeNameEnd = P;
				++P;

				// read the attribute value
				// check for quotes and single quotes, thx to murphy
				while( (*P != '\"') && (*P != '\'') && *P) 
					++P;

				if (!*P) // malformatted xml file
					return;

				const CHAR attributeQuoteChar = *P;

				++P;
				const CHAR* attributeValueBegin = P;
				
				while(*P != attributeQuoteChar && *P)
					++P;

				if (!*P) // malformatted xml file
					return;

				const CHAR* attributeValueEnd = P;
				++P;

				SAttribute attr;
				attr.Name = string(attributeNameBegin, 
					(int)(attributeNameEnd - attributeNameBegin));

				string s(attributeValueBegin, 
					(int)(attributeValueEnd - attributeValueBegin));

				attr.Value = replaceSpecialCharacters(s);
				Attributes.push_back(attr);
			}
			else
			{
				// tag is closed directly
				++P;
				IsEmptyElement = true;
				break;
			}
		}
	}

	// check if this tag is closing directly
	if (endName > startName && *(endName-1) == '/')
	{
		// directly closing tag
		IsEmptyElement = true;
		endName--;
	}
	
	NodeName = string(startName, (int)(endName - startName));

	++P;
}


//! parses an closing xml tag
void XMLReader::parseClosingXMLElement()
{
	CurrentNodeType = EXN_ELEMENT_END;
	IsEmptyElement = false;
	Attributes.clear();

	++P;
	const CHAR* pBeginClose = P;

	while(*P != '>')
		++P;

	NodeName = string(pBeginClose, (int)(P - pBeginClose));
	++P;
}

//! parses a possible CDATA section, returns false if begin was not a CDATA section
bool XMLReader::parseCDATA()
{
	if (*(P+1) != '[')
		return false;

	CurrentNodeType = EXN_CDATA;

	// skip '<![CDATA['
	int count=0;
	while( *P && count<8 )
	{
		++P;
		++count;
	}

	if (!*P)
		return true;

	CHAR *cDataBegin = P;
	CHAR *cDataEnd = 0;

	// find end of CDATA
	while(*P && !cDataEnd)
	{
		if (*P == '>' && 
		   (*(P-1) == ']') &&
		   (*(P-2) == ']'))
		{
			cDataEnd = P - 2;
		}

		++P;
	}

	if ( cDataEnd )
		NodeName = string(cDataBegin, (int)(cDataEnd - cDataBegin));
	else
		NodeName = "";

	return true;
}

// finds a current attribute by name, returns 0 if not found
SAttribute* XMLReader::getAttributeByName(const CHAR* name)
{
	if (!name)
		return 0;

	string n = name;

	for (int i=0; i<(int)Attributes.size(); ++i)
		if (Attributes[i].Name == n)
			return &Attributes[i];

	return 0;
}

// replaces xml special characters in a string and creates a new one
string XMLReader::replaceSpecialCharacters(
	string& origstr)
{
	//int pos = origstr.findFirst(L'&');
	int pos = (int)origstr.find('&');
	int oldPos = 0;

	if (pos == -1)
		return origstr;

	string newstr;

	while(pos != -1 && pos < (int)origstr.size()-2)
	{
		// check if it is one of the special characters

		int specialChar = -1;
		for (int i=0; i<(int)SpecialCharacters.size(); ++i)
		{
			const CHAR* p = &origstr.c_str()[pos]+1;

			if (equalsn(&SpecialCharacters[i][1], p, (int)SpecialCharacters[i].size()-1))
			{
				specialChar = i;
				break;
			}
		}

		if (specialChar != -1)
		{
			newstr += origstr.substr(oldPos, pos - oldPos);
			newstr += SpecialCharacters[specialChar][0];
			pos += (int)SpecialCharacters[specialChar].size();
		}
		else
		{
			newstr.append(origstr.substr(oldPos, pos - oldPos + 1));
			pos += 1;
		}

		// find next &
		oldPos = pos;
		pos = (int)origstr.find('&', pos);		
	}

	if (oldPos < (int)origstr.size())
		newstr.append(origstr.substr(oldPos, origstr.size()-oldPos));

	return newstr;
}

//! returns if a format is little endian
bool XMLReader::isLittleEndian(ETEXT_FORMAT f)
{
	return f == ETF_ASCII ||
	       f == ETF_UTF8 ||
	       f == ETF_UTF16_LE ||
	       f == ETF_UTF32_LE;
}

//! returns true if a character is whitespace
bool XMLReader::isWhiteSpace(CHAR c)
{
	return (c==' ' || c=='\t' || c=='\n' || c=='\r');
}


//! generates a list with xml special characters
void XMLReader::createSpecialCharacterList()
{
	// list of strings containing special symbols, 
	// the first character is the special character,
	// the following is the symbol string without trailing &.

	SpecialCharacters.push_back("&amp;");
	SpecialCharacters.push_back("<lt;");
	SpecialCharacters.push_back(">gt;");
	SpecialCharacters.push_back("\"quot;");
	SpecialCharacters.push_back("'apos;");

	GetXMLSpecialChars(SpecialCharacters);
}

//! compares the first n characters of the strings
bool XMLReader::equalsn(const CHAR* str1, const CHAR* str2, int len)
{
	int i;
	for(i=0; str1[i] && str2[i] && i < len; ++i)
		if (str1[i] != str2[i])
			return false;

	// if one (or both) of the strings was smaller then they
	// are only equal if they have the same lenght
	return (i == len) || (str1[i] == 0 && str2[i] == 0);
}


bool StartAttribute(XMLReader* xml, string attribute)
{
	if (xml->getNodeType() != EXN_ELEMENT) return false;
	string name = xml->getNodeName();
	if (name == attribute) return true;
	return false;
}

bool EndAttribute(XMLReader* xml, string attribute)
{
	string name = xml->getNodeName();
	if (xml->isEmptyElement() && attribute == name)
	{
		return true;
	}
	if (!xml->read())
		return true;

	if (xml->getNodeType() != EXN_ELEMENT_END) return false;
	name = xml->getNodeName();
	if (name == attribute) return true;
	return false;
}

void FreeXMLAndText(XMLReader * xml)
{
	if (xml)
	{
		SAFE_DELETE_A(xml->TextBegin);
		SAFE_DELETE(xml);
	}
}

void FreeXML(XMLReader * xml)
{
	SAFE_DELETE(xml);
}