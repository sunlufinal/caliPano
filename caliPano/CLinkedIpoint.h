#ifndef CLINKED_IPOINT_H_
#define CLINKED_IPOINT_H_

#include <vector>

// include opencv\cv.h before OpenSURF\ipoint.h, since ipoint.h needs some structures in OpenCV
#include "opencv\cv.h"
#include "OpenSURF\ipoint.h"

struct CLinkedIpoint;
typedef std::vector<CLinkedIpoint*> LIpVector;
//abrunton 18/05/2011
//just for dirt, let's save the indices of the features as well so we can track features through more than two images
typedef std::vector<std::pair<int, int> > IndexPairVec;

struct CLinkedIpoint 
{
	IpVec& m_features;//reference to vector of Ipoint features
	int m_iFeature;//index of this feature in its image
	CLinkedIpoint* m_pPrev;//pointer to matched feature in previous frame
	CLinkedIpoint* m_pNext;//pointer to matched feature in next frame
	int m_i3d;//index of reconstructed 3d point
	int m_i3dPairwise;//index of reconstructed 3d point from pairwise matching

	CLinkedIpoint(IpVec& features, int iFeature, CLinkedIpoint* pPrev = NULL, CLinkedIpoint* pNext = NULL): 
	m_features(features),  m_iFeature(iFeature), 
		m_pPrev(pPrev), m_pNext(pNext), 
		m_i3d(-1), m_i3dPairwise(-1) { 
	}

	//compute matches based on similar function in ipoint.cpp
	static int getMatches(LIpVector& linkPts1, LIpVector& linkPts2, IpPairVec &matches, IndexPairVec& indices)
	{
		const float DIST_MATCH_RATIO = 0.65f;

		int i, j;
		float dist, d1, d2;
		CLinkedIpoint* pip1;
		CLinkedIpoint* pip2;
		CLinkedIpoint* pipmatch;
		int nMatches = 0;

		matches.clear();
		indices.clear();//abrunton
		// int jmatch;//abrunton // Not used

		for (i = 0; i < (int)linkPts1.size(); i++)
		{
			d1 = d2 = FLT_MAX;
			pip1 = linkPts1[i];
			pipmatch = NULL;

			for (j = 0; j < (int)linkPts2.size(); j++)
			{
				pip2 = linkPts2[j];

				dist = pip1->m_features[pip1->m_iFeature] - pip2->m_features[pip2->m_iFeature];

				if (dist < d1)
				{
					d2 = d1;
					d1 = dist;
					pipmatch = pip2;
					// jmatch = j; // Not used
				}
				else if (dist < d2)
				{
					d2 = dist;
				}
			}

			if (pipmatch != NULL && d1 / d2 < DIST_MATCH_RATIO)
			{
				if (pipmatch->m_pPrev != NULL)
				{
					//check reverse match
					//pipmatch already matched to another feature in linkPts1
					//make sure threshold is satisfied
					dist = (pipmatch->m_features[pipmatch->m_iFeature] - pipmatch->m_pPrev->m_features[pipmatch->m_pPrev->m_iFeature]);
					if (d1 < DIST_MATCH_RATIO * dist)
					{
						pip1->m_pNext = pipmatch;
						pipmatch->m_pPrev->m_pNext = NULL;
						pipmatch->m_pPrev = pip1;
						// matches.push_back(std::make_pair(pip1->m_features[pip1->m_iFeature], pipmatch->m_features[pipmatch->m_iFeature]));
						// indices.push_back(std::make_pair(pip1->m_iFeature, pipmatch->m_iFeature));
					}
					else if (dist > DIST_MATCH_RATIO * d1)
					{
						//undo previous found match
						pipmatch->m_pPrev->m_pNext = NULL;
						pipmatch->m_pPrev = NULL;
						nMatches--;//one less match
					}
				}
				else
				{
					pip1->m_pNext = pipmatch;
					pipmatch->m_pPrev = pip1;
					// matches.push_back(std::make_pair(pip1->m_features[pip1->m_iFeature], pipmatch->m_features[pipmatch->m_iFeature]));
					// indices.push_back(std::make_pair(pip1->m_iFeature, pipmatch->m_iFeature));
					nMatches++;//one more match
				}
			}
		}

		setMatches(linkPts1, linkPts2, matches, indices);

		return nMatches;
	}

	// Not necessary -- std::vector has clear() 
	static void clearLIpVector(LIpVector& vec)
	{
		for (int i = 0; i < (int)vec.size(); i++)
		{
			_ASSERT(vec[i] != NULL);
			delete vec[i];
			vec[i] = NULL;
		}
	}


