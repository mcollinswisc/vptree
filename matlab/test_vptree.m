function test_vptree()
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
    
    vp = vptree_mex('create', distance);
    
    X = rand(d, n);
    tic;
    for i=1:n
        vptree_mex('add', vp, X(:, i))
    end
    toc
    
    tic;
    query = rand(d, 1);
    nbrs = vptree_mex('nearest_neighbor', vp, query, k);
    toc
    
    tic;
    alldist = sqrt(sum((X - repmat(query, 1, n)) .^ 2, 1));
    alldist = sort(alldist);
    toc
    alldist = alldist(1:k)
    
    vpdist = neighbor_distances(nbrs, query)
    
    tic;
    nbrs = vptree_mex('nearest_neighbor_approx', vp, query, k, 32);
    toc
    approxvpdist = neighbor_distances(nbrs, query)
    
    tic;
    nbrs = vptree_mex('neighborhood', vp, query, alldist(end) * 1.0001);
    toc
    epdist = neighbor_distances(nbrs, query)
    
    incnn = vptree_mex('incnn_begin', vp, query);
    for i=1:k
        nbr = vptree_mex('incnn_next', vp, incnn);
        fprintf(1, '%d %g\n', i, norm(nbr - query, 2));
    end
    vptree_mex('incnn_destroy', vp, incnn);
    
    vptree_mex('destroy', vp)
end