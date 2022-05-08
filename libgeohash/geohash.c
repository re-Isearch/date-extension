/*
 *  geohash.c
 */

#include "geohash.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

static const float MAX_LAT  =   90.0;
static const float MIN_LAT  =  -90.0;
static const float MAX_LNG  =   180.0;
static const float MIN_LNG  =  -180.0;

enum COMPASS { NORTH=0, EAST, SOUTH, WEST};

// static const int  LENGTH_OF_DEGREE = 111100;	// meters

typedef struct IntervalStruct
{

  double high;
  double low;

} Interval;


/* This is the Normal 32 characer map used for geohashing */
/* NOTE: a, i, l and o are not included */
/* see https://en.wikipedia.org/wiki/Geohash#Algorithm_and_example */
static const char char_map[33] = "0123456789bcdefghjkmnpqrstuvwxyz";
/* index to val & 127  */
int char_map_val[] = {
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
   -1,  -1,  10,  11,  12,  13,  14,  15,  16,  -1,  17,  18,  -1,  19,  20,  -1,
   21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  -1,  -1,  -1,  -1,  -1
};

/*
 * NOTE: If we see upper case letters in a hash that means it is probably 
 * Maidenhead Locator System
*/


/*
 *  The follow character maps were created by Dave Troy and used in his Javascript Geohashing
 *  library. http://github.com/davetroy/geohash-js
 */
static const char *even_neighbors[] = {
  "p0r21436x8zb9dcf5h7kjnmqesgutwvy",
  "bc01fg45238967deuvhjyznpkmstqrwx",
  "14365h7k9dcfesgujnmqp0r2twvyx8zb",
  "238967debc01fg45kmstqrwxuvhjyznp"
};

static const char *odd_neighbors[] = {
  "bc01fg45238967deuvhjyznpkmstqrwx",
  "p0r21436x8zb9dcf5h7kjnmqesgutwvy",
  "238967debc01fg45kmstqrwxuvhjyznp",
  "14365h7k9dcfesgujnmqp0r2twvyx8zb"
};

static const char *even_borders[] = { "prxz", "bcfguvyz", "028b", "0145hjnp" };
static const char *odd_borders[] = { "bcfguvyz", "prxz", "0145hjnp", "028b" };

inline static int
index_for_char (char ch, const char *str)
{
#if 1
  if (str) {
    const char *tcp = str;
    while (*tcp && *tcp != ch) tcp++;
    if (*tcp) return tcp - str;
   }
   return -1;
#else
  int index = -1;
  int len = strlen (str);
  int i;
  for (i = 0; i < len; i++)
    {

      if (ch == str[i])
	{

	  index = i;
	  break;
	}

    }

  return index;
#endif
}


static inline char _f_base_32 (int d)
{
  return d >= 0 ? char_map[d % 33] : '\0';
}

static inline int _t_base_32 (char ch)
{
#if 1
  return char_map_val[ch & 127];
#else
  return index_for_char (ch, char_map);
#endif
}


static char *
get_neighbor (const char *hash, enum COMPASS direction)
{
  size_t hash_length = strlen (hash);
  char last_char = hash[hash_length - 1];

  char *base = (char *) malloc (hash_length + 1);

  int is_odd = hash_length % 2;
  const char **border = is_odd ? odd_borders : even_borders;
  const char **neighbor = is_odd ? odd_neighbors : even_neighbors;

  if (hash_length > 1)
    memcpy (base, hash, hash_length - 1);

  if (index_for_char (last_char, border[direction]) != -1)
    {
      char *tcp = base;
      tcp[hash_length - 1] = '\0';
      base = get_neighbor (tcp, direction);
      free (tcp);		// free memory
    }

  int neighbor_index = index_for_char (last_char, neighbor[direction]);
  last_char = _f_base_32 (neighbor_index);	//  char_map[neighbor_index];

  base[hash_length - 1] = last_char;
  base[hash_length] = '\0';

  return base;
}