	static bool save2Tracks(std::ostream& out, const LIpVector* vec, int numImages ) {
		for (int i=0; i<numImages; ++i) {
			int backCount = 0, forwardCount = 0;
			std::cerr << "Image " << i << " " << vec[i].size() << " features" <<std::endl;
			for (unsigned int j=0; j < vec[i].size(); ++j ) {
				_ASSERT(vec[i][j] != NULL);
				// check if we have a new track
				if ((vec[i][j]->m_pPrev == NULL && vec[i][j]->m_pNext != NULL)) {
					CLinkedIpoint* curr = vec[i][j];
					++forwardCount;
					int currImg = i;
					// write current tracks
					do {
						out << currImg << ":0:" << curr->m_iFeature << " ";
						curr = curr->m_pNext;
						++currImg;
					} while (curr != NULL);
					out << std::endl;
				} else
					if (vec[i][j]->m_pPrev != NULL) {
						++backCount;
					}
			}
			std::cerr << forwardCount << " features chained." << std::endl;
			std::cerr << backCount << " previous were not NULL!" << std::endl;
		}
		return true;
	}

	// load back tracks which were saved with save2Tracks
	// Note that the vector at m_feature will be of size 0
	static bool loadFromTracks(std::istream& in, const LIpVector* vec, int numImages) {
		std::string line;
		while (std::getline(in, line)) {
#ifdef DEBUG_CLINKED_IPOINT
			cerr << "Got: " << line << endl;
#endif
			std::istringstream streamLine(line);
			// Get individual white space separated features
			int i=-1, v=-1, j=-1;
			char cA, cB;
			// i is the frame, j is the feature id
			std::vector<int> iVec;
			// std::vector<int> vVec; // assume all views are 0 for cylindrical panorama
			std::vector<int> jVec;
			while ( streamLine >> i >> cA >> v >> cB >> j) {
				// Process features
#ifdef DEBUG_CLINKED_IPOINT
				cerr << i << cA << v << cB << j << endl;
#endif
				_ASSERT( i < numImages );
				_ASSERT( v == 0 );
				iVec.push_back(i);
				// vVec.push_back(v);
				jVec.push_back(j);
			}
			// now link the points forward
			// *jIter is the index of the feature point 
			std::vector<int>::iterator jIter=jVec.begin();
			for ( std::vector<int>::iterator iIter=iVec.begin(); iIter != --(iVec.end()); ) {
				_ASSERT(jIter != jVec.end());
				std::vector<int>::iterator iItNext = iIter + 1, jItNext = jIter + 1;
				//          (vec[*iIter])[*jIter]->m_pNext = (vec[*(++iIter)])[*(++jIter)];
				vec[*iIter][*jIter]->m_pNext = vec[*iItNext][*jItNext];
				iIter = iItNext;
				jIter = jItNext;
			}
			vec[*iVec.rbegin()][*jVec.rbegin()]->m_pNext = NULL;
			// and backwards
			std::vector<int>::reverse_iterator rjIter=jVec.rbegin();
			for ( std::vector<int>::reverse_iterator riIter=iVec.rbegin(); riIter != --(iVec.rend()); ) {
				_ASSERT(jIter != jVec.end());
				std::vector<int>::reverse_iterator iItNext = riIter + 1, jItNext = rjIter + 1;
				//          vec[*riIter][*rjIter]->m_pPrev = vec[*(++riIter)][*(++rjIter)];
				vec[*riIter][*rjIter]->m_pPrev = vec[*iItNext][*jItNext];
				riIter = iItNext;
				rjIter = jItNext;
			}
			vec[*iVec.begin()][*jVec.begin()]->m_pPrev = NULL;
		}
		return true;
	}


	static void setMatches(const LIpVector& vec, 
		const LIpVector& nextVec, 
		IpPairVec& pMatches,
		IndexPairVec& pMatchIndices) {
			for ( std::vector<CLinkedIpoint*>::const_iterator pipIter=vec.begin(); 
				pipIter != vec.end(); ++pipIter ) {
					// check if match and if match is with next image
					if ( (*pipIter)->m_pNext != NULL && &((*pipIter)->m_pNext->m_features) ==  &(nextVec[0]->m_features) ) 
					{ 
						CLinkedIpoint* pipmatch = (*pipIter)->m_pNext;  
						pMatches.push_back(std::make_pair((*pipIter)->m_features[(*pipIter)->m_iFeature], 
							pipmatch->m_features[pipmatch->m_iFeature]));
						pMatchIndices.push_back(std::make_pair((*pipIter)->m_iFeature, pipmatch->m_iFeature));
					}
			}
			return;
	}



	// set matches and indicies from linkPts
	static void setMatches(const LIpVector* vec, const int numImages, 
		IpPairVec* pMatches,
		IndexPairVec* pMatchIndices) {
			for ( int nImg=0; nImg < numImages-1; ++nImg ) {
				setMatches(vec[nImg], vec[nImg+1], pMatches[nImg], pMatchIndices[nImg]);
			}
			return;
	}
};
#endif
