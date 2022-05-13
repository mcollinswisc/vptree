from __future__ import print_function, division
import sys
import pyvptree
import math
import re
import random

class City:
    def __init__(self, name, latitude, longitude):
        self.name = name

        if type(latitude) == str:
            self.latitude = City.parse_coord(latitude)
        else:
            self.latitude = latitude

        if type(longitude) == str:
            self.longitude = City.parse_coord(longitude)
        else:
            self.longitude = longitude

    coord_re = re.compile('([0-9]+)\xc2\xb0([0-9]+)\'([NESW])')

    @staticmethod
    def parse_coord(coord_str):
        m = City.coord_re.match(coord_str)
        if m is None:
            return None

        deg, min, dir = m.groups()
        deg = float(deg) + float(min) / 60

        if dir in ['S', 'W']:
            deg *= -1


        return deg

    @staticmethod
    def load_cities(fp):
        if type(fp) == str:
            fp = open(fp, 'r')

        for line in fp:
            line = line.strip()

            if line[0] != '"':
                continue
            i = line[1:].find('"') + 1
            name = line[1:i]

            coords = line[i+1:]
            coords = coords.split()
            latitude, longitude = tuple(coords)

            yield City(name, latitude, longitude)

    def __repr__(self):
        return str(self)

    def __str__(self):
        return '%s (%.2f,%.2f)' % (self.name, self.latitude, self.longitude)


class CityVPTree(pyvptree.VPTree):
    def distance(self, c1, c2):
        def haversine(x):
            return math.sin(x / 2) ** 2

        lat1 = math.radians(c1.latitude)
        lat2 = math.radians(c2.latitude)

        dlat = lat1 - lat2
        dlon = math.radians(c1.longitude - c2.longitude)

        dist = haversine(dlat) + math.cos(lat1) * math.cos(lat2) * haversine(dlon)
        dist = 2 * math.asin(math.sqrt(dist))

        dist *= 6378.1

        return dist

def main(argv):
    # Parse cities
    if len(argv) <= 1:
        cities_path = 'examples/cities.txt'
    else:
        cities_path = argv[1]

    cities = list(City.load_cities(cities_path))

    # Create vp tree
    vp = CityVPTree()
    vp.add_many(cities)

    # Nearest neighbor query
    k = 5
    query = random.choice(cities)
    nbrs = vp.nearest_neighbors(query, k+1)
    nbrs.remove(query)

    print('%d nearest neighbors of %s:' % (k, query.name))
    for nbr in nbrs:
        print('  %s (%.1fkm)' % (nbr.name, vp.distance(query, nbr)))

    # Neighborhood query
    distance = 1000.0
    query = random.choice(cities)
    nbrs = vp.neighborhood(query, distance)
    nbrs.remove(query)

    print('Cities within %.0fkm of %s:' % (distance, query.name))
    for nbr in nbrs:
        print('  %s (%.1fkm)' % (nbr.name, vp.distance(query, nbr)))
    

if __name__ == '__main__':
    main(sys.argv)

