#define WIN32

#include <string>

#include "opencv\cv.h"
#include "opencv2\opencv.hpp"
#include "OpenSURF\surflib.h"

#include "Report.h"
#include "CLinkedIpoint.h"
#include "FeaturesSurf.h"
#include "panoConfig.h"
#include "utility.h"

using namespace std;
using namespace cv;

CReport g_report("caliPano_report.txt");

int main(int argc, char** argv)
{
	if (argc < 2){
      g_report.loud("Please pass configuration file as argument\n");
      return 1;
    }

	// load configuration file
	g_report.loud("Loading configuration file ");
	g_report.loud(argv[1]);
	g_report.loud("...\n");
	PanoConfig calConf(argv[1]);
	g_report.loud(calConf.getNImages());
	g_report.loud(" images\n");
	if(calConf.getNImages() < 2)
		g_report.loud("not enough images");

	// loading images
	g_report.loud("\nLoading images...\n");
	for(int i = 0; i < calConf.getNImages(); i++){
		string strFile = calConf.d_strWD + calConf.d_imageFilenames[i];	// get image path from config class

		g_report.loud("Loading image ");
		g_report.loud(strFile.c_str());
		g_report.loud("......");

		calConf.d_pimgs[i] = cvLoadImage(strFile.c_str());
		if (calConf.d_pimgs[i] == NULL)	{
			g_report.loud(" Failed!");
			g_report.loud(" Image file ");
			g_report.loud(strFile.c_str());
			g_report.loud(" not found!\n");
			calConf.d_pbSkip[i] = true;
			continue;
		}else{
			g_report.loud(" Successfully!\n");
		}

		calConf.d_pImageDims[0] = cvGetSize(calConf.d_pimgs[i]);
	}

	// detecting features
	g_report.loud("\nDetecting features in images...\n");

	LIpVector* pLinkedFeatures = NULL;
	FeaturesSurf fs(calConf.getNImages());
	pLinkedFeatures = new LIpVector[calConf.getNImages()];
	fs.resize(calConf.getNImages());

	for(int i = 0; i < calConf.getNImages(); i++){
		surfDetDes(calConf.d_pimgs[i], fs.d_pImageFeatures[i], false, 3, 4, 2, 0.0005f);
		g_report.loud(fs.d_pImageFeatures[i].size());
		g_report.loud(" features detected.\n");
	}

	// display features detected
	/*for(int i = 0; i < calConf.getNImages(); i++){
		Mat img(calConf.d_pimgs[i]);
		for(unsigned int j = 0; j < fs.d_pImageFeatures[i].size(); j++){
			Point center((int)(fs.d_pImageFeatures[i][j].x), (int)(fs.d_pImageFeatures[i][j].y));
			circle(img,center,10,Scalar(255,0,0));
		}
		Mat img_resized;
		Size dim(1024,326);
		resize(img,img_resized,dim);
		namedWindow(calConf.d_imageFilenames[i]);
		imshow(calConf.d_imageFilenames[i],img_resized);
	}*/

	// confused part
	for(int i = 0; i < calConf.getNImages(); i++){
		for (int j = 0; j < (int)fs.d_pImageFeatures[i].size(); j++) {
		  CLinkedIpoint* plink = new CLinkedIpoint(fs.d_pImageFeatures[i], j);
		  _ASSERT(plink != NULL);

		  pLinkedFeatures[i].push_back(plink);
		}
	}

	// ***********************
	// feature matching
	// ***********************

	//minimum number of matches between two images to compute rotation and translation
	const int g_nInitMatchThreshold = 12;

	for(int i = 0; i < calConf.getNImages()-1; i++){
		if(calConf.d_pbSkip[i])
			continue;

		// looking for the next valid picture
		int j = i + 1;
		while (j < calConf.getNImages() && calConf.d_pbSkip[j])
			j++;
		if(j >= calConf.getNImages())
			break;



		CLinkedIpoint::getMatches(pLinkedFeatures[i], pLinkedFeatures[j], 
				fs.d_pFeatureMatches[i], fs.d_pFeatureMatchIndices[i]);

		while (fs.d_pFeatureMatches[i].size() < (unsigned int)g_nInitMatchThreshold && 
			j < calConf.getNImages()) {
				g_report.loud("Insufficient matches between image ");
				g_report.loud(i);
				g_report.loud(" and image ");
				g_report.loud(j);
				g_report.newlineLoud();

				fs.d_pFeatureMatches[i].clear();

				calConf.d_pbSkip[j] = true;
				while (j < calConf.getNImages() && calConf.d_pbSkip[j])
					j++;

				if (j < calConf.getNImages())
					//				getMatches(fs.d_pImageFeatures[i], fs.d_pImageFeatures[j], fs.d_pFeatureMatches[i], fs.d_pFeatureMatchIndices[i]);
					// nMatches = 
					CLinkedIpoint::getMatches(pLinkedFeatures[i], pLinkedFeatures[j], fs.d_pFeatureMatches[i], fs.d_pFeatureMatchIndices[i]);
		}

		if (j == calConf.getNImages()) {
			calConf.d_pbSkip[i] = true;
			break;
		}

		g_report.loud("Found ");
		g_report.loud((unsigned long)fs.d_pFeatureMatches[i].size());
		//		g_report.loud(nMatches);
		g_report.loud(" feature matches between image ");
		g_report.loud(i);
		g_report.loud(" and image ");
		g_report.loud(j);
		g_report.newlineLoud();
	}

	// display matches

	
	//go through and "remove" images marked to be skipped
	int validImg = 0;
	for (int i = 0; i < calConf.getNImages(); i++) {
		if (calConf.d_pbSkip[i]) continue;

		if (validImg < i) {
			calConf.d_pImageDims[validImg] = calConf.d_pImageDims[i];
			fs.d_pImageFeatures[validImg] = fs.d_pImageFeatures[i];
			fs.d_pFeatureMatches[validImg] = fs.d_pFeatureMatches[i];
			fs.d_pFeatureMatchIndices[validImg] = fs.d_pFeatureMatchIndices[i];
			calConf.d_imageFilenames[validImg] = calConf.d_imageFilenames[i];
			pLinkedFeatures[validImg] = pLinkedFeatures[i];
		}
		validImg++;
	}

	calConf.d_nImagesValid = validImg;

	for(int i = 0; i < calConf.getNImages()-1; i++){
		Mat img1(calConf.d_pimgs[i]);
		Mat img2(calConf.d_pimgs[i+1]);

		Size size1 = img1.size();

		IpVec features1 = fs.d_pImageFeatures[i];
		IpVec features2 = fs.d_pImageFeatures[i+1];

		IpPairVec matches = fs.d_pFeatureMatches[i];
		//IpPairVec matches2 = fs.d_pFeatureMatches[i+1];

		for(unsigned int j = 0; j < matches.size(); j++){
			Point center((int)(matches[j].first.x), (int)(matches[j].first.y));
			circle(img1,center,10,Scalar(255,0,0));
		}
		for(unsigned int j = 0; j < matches.size(); j++){
			Point center((int)(matches[j].second.x), (int)(matches[j].second.y));
			circle(img2,center,10,Scalar(255,0,0));
		}

		Mat com = combine2ImagesVertically(img1, img2);

		for(unsigned int j = 0; j < matches.size(); j++){
			Point start((int)(matches[j].first.x),(int)(matches[j].second.y));
			Point end((int)(matches[j].second.x),(int)(size1.height+matches[j].second.y));
			line(com,start,end,Scalar(0,255,255));
		}

		namedWindow(to_string(static_cast<long long>(i)),CV_WINDOW_NORMAL);
		imshow(to_string(static_cast<long long>(i)),com);
	}

	// ------------------------------
	// point cloud construction
	// ------------------------------

	for(int i = 0; i < calConf.d_nImagesValid-1; i++){
		double pruneThresh, baselineEstimate;

		g_report.loud("Frames ");
		g_report.loud(i);
		g_report.loud(" and ");
		g_report.loud(i + 1);
		g_report.loud(":\n");
	}

	waitKey(0);

	return 0;
}