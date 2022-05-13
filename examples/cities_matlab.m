function cities_matlab
  % VP-tree example code for Octave

  k = 5;
  nbr_dist = 1000.0;

  % Get list of cities
  city_list = cities_parse;
  n_cities = numel(city_list);
  
  % Initialize VP-tree
  distance_handle = @(x, y) great_circle_distance(x, y);
  vp = VPTree(distance_handle);
  
  % Add cities
  for i=1:n_cities
      vp.add_point(city_list(i));
  end
  
  % Nearest neighbor query
  query = city_list(randi(n_cities));
  nbrs = vp.nearest_neighbor(query, k+1);
  
  fprintf('%d nearest neighbors of %s:\n', k, query.name);
  print_city_neighbors(query, nbrs);
  
  % Epsilon-distance query
  query = city_list(randi(n_cities));
  nbrs = vp.neighborhood(query, nbr_dist);
  
  fprintf('Cities within %.0fkm of %s:\n', nbr_dist, query.name);
  print_city_neighbors(query, nbrs);
  
  % Cleanup
  delete(vp);
end