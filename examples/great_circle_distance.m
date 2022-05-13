function d = great_circle_distance(x, y)
  % d = great_circle_distance(x, y)
  %
  %   Calculates the great-circle distance between cities x and y.
  %   The arguments are structs with members 'latitude' and
  %   'longitude'.
  %

  function h = haversined(x)
    h = sind(x/2);
    h = h * h;
  end

  latx = x.latitude;
  laty = y.latitude;
  dlon = x.longitude - y.longitude;
    
  d = haversined(latx - laty) + cosd(latx) * cosd(laty) * haversined(dlon);
  d = 6378.1 * 2 * asin(sqrt(d));
end