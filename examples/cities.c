#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#include "vptree/vptree.h"

typedef struct
{
  char *name;
  double latitude;
  double longitude;
} city;

static double deg2rad(double d);
static double parse_coord(const char *coord_begin, const char *coord_end, int lat);
static int parse_city(const char *city_str, city *out);
static city *parse_city_file(FILE *fp, int *num_cities);
static city *parse_city_path(const char *path, int *num_cities);
static void cleanup_cities(int num_cities, city *cities_list);

/**
 * Calculates the great circle distance between two cities using the
 * Haversine formula.
 */
static double great_circle_distance(void *user_data, const void *c1, const void *c2);

int main(int argc, char **argv)
{
  const char *cities_path;
  int i, num_cities;
  city *cities;

  vptree_options vpopts;
  vptree *vp;
  const void **ptrs, **nbrs;

  city *query;
  int k = 5;
  double dist, query_dist = 1000;

  srand(time(NULL));

  // Load the list of city latitude/longitudes from a file
  if(argc <= 1) {
    cities_path = "examples/cities.txt";
  }
  else {
    cities_path = argv[1];
  }

  cities = parse_city_path(cities_path, &num_cities);

  // Set up VP-tree
  vpopts = vptree_default_options;
  vpopts.user_data = NULL;
  vpopts.distance = great_circle_distance;

  vp = vptree_create(sizeof(vpopts), &vpopts);
  
  // Populate VP-tree
  ptrs = (const void **)malloc(sizeof(const void *) * num_cities);
  for(i = 0; i < num_cities; ++i) {
    ptrs[i] = &cities[i];
  }
  vptree_add_many(vp, num_cities, ptrs);

  // Do a nearest neighbor query
  query = &cities[rand() % num_cities];
  vptree_nearest_neighbor(vp, query, k+1, ptrs);
  
  printf("%d nearest neighbors of %s:\n", k, query->name);
  for(i = 0; i < k+1; ++i) {
    if(query == ptrs[i]) {
      continue;
    }

    dist = great_circle_distance(NULL, query, ptrs[i]);
    printf("  %s (%.1fkm)\n", ((city *)ptrs[i])->name, dist);
  }

  // Do an eps-neighborhood query
  query = &cities[rand() % num_cities];
  nbrs = vptree_neighborhood(vp, query, query_dist, &k);

  printf("Cities within %.0fkm of %s:\n", query_dist, query->name); 
  for(i = 0; i < k; ++i) {
    if(query == nbrs[i]) {
      continue;
    }

    dist = great_circle_distance(NULL, query, nbrs[i]);
    printf("  %s (%.1fkm)\n", ((city *)nbrs[i])->name, dist);
  }

  free(nbrs);
  free(ptrs);

  vptree_destroy(vp);
  cleanup_cities(num_cities, cities);
  return 0;
}

static double deg2rad(double d)
{
  return d * M_PI / 180.0;
}

static double haversine(double x)
{
  double s;
  s = sin(x / 2.0);
  return s*s;
}

static double great_circle_distance(void *user_data, const void *c1, const void *c2)
{
  const city *city1, *city2;
  double lat1, lat2, dlat, dlon, dist;

  city1 = (const city *)c1;
  city2 = (const city *)c2;

  lat1 = deg2rad(city1->latitude);
  lat2 = deg2rad(city2->latitude);
  dlon = deg2rad(city1->longitude - city2->longitude);

  dist = haversine(lat1 - lat2) + cos(lat1) * cos(lat2) * haversine(dlon);
  dist = 2.0 * asin(sqrt(dist));

  dist *= 6378.1; // radius of earth in km

  return dist;
}

static double parse_coord(const char *coord_begin, const char *coord_end, int lat)
{
  int nparsed, degrees, minutes;
  char dir;
  double coord;

  nparsed = sscanf(coord_begin, "%d°%d'%c", &degrees, &minutes, &dir);
  if(nparsed != 3) {
    return NAN;
  }

  coord = (double)degrees + ((double)minutes) / 60.0;
  if(lat) {
    if(dir == 'S') {
      coord = -coord;
    }
    else if(dir != 'N') {
      return NAN;
    }
  }
  else {
    if(dir == 'W') {
      coord = -coord;
    }
    else if(dir != 'E') {
      return NAN;
    }
  }

  return coord;
}

static int parse_city(const char *city_str, city *out)
{
  const char *p, *end;
  char *city_name;

  for(p = city_str; isspace(*p); ++p);
  if(*p != '"') {
    return -1;
  }

  // Get city name (enclosed by quotes)
  for(end = p + 1; *end != '"' && *end != '\0'; ++end);
  if(*end != '"') {
    return -1;
  }
    
  city_name = (char *)malloc(end - p);
  strncpy(city_name, p+1, end - p - 1);
  city_name[end-p-1] = '\0';
  out->name = city_name;

  // Tokenize coordinates
  for(p = end+1; isspace(*p); ++p);
  for(end = p; *end != '\0' && !isspace(*end); ++end);
  if(*end == '\0') {
    free(city_name);
    return -1;
  }
  out->latitude = parse_coord(p, end, 1);

  for(p = end+1; isspace(*p); ++p);
  for(end = p; *end != '\0' && !isspace(*end); ++end);
  if(*end == '\0') {
    free(city_name);
    return -1;
  }
  out->longitude = parse_coord(p, end, 0);


  return 0;
}

static city *parse_city_file(FILE *fp, int *num_cities)
{
  int status, n;
  city *cities, new_city;
  char *line, *p;
  size_t len;
  ssize_t nread;

  n = 0;
  cities = NULL;

  len = 0;
  line = NULL;
  while(!feof(fp)) {
    nread = getline(&line, &len, fp);
    if(nread < 0) {
      break;
    }

    status = parse_city(line, &new_city);
    if(status != 0) {
      continue;
    }

    n++;
    cities = realloc(cities, sizeof(city) * n);
    cities[n-1] = new_city;
  }

  free(line);

  *num_cities = n;
  return cities;
}

static city *parse_city_path(const char *path, int *num_cities)
{
  FILE *fp;
  city *cities;

  fp = fopen(path, "r");
  if(fp == NULL) {
    *num_cities = 0;
    return NULL;
  }

  cities = parse_city_file(fp, num_cities);

  fclose(fp);

  return cities;
}

static void cleanup_cities(int num_cities, city *cities_list)
{
  int i;

  for(i = 0; i < num_cities; ++i) {
    free(cities_list[i].name);
  }
  free(cities_list);
}
