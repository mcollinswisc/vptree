function city_list = cities_parse(path)
  if ~exist('path', 'var')
    path = mfilename('fullpath');
    [path, mname, mext] = fileparts(path);
    path = [path, filesep, 'cities.txt'];
  end

  function coord = parse_coord(coord_str)
    elts = strsplit(coord_str, '°');
    degrees = str2num(elts{1});
    elts = strsplit(elts{2}, '''');
    minutes = str2num(elts{1});
    dir = elts{2};
    
    coord = degrees + minutes / 60.0;
    if strcmp(dir, 'W') || strcmp(dir, 'S')
      coord = -coord;
    elseif ~strcmp(dir, 'E') && ~strcmp(dir, 'N')
      coord = NaN;
    end
  end

  city_list = {};
  fid = fopen(path, 'r');
  
  tline = fgetl(fid);
  while ischar(tline)
    % Separate out name, appears in double-quotes
    name_start = strfind(tline, '"');
    if length(name_start) == 0
      continue
    end
    name_end = strfind(tline((name_start+1):end), '"') + name_start;
    if length(name_end) == 0
      continue
    end
    name = tline((name_start+1):(name_end-1));
      
    % Parse coordinates
    latitude = NaN;
    longitude = NaN;
      
    coord_strs = tline((name_end+1):end);
    coord_strs = strsplit(coord_strs);
    for j=1:numel(coord_strs)
      if sum(isspace(coord_strs{j})) == length(coord_strs{j})
        continue
      end
          
      if isnan(latitude)
        latitude = parse_coord(coord_strs{j});
      else
        longitude = parse_coord(coord_strs{j});
      end
    end
      
    if isnan(latitude) || isnan(longitude)
      continue
    end
     
    % Add to list of cities
    new_city = struct('name', name, ...
                      'latitude', latitude, ...
                      'longitude', longitude');
    
    city_list{numel(city_list)+1} = new_city;
                    
      
    tline = fgetl(fid);
  end
  
  fclose(fid);
  city_list = cell2mat(city_list);
end