#ifdef __cplusplus
namespace _private_geo {
#endif

const char *
geohash_encode (const double lat, const double lng, int precision)
{

  if (precision < 1 || precision > 12)
    precision = 6;

  char *hash = NULL;

  if (lat <= 90.0 && lat >= -90.0 && lng <= 180.0 && lng >= -180.0)
    {

      hash = (char *) malloc (sizeof (char) * (precision + 1));
      hash[precision] = '\0';

      precision *= 5.0;

      Interval lat_interval = { MAX_LAT, MIN_LAT };
      Interval lng_interval = { MAX_LNG, MIN_LNG };

      Interval *interval;
      double coord, mid;
      int is_even = 1;
      unsigned int hashChar = 0;
      size_t i;
      for (i = 1; i <= precision; i++)
	{

	  if (is_even)
	    {

	      interval = &lng_interval;
	      coord = lng;

	    }
	  else
	    {

	      interval = &lat_interval;
	      coord = lat;
	    }

	  mid = (interval->low + interval->high) / 2.0;
	  hashChar = hashChar << 1;

	  if (coord > mid)
	    {

	      interval->low = mid;
	      hashChar |= 0x01;

	    }
	  else
	    interval->high = mid;

	  if (!(i % 5))
	    {

	      hash[(i - 1) / 5] = char_map[hashChar];
	      hashChar = 0;

	    }

	  is_even = !is_even;
	}


    }

  return hash;
}

GeoCoord
geohash_decode (const char *hash)
{

  GeoCoord coordinate = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  if (hash)
    {
      size_t char_amount = strlen (hash);
      if (char_amount)
	{
	  /* NOTE: If we see upper case letters in a hash that means it is probably
	   * Maidenhead Locator System
	   */
	  if (isupper(hash[0]))
		return maidenhead_deocde(hash);

	  unsigned int char_mapIndex;
	  Interval lat_interval = { MAX_LAT, MIN_LAT };
	  Interval lng_interval = { MAX_LNG, MIN_LNG };
	  Interval *interval;

	  int is_even = 1;
	  double delta;
	  size_t i, j;
	  for (i = 0; i < char_amount; i++)
	    {

	      char_mapIndex = _t_base_32(hash[i]);   // index_for_char (hash[i], char_map);

	      if (char_mapIndex < 0)
		break;

	      // Interpret the last 5 bits of the integer
	      for (j = 0; j < 5; j++)
		{

		  interval = is_even ? &lng_interval : &lat_interval;

		  delta = (interval->high - interval->low) / 2.0;

		  if ((char_mapIndex << j) & 0x0010)
		    interval->low += delta;
		  else
		    interval->high -= delta;

		  is_even = !is_even;
		}

	    }

	  coordinate.latitude =
	    lat_interval.high -
	    ((lat_interval.high - lat_interval.low) / 2.0);
	  coordinate.longitude =
	    lng_interval.high -
	    ((lng_interval.high - lng_interval.low) / 2.0);

	  coordinate.north = lat_interval.high;
	  coordinate.east = lng_interval.high;
	  coordinate.south = lat_interval.low;
	  coordinate.west = lng_interval.low;
	}
    }

  return coordinate;
}


char **
geohash_neighbors (const char *hash)
{

  char **neighbors = NULL;

  if (hash)
    {

      // N, NE, E, SE, S, SW, W, NW
      neighbors = (char **) malloc (sizeof (char *) * 8);

      neighbors[N] = get_neighbor (hash, NORTH); // North
      neighbors[NE] = get_neighbor (neighbors[N], EAST); // North East
      neighbors[E] = get_neighbor (hash, EAST); // East
      neighbors[SE] = get_neighbor (neighbors[E], SOUTH); // South East
      neighbors[S] = get_neighbor (hash, SOUTH); // South
      neighbors[SW] = get_neighbor (neighbors[S], WEST); // South West
      neighbors[W] = get_neighbor (hash, WEST); // West
      neighbors[NW] = get_neighbor (neighbors[W], NORTH); // North West

    }

  return neighbors;
}

GeoBoxDimension
geohash_dimensions_for_precision (int precision)
{

  GeoBoxDimension dimensions = { 0.0, 0.0 };

  if (precision > 0)
    {

      int lat_times_to_cut = precision * 5 / 2;
      int lng_times_to_cut = precision * 5 / 2 + (precision % 2 ? 1 : 0);

      double width = 360.0;
      double height = 180.0;

      int i;
      for (i = 0; i < lat_times_to_cut; i++)
	height /= 2.0;

      for (i = 0; i < lng_times_to_cut; i++)
	width /= 2.0;

      dimensions.width = width;
      dimensions.height = height;

    }

  return dimensions;
}


GeoCoord maidenhead_deocde(const char *hash)
{
  GeoCoord coordinate = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  if (hash) {
    abort(); // ("Maidenthead decode not yet implemented"); 
  }
  return coordinate;
};


char * 
maidenhead_encode (const double latitude, const double longitude)
{
  const size_t pairs = 4;
  const double scaling[] = { 360., 360. / 18., (360. / 18.) / 10.,
    ((360. / 18.) / 10.) / 24., (((360. / 18.) / 10.) / 24.) / 10.,
    ((((360. / 18.) / 10.) / 24.) / 10.) / 24.,
    (((((360. / 18.) / 10.) / 24.) / 10.) / 24.) / 10.
  };

  char *hash = (char *)malloc (sizeof(char)*pairs*2);
  if (hash) {
    size_t i;
    for (i = 0; i < pairs; i++)
      {
	int index;
        index = (int) floor (fmod ((180.0 + longitude), scaling[i]) / scaling[i + 1]);
        hash[i * 2] = (i & 1) ? 0x30 + index : (i & 2) ? 0x61 + index : 0x41 + index;
        index = (int) floor (fmod ((90.0 + latitude), (scaling[i] / 2)) / (scaling[i + 1] / 2));
        hash[i * 2 + 1] = (i & 1) ? 0x30 + index : (i & 2) ? 0x61 + index : 0x41 + index;
      }
    hash[pairs * 2] = 0;
  }
  return hash; 
}

#ifdef EBUG
int main(int argc, char **argv)
{
  struct position location;
  char code[10];

 // Encodes latitude and longitude into a Plus+Code.
  location.lat = 47.0000625;
  location.lon = 8.0000625;

  if (argc > 2)
    location.lat = atof(argv[1]),   location.lon = atof(argv[2]);
  positionToMaidenhead(&location, code);
  printf("%f,%f --> %s\n", location.lat, location.lon, code);

}
#endif


#ifdef __cplusplus
};
#endif
