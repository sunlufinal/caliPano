#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include <opencv\cv.h>

#include "XArray.h"

using namespace std;
using namespace cv;

extern CReport g_report;

struct PanoConfig
{
	vector<IplImage *> d_pimgs;
	std::string d_strWD;
	vector<string> d_imageFilenames;
	CvSize* d_pImageDims;
	string d_plyFileName;
	bool* d_pbSkip;
	int d_nImagesValid;

	inline PanoConfig(const char* szFilename);
	inline int getNImages();

protected:

	int d_nImages;

	// These are default height/width
	int d_panWidth;
	int d_panHeight;

	//tokenize a string according to a set of delimiting characters
	void tokenize(char* szInput, const char* cszDelim, CXArray<char*>& tokens);
};

PanoConfig::PanoConfig(const char* szFilename) : d_pImageDims(NULL),
	d_plyFileName("result.ply"),
	d_panWidth(4096),
	d_panHeight(1304){
	const char cszCommentChar[] = "#";
	const char cszWhiteSpaces[] = " \t\n\0";

	int i;
	ifstream config;
	char szLineBuffer[8192];
	int linelen;
	int line, commentpos;
	CXArray<char*> tokens;

	config.open(szFilename);

	line = 0;
	while(config.good()){
		config.getline(szLineBuffer, 8192);
		linelen = strlen(szLineBuffer);
		commentpos = strcspn(szLineBuffer, cszCommentChar);
		if(commentpos < linelen)
			szLineBuffer[commentpos] = 0;

		tokenize(szLineBuffer, cszWhiteSpaces, tokens);

		line++;

		if (tokens.length() == 0)
		{
			continue;
		}

		i = 0;
		if (strcmp(tokens[i], "IMAGE") == 0)
		{
			i++;
			if (i == tokens.length())
			{
				g_report.loud("Missing filename for IMAGE on line ");
				g_report.loud(line);
				g_report.newlineLoud();
				continue;
			}

			std::string strFile = tokens[i];
			d_imageFilenames.push_back(strFile);
		}else if (strcmp(tokens[i], "WD") == 0)
		{
			i++;
			if (i == tokens.length())
			{
				g_report.loud("Missing directory path for WD on line ");
				g_report.loud(line);
				g_report.newlineLoud();
				continue;
			}

			d_strWD = tokens[i];
			if (d_strWD.at(d_strWD.length() - 1) != '/' && d_strWD.at(d_strWD.length() - 1) != '\\')
				d_strWD += '/';
		}
	}

	config.close();

	d_nImages = (int)d_imageFilenames.size();
	d_nImagesValid = d_nImages;

	// initialize image pointer array
	d_pimgs.resize(d_nImages);

	if ( d_nImages > 0 ) {
		d_pbSkip = new bool[d_nImages];
		d_pImageDims = new CvSize[d_nImages];
		for ( int i=0; i<d_nImages; ++i ) {
			d_pbSkip[i] = false;
			d_pImageDims[i].width = d_panWidth;
			d_pImageDims[i].height = d_panHeight;
		}
	}
}

//tokenize a string according to a set of delimiting characters
void PanoConfig::tokenize(char* szInput, const char* cszDelim, CXArray<char*>& tokens)
{
	_ASSERT(szInput != NULL);
	_ASSERT(cszDelim != NULL);

	int inputlen;
	int delimpos = 0;
	char* szToken = szInput;

	tokens.fastInit();

	do
	{
		inputlen = (int)strlen(szToken);
		delimpos = (int)strcspn(szToken, cszDelim);
		if (delimpos > 0)
			tokens.pushEnd(szToken);
		szToken[delimpos] = 0;
		szToken += delimpos + 1;
	} while (delimpos < inputlen);
}

int PanoConfig::getNImages() {
	return d_nImages;
}