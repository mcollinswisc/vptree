classdef VPTree < handle
    properties (Access = private)
        vp
    end
    
    methods
        function obj = VPTree(distance)
            % obj = VPTree(distance)
            %
            %   Initialize the VP-tree object.
            %
            % distance: Handle to a function that accepts two arguments
            %           and returns the distance between them.
            
            obj.vp = vptree_mex('create', distance);
        end
        
        function delete(obj)
            vptree_mex('destroy', obj.vp);
        end
        
        function p = add_point(obj, pt)
            % p = obj.add_point(pt)
            %
            %   Adds a single point to the vptree.
            %
            % pt: May be any Matlab object, so long as the distance
            %     handle passed to the constructor accepts it as an argument.
                
            
            vptree_mex('add', obj.vp, pt);
        end
        
        function nbrs = nearest_neighbor(obj, query, k)
            % nbrs = obj.nearest_neighbor(query, k)
            %
            %   Perform a k-nearest neighbor query.
            %
            % query: The query point.
            %     k: The number of neighbors to return.
            
            nbrs = vptree_mex('nearest_neighbor', obj.vp, query, k);
        end
        
        function nbrs = nearest_neighbor_approx(obj, query, k, max_nodes)
            % nbrs = obj.nearest_neighbor_approx(query, k, max_nodes)
            %
            %   Perform an approximate k-nearest neighbor query.
            %   Visits a limited number of nodes in a priority
            %   order, and returns the best k neighbors found in
            %   this limited search.
            %
            %
            %     query: The query point.
            %         k: The number of neighbors to return.
            % max_nodes: The maximum number of nodes to visit in
            %            the search.
            
            nbrs = vptree_mex('nearest_neighbor_approx', obj.vp, ...
                              query, k, max_nodes);
        end
        
        function nbrs = neighborhood(obj, query, max_dist)
            % nbrs = obj.neighborhood(query, max_dist)
            %
            %   Perform an epsilon-neighbor search.  Finds all
            %   points in the VP-tree (strictly) within max_dist of the query
            %   point.
            %
            %     query: The query point.
            %  max_dist: Returns all neighbors within this distance
            %            from the query.
                
            nbrs = vptree_mex('neighborhood', obj.vp, query, max_dist);
        end
        
        function incnn = incremental_neighbors(obj, query)
           % incnn = obj.incremental_neighbors(query)
           %
           %   Performs an incremental nearest neighbor search on
           %   query.  Returns a object 'incnn' with method 'next'.
           %   Repeated calls to the 'next' method will return the
           %   points in the VP-tree in order of increasing
           %   distance from query.  This can return an arbitrary
           %   number of such neighbors.
               
           incnn = VPTreeIncNN(obj.vp, query);
        end
    end
end
