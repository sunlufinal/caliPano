#ifndef FEATURES_IO_H_
#define FEATURES_IO_H_

#define _USE_MATH_DEFINES
#include <cmath>

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "CLinkedIpoint.h"

using std::string;
using std::cerr;
using std::endl;


/**
 * Helper class to write features and feature tracks to file
 * Assumes cylindical images.
 */
class Features2File {
  int d_width, d_height;
  float d_radius;

public:
  inline Features2File( string outFile, const IpVec* features, const LIpVector* vec, int numImages, int width, int height );

protected:
  inline bool saveFeatures(std::ostream& out, const IpVec* features, int numImages );
  inline void convertXyDir( float x, float y, float& cx, float& cy, float& cz); 
};


void Features2File::convertXyDir( float x, float y, float& cx, float& cy, float& cz) 
{
  // front axis would be the +Z	
  double angle = x / d_radius; 
  cy = (float)-(y - d_height/2.0);
  cx = (float)(d_radius*sin(angle));
  cz = (float)(-d_radius*cos(angle));
  float length = sqrt( cx*cx + cy*cy + cz*cz );
  cx /= length;
  cy /= length;
  cz /= length;
}


Features2File::Features2File( string outFile, const IpVec* features, const LIpVector* vec, int numImages, 
			      int _width, int _height ) : d_width(_width),  d_height(_height) 
{ 
  // set radius, width and height
  d_radius = d_width/(2.0*M_PI);
  // open out file
  std::ofstream out( outFile.c_str());
  if ( !out ) {
    cerr << "Could not open file for output: " << outFile << endl;
    throw outFile;
  }
  out << std::scientific << std::setprecision(17);
  saveFeatures( out, features, numImages );
  out << endl; // Add empty line
  CLinkedIpoint::save2Tracks(out, vec, numImages );
}


bool Features2File::saveFeatures(std::ostream& out, const IpVec* features, int numImages ) {
  for (int i=0; i<numImages; ++i) {
    for (unsigned int j=0; j < features[i].size(); ++j ) {
      const Ipoint& ip = features[i][j];
      // convert from x and y in cylinder coordinates to a direction
      float cx, cy, cz;
      convertXyDir( ip.x, ip.y, cx, cy, cz);
      // We write as image num 0 because we use cylinderical panoramas
      out << i << " " << 0 << " " << cx << " " << cy << " " << cz << std::endl;
    }
  }
  return true;
}


/**
 * Helper class to read features and feature tracks from file
 * Assume cylindical images were stored in file.
 * width and height are used to convert directions into pixel coordinates
 */
class FeaturesFromFile {
  int d_width, d_height;
  float d_radius;
  IpVec* d_features;
  LIpVector* d_vec;
  int d_numImages;
public:
  inline FeaturesFromFile( string inFile, int width, int height );
  inline IpVec* getIpVec() { return d_features; }
  inline LIpVector* getLIpVector() { return d_vec; }
  inline int getNumImages() { return d_numImages; }


protected:
  inline FeaturesFromFile();
  inline int findNumImages( std::istream& in );
  inline bool loadFeatures(std::istream& in, int numImages);
  inline void convertDir2xy( float cx, float cy, float cz, float& x, float& y );
};


inline void FeaturesFromFile::convertDir2xy( float cx, float cy, float cz, 
					     float& x, float& y ) {
  double angle = atan2( cx, -cz );
  x = d_radius * ((angle<0)?(angle+2.0*M_PI):angle);
  y = d_radius * cy/sqrt( cx*cx + cz*cz );
  y = -y+d_height/2.0;
}


// protected constructor for sub-classing
FeaturesFromFile::FeaturesFromFile() {
}


FeaturesFromFile::FeaturesFromFile( string inFile, int _width, int _height )
  : d_width( _width ), d_height( _height )
{ 
  d_radius = d_width/(2.0*M_PI);
  // open out file
  std::ifstream in( inFile.c_str());
  if ( !in ) {
    cerr << "Could not open file for input: " << inFile << endl;
    throw inFile;
  }
  d_numImages = findNumImages(in);
  d_features = new IpVec[d_numImages];
  loadFeatures( in, d_numImages );
  // initialize d_vec
  d_vec = new LIpVector[d_numImages];
  for ( int iNum=0; iNum<d_numImages; ++iNum ) {
    cerr << "Image " << iNum << ": " << d_features[iNum].size() << endl;
    for (int fNum = 0; fNum<(int)d_features[iNum].size(); ++fNum) {
      CLinkedIpoint* plink = new CLinkedIpoint(d_features[iNum], fNum);
      _ASSERT(plink != NULL);
      d_vec[iNum].push_back(plink);
    }
  }
  CLinkedIpoint::loadFromTracks(in, d_vec, d_numImages);
}

// scan the file to determine the number of images
int FeaturesFromFile::findNumImages( std::istream& in ) {
  long begin = in.tellg();
  // Read the first number per line
  string line;
  int maxImg=-1, iNum; 
  while ( in ) {
    std::getline(in,line);
    std::istringstream streamLine(line);
    streamLine >> iNum;
    if ( !streamLine.fail() ) {
      if ( iNum > maxImg ) maxImg = iNum;
    } else {
      // at the end of features -- stop
      in.seekg( begin );
      return maxImg+1;
    }
  }
  // We should not be here
  cerr << "Unexpected file format: maxImage" << endl;
  in.clear();
  in.seekg( begin );
  return maxImg+1;
}


bool FeaturesFromFile::loadFeatures(std::istream& in, int numImages) {
  string line;
  while ( in ) {
    std::getline(in,line);
    std::istringstream streamLine(line);
    int iNum, cNum;
    float cx,cy,cz;
    Ipoint ip;
    streamLine >> iNum >> cNum >> cx >> cy >> cz;
    if ( !streamLine.fail() ) {
      if ( iNum < 0 || iNum >= numImages ) return false;
      convertDir2xy( cx, cy, cz, ip.x, ip.y );
      d_features[iNum].push_back( ip );
    } else {
      return true;
    }
  }
  return true;
}
#endif
