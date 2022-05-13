function cities_octave
  % VP-tree example code for Octave

  k = 5;
  nbr_dist = 1000.0;

  % Get list of cities
  city_list = cities_parse;
  n_cities = numel(city_list);
  
  % Initialize VP-tree
  distance_handle = @(x, y) great_circle_distance(x, y);
  vp = vptree_mex('create', distance_handle);
  
  % Add cities
  for i=1:n_cities
      vptree_mex('add', vp, city_list(i));
  end
  
  % Nearest neighbor query
  query = city_list(randi(n_cities));
  nbrs = vptree_mex('nearest_neighbor', vp, query, k+1);
  
  fprintf('%d nearest neighbors of %s:\n', k, query.name);
  print_city_neighbors(query, nbrs);
  
  % Epsilon-distance query
  query = city_list(randi(n_cities));
  nbrs = vptree_mex('neighborhood', vp, query, nbr_dist);
  
  fprintf('Cities within %.0fkm of %s:\n', nbr_dist, query.name);
  print_city_neighbors(query, nbrs);
  
  % Cleanup
  vptree_mex('destroy', vp);
end