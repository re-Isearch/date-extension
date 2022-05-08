/*
 *  geohash.h
 *
 * This module current supports geohash and Maidenhead
 *
 */


#ifdef __cplusplus
namespace _private_geo {
#endif

// Metric in meters
typedef struct GeoBoxDimensionStruct
{

  double height;
  double width;

} GeoBoxDimension;

/*
typedef struct PositionStruct
{
  double lattude;
  double longitude;
} Position;
*/


typedef struct GeoCoordStruct
{
  double latitude;
  double longitude;
  /*Postion position; */

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
extern const char *geohash_encode (const double lat, const double lng,
				   int precision);

/* 
 * Returns the latitude and longitude used to create the hash along with
 * the bounding box for the encoded coordinate.
 */
extern GeoCoord geohash_decode (const char *hash);

/* 
 * Return an array of geohashes that represent the neighbors of the passed
 * in value. The neighbors are indexed as followed:
 *
 *                  N, NE, E, SE, S, SW, W, NW
 * 					0, 1,  2,  3, 4,  5, 6, 7
 */
enum GeoDirections { N, NE, E, SE, S, SW, W, NW };

extern char **geohash_neighbors (const char *hash);

/*
 * Returns the width and height of a precision value.
 */
extern GeoBoxDimension geohash_dimensions_for_precision (int precision);


/* Returns the MLS hash for a given latitude and longitude, similar to geocode
 * but using a different hash
 */
extern char *maidenhead_encode (const double lattude, const double longitude);
extern GeoCoord maidenhead_deocde(const char *hash);



#ifdef __cplusplus
};
#endif
