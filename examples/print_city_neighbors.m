function print_city_neighbors(query, nbr_list)
  % print_city_neighbors(query, nbr_list)
  %
  %   Print the result of a VP-tree query.
  %

  for i=1:numel(nbr_list)
    if strcmp(query.name, nbr_list{i}.name)
        continue
    end
    
    dist = great_circle_distance(query, nbr_list{i});
    fprintf('  %s (%.1fkm)\n', nbr_list{i}.name, dist);
  end
end