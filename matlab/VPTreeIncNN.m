classdef VPTreeIncNN < handle
    properties (Access = private)
        vp
        incnn
    end
    
    methods
        function obj = VPTreeIncNN(vp, query)
            obj.vp = vp;
            obj.incnn = vptree_mex('incnn_begin', vp, query);
        end
        
        function nbr = next(obj)
            nbr = vptree_mex('incnn_next', obj.vp, obj.incnn);
        end
        
        function delete(obj)
            vptree_mex('incnn_destroy', obj.vp, obj.incnn);
        end
    end
end