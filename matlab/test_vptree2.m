function test_vptree2()
    function d = neighbor_distances(nbrs, query)
        k = numel(nbrs);
        d = zeros(1, k);
        for i=1:numel(nbrs)
            d(i) = norm(query - nbrs{i}, 2);
        end
    end
    
    
    d = 2;
    n = 1000;
    k = 4;
    
    
    distance = @(x, y) norm(x - y, 2);
    
    vp = VPTree(distance);
    
    
    X = rand(d, n);
    tic;
    for i=1:n
        vp.add_point(X(:, i));
    end
    toc
    
    tic;
    query = rand(d, 1);
    nbrs = vp.nearest_neighbor(query, k);
    toc
    
    tic;
    alldist = sqrt(sum((X - repmat(query, 1, n)) .^ 2, 1));
    alldist = sort(alldist);
    toc
    alldist = alldist(1:k)
    
    vpdist = neighbor_distances(nbrs, query)
    
    tic;
    nbrs = vp.nearest_neighbor_approx(query, k, 32);
    toc
    approxvpdist = neighbor_distances(nbrs, query)
    
    tic;
    nbrs = vp.neighborhood(query, alldist(end) * 1.0001);
    toc
    epdist = neighbor_distances(nbrs, query)
    
    
    incnn = vp.incremental_neighbors(query);
    for i=1:k
        nbr = incnn.next();
        fprintf(1, '%d %g\n', i, norm(nbr - query, 2));
    end
    
    delete(incnn)
    delete(vp)
end