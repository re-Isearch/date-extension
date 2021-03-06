

typedef struct GeoGPSStruct {
   double latitude;
   double longitude;
   int    precision;
}  GeoGPS;



// Metric in meters
typedef struct GeoBoxDimensionStruct {
	
	double height;
	double width;

} GeoBoxDimension;

typedef struct GeoCoordStruct {
    
    double latitude;
    double longitude;
    
    double north;
    double east;
    double south;
    double west;

	GeoBoxDimension dimension;
    
} GeoCoord;

/*
 * Creates a the hash at the specified precision. If precision is set to 0.
 * or less than it defaults to 12.
 */
//extern const char* geohash_encode(const double lat, const double lng) {
//  return geohash_encode(lat, lng, 0);
//}	
extern const char* geohash_encode(const double lat, const double lng, int precision);

/* 
 * Returns the latitude and longitude used to create the hash along with
 * the bounding box for the encoded coordinate.
 */
extern GeoCoord geohash_decode(const char* hash);

/* 
 * Return an array of geohashes that represent the neighbors of the passed
 * in value. The neighbors are indexed as followed:
 *
 *                  N, NE, E, SE, S, SW, W, NW
 * 					0, 1,  2,  3, 4,  5, 6, 7
 */ 
extern char** geohash_neighbors(const char* hash);

/*
 * Returns the width and height of a precision value.
 */
extern GeoBoxDimension geohash_dimensions_for_precision(int precision);
