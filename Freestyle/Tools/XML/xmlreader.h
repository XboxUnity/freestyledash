#pragma once

using namespace std;

// structure for storing attribute-name pairs
struct SAttribute
{
	string Name;
	string Value;
};

//! Enumeration of all supported source text file formats 
enum ETEXT_FORMAT
{
	ETF_ASCII,
	ETF_UTF8,
	ETF_UTF16_BE,
	ETF_UTF16_LE,
	ETF_UTF32_BE,
	ETF_UTF32_LE,
};


//! Enumeration for all xml nodes which are parsed by IrrXMLReader
enum EXML_NODE
{
	EXN_NONE,
	EXN_ELEMENT,
	EXN_ELEMENT_END,
	EXN_TEXT,
	EXN_COMMENT,
	EXN_CDATA,
	EXN_UNKNOWN
};

class XMLReader
{
public:
	CHAR filename[100];
	XMLReader();
	~XMLReader(void);
	void LoadXML(const CHAR* srcbuf, ETEXT_FORMAT in_SourceFormat = ETF_UTF16_LE, ETEXT_FORMAT in_TargetFormat = ETF_UTF16_LE);
	bool read();
	EXML_NODE XMLReader::getNodeType();
	int XMLReader::getAttributeCount();
	const CHAR* getAttributeName(int idx);
	const CHAR* getAttributeValue(int idx);
	const CHAR* getAttributeValue(const CHAR* name);
	const CHAR* getAttributeValue(const CHAR* name, const CHAR * defval);
	const CHAR* getAttributeValueSafe(const CHAR* name);
	int getAttributeValueAsInt(const CHAR* name);
	bool getAttributeValueAsBool(const CHAR* name);
	int getAttributeValueAsInt(int idx);
	float getAttributeValueAsFloat(const CHAR* name);
	float getAttributeValueAsFloat(int idx);
	const CHAR* getNodeName();
	const CHAR* getNodeData();
	string getSubNodeData();
	bool isEmptyElement();
	ETEXT_FORMAT getSourceFormat();
	ETEXT_FORMAT getParserFormat();
	void restart();
	int GetOffset();
	int GetSize();
	SAttribute* getAttributeByName(const CHAR* name);
private:
	void parseCurrentNode();
	bool setText(CHAR* start, CHAR* end);
	void ignoreDefinition();
	void parseComment();
	void parseOpeningXMLElement();
	void parseClosingXMLElement();
	bool parseCDATA();
	string replaceSpecialCharacters( string& origstr);
	//bool readFile(IFileReadCallBack* callback);
	inline bool isLittleEndian(ETEXT_FORMAT f);
	bool isWhiteSpace(CHAR c);
	void createSpecialCharacterList();
	bool equalsn(const CHAR* str1, const CHAR* str2, int len);

public:
	CHAR* TextBegin;        // start of text to parse
	
private:
	CHAR* TextData;         // data block of the text file
	CHAR* P;                // current point in text to parse
	unsigned int TextSize;       // size of text to parse in characters, not bytes

	EXML_NODE CurrentNodeType;   // type of the currently parsed node
	ETEXT_FORMAT SourceFormat;   // source format of the xml file
	ETEXT_FORMAT TargetFormat;   // output format of this parser

	string NodeName;    // name of the node currently in
	string EmptyString; // empty string to be returned by getSafe() methods

	bool IsEmptyElement;       // is the currently parsed node empty?

#pragma warning(disable:4251)
	vector<string> SpecialCharacters; // see createSpecialCharacterList()
	vector<SAttribute> Attributes; // attributes of current element
#pragma warning(default:4251)
};

XMLReader* LoadConfigFile(string configfile, string path = "");
bool StartAttribute(XMLReader* xml, string attribute);
bool EndAttribute(XMLReader* xml, string attribute);
void FreeXMLAndText(XMLReader * xml);
void FreeXML(XMLReader * xml);
XMLReader* XMLFromString(wstring & input);

