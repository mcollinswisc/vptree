#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <exception>

#include "vptree/vptree.hh"

using namespace std;

class City
{
public:
  City(const string &str);

  string &name();
  const string &name() const;

  double &latitude();
  double latitude() const;

  double &longitude();
  double longitude() const;

  static double parseNum(const string &);
  static double parseCoord(const string &);

  static vector<City> parseCityFile(istream &stream);
  static vector<City> parseCityFile(const string &path);

  class ParseError: public exception
  {
  public:
    ParseError(const char *type, const std::string &bad_str) throw();

    virtual const char *what() const throw();

  private:
    char buf[256];
  };

  bool operator == (const City &) const;

private:
  string name_;
  double latitude_;
  double longitude_;
};

ostream &operator << (ostream &stream, const City &city);

class CityVPTree: public VPTree<City>
{
public:
  virtual double distance(const City &city1, const City &city2);

protected:
  static double deg2rad(double deg);
  static double haversine(double x);
};

int main(int argc, char **argv)
{
  srand(time(NULL));

  cout<<fixed<<setprecision(1);

  // Read in cities from file
  string cities_path;
  if(argc <= 1) {
    cities_path = "examples/cities.txt";
  }
  else {
    cities_path = argv[1];
  }

  vector<City> cities = City::parseCityFile(cities_path);
  int num_cities = static_cast<int>(cities.size());
  
  // Set up the VP-tree
  CityVPTree vp;
  vp.addMany(cities.begin(), cities.end());

  // Do a nearest neighbor query
  {
    const City &query = cities[rand() % num_cities];
    int k = 5;
    vector<const City *> nbrs = vp.nearestNeighbors(query, k+1);

    cout<<k<<" nearest neighbors of "<<query.name()<<":"<<endl;
    for(int i = 0; i < k; ++i) {
      if(*(nbrs[i]) == query) {
	continue;
      }
    
      double distance = vp.distance(query, *(nbrs[i]));
      cout<<"  "<<nbrs[i]->name()<<" ("<<distance<<"km"<<')'<<endl;
    }
  }

  // Do an eps-neighborhood query
  {
    const City &query = cities[rand() % num_cities];
    double distance = 1000.0;
    vector<const City *> nbrs = vp.neighborhood(query, distance);

    cout<<"Cities within ";
    cout<<setprecision(0)<<distance<<setprecision(1);
    cout<<"km of "<<query.name()<<':'<<endl;
    for(size_t i = 0; i < nbrs.size(); ++i) {
      if(query == *(nbrs[i])) {
	continue;
      }

      distance = vp.distance(query, *(nbrs[i]));
      cout<<"  "<<nbrs[i]->name()<<" ("<<distance<<"km"<<')'<<endl;
    }
  }

  return 0;
}


City::City(const string &str)
{
  size_t name_start = str.find('"');
  if(name_start == string::npos) {
    throw ParseError("city", str.c_str());
  }

  size_t name_end = str.find('"', name_start + 1);
  if(name_start == string::npos) {
    throw ParseError("city", str.c_str());
  }

  name_ = str.substr(name_start+1, name_end-name_start-1);

  stringstream sstream(str.substr(name_end+1), ios::in);

  string coord_str;
  sstream>>coord_str;
  latitude_ = parseCoord(coord_str);

  sstream>>coord_str;
  longitude_ = parseCoord(coord_str);
}

string &City::name()
{
  return name_;
}

const string &City::name() const
{
  return name_;
}

double &City::latitude()
{
  return latitude_;
}

double City::latitude() const
{
  return latitude_;
}

double &City::longitude()
{
  return longitude_;
}

double City::longitude() const
{
  return longitude_;
}

City::ParseError::ParseError(const char *type, const string &bad_str) throw()
{
  snprintf(buf, 256, "Error in parsing %s: \"%s\"", type, bad_str.c_str());
  buf[255] = '\0';
}

const char *City::ParseError::what() const throw()
{
  return buf;
}

double City::parseNum(const string &str)
{
  const char *s = str.c_str();
  const char *end = s + str.size();

  char *endptr;
  double d = strtod(s, &endptr);

  if(endptr != end) {
    throw ParseError("number", str);
  }

  return d;
}

double City::parseCoord(const string &str)
{
  size_t d = str.find("\xc2\xb0");
  if(d == string::npos) {
    throw ParseError("coordinate", str);
  }

  size_t m = str.find('\'', d+2);
  if(m == string::npos) {
    throw ParseError("coordinate", str);
  }

  size_t c;
  for(c = m+1; c < str.size() && str[c] != 'N' && str[c] != 'S' && str[c] != 'E' && str[c] != 'W'; ++c);
  if(c == str.size()) {
    throw ParseError("coordinate", str);
  }

  double deg = parseNum(str.substr(0, d));
  double min = parseNum(str.substr(d+2, m - d - 2));
  double sign = (str[c] == 'S' || str[c] == 'W') ? -1.0 : 1.0;

  return sign * (deg + min / 60.0);
}

vector<City> City::parseCityFile(istream &stream)
{
  vector<City> cities;
  string line;

  while(!stream.eof()) {
    getline(stream, line);
    if(line.size() == 0 || stream.eof()) {
      break;
    }

    try {
      cities.push_back(City(line));
    }
    catch(ParseError) { // ignore malformed cities
    }
  }

  return cities;
}

vector<City> City::parseCityFile(const string &path)
{
  ifstream stream(path.c_str());
  vector<City> cities = parseCityFile(stream);
  stream.close();
  return cities;
}

bool City::operator == (const City &other) const
{
  return name_ == other.name_ && latitude_ == other.latitude_ && longitude_ == other.longitude_;
}

ostream &operator << (ostream &stream, const City &city)
{
  stream<<city.name()<<" ("<<city.latitude()<<", "<<city.longitude()<<')';
}

double CityVPTree::deg2rad(double deg)
{
  return deg * M_PI / 180.0;
}

double CityVPTree::haversine(double x)
{
  double s = sin(x / 2.0);
  return s*s;
}

double CityVPTree::distance(const City &city1, const City &city2)
{
  double lat1 = deg2rad(city1.latitude());
  double lat2 = deg2rad(city2.latitude());
  double dlon = deg2rad(city1.longitude() - city2.longitude());

  double dist = haversine(lat1 - lat2) + cos(lat1) * cos(lat2) * haversine(dlon);
  dist = 2.0 * asin(sqrt(dist));

  dist *= 6378.1; // radius of earth in km

  return dist;
}
