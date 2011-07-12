#pragma once

#define FONT_EXTRAROWS 1
#define FONT_IMAGEROWS 1


#define CHAR_STAR "\x090"
#define CHAR_GREYSTAR "\x091"
#define CHAR_PLAY "\x092"
#define CHAR_RANDOM "\x093"
#define CHAR_CLOSE "\x094"
#define CHAR_CD "\x095"

#define CHAR_ELIPSIS "\x07F"
#define CHAR_DEGREES "\x080"
#define CHAR_LBRKT "\x081"
#define CHAR_RBRKT "\x082"

inline void GetXMLSpecialChars(vector<string>& SpecialCharacters)
{
	/*SpecialCharacters.push_back(string(CHAR_ELIPSIS+"elip;"));
	SpecialCharacters.push_back(string(CHAR_DEGREES+"deg;"));

	SpecialCharacters.push_back(string(CHAR_STAR+"star;"));
	SpecialCharacters.push_back(string(CHAR_GREYSTAR+"gstar;"));
	SpecialCharacters.push_back(string(CHAR_PLAY+"play;"));
	SpecialCharacters.push_back(string(CHAR_RANDOM+"random;"));
	SpecialCharacters.push_back(string(CHAR_CD+"cd;"));
	SpecialCharacters.push_back(string(CHAR_LBRKT+"lb;"));
	SpecialCharacters.push_back(string(CHAR_RBRKT+"rb;"));*/
}

inline void GetExtraGlyphs(vector<string>& extraglyphs)
{
	extraglyphs.push_back("star.png");
	extraglyphs.push_back("greystar.png");
	extraglyphs.push_back("rightblue.png");
	extraglyphs.push_back("random.png");
	extraglyphs.push_back("close.png");
	extraglyphs.push_back("cd.png");
}

inline char GetExtraChar(int i)
{
	char res = ' ';
	switch (i)
	{
	case 0x80:
		res = '°';
		break;
	case 0x81:
		res = '[';
		break;
	case 0x82:
		res = ']';
		break;
	}
	return res;
}
