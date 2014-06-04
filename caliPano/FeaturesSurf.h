#ifndef FEATURES_SURF_H
#define FEATURES_SURF_H

#include "OpenSURF\ipoint.h"

struct FeaturesSurf {
  // surf features later stored in pLinkedFeature
  IpVec* d_pImageFeatures;
  IpPairVec* d_pFeatureMatches;
  IndexPairVec* d_pFeatureMatchIndices;

  FeaturesSurf() :
    d_pImageFeatures(NULL),
    d_pFeatureMatches(NULL)
  {}

  FeaturesSurf( int nImages ) {
    d_pImageFeatures = new IpVec[nImages];
    d_pFeatureMatches = new IpPairVec[nImages - 1];
	d_pFeatureMatchIndices = new IndexPairVec[nImages - 1];
  }

  ~FeaturesSurf() {
    delete [] d_pImageFeatures;
    delete [] d_pFeatureMatches;
  }

  void resize(int nImages) {
    delete [] d_pImageFeatures;
    delete [] d_pFeatureMatches;
    d_pImageFeatures = new IpVec[nImages];
    d_pFeatureMatches = new IpPairVec[nImages - 1];
	d_pFeatureMatchIndices = new IndexPairVec[nImages - 1];
  }

private:
  FeaturesSurf(const FeaturesSurf& );
  FeaturesSurf& operator=(const FeaturesSurf& );
};

#endif