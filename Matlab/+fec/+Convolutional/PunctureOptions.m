classdef PunctureOptions < hgsetget
    properties
        mask;
        tailMask = [];
    end

    methods
        function self = PunctureOptions(varargin)
            if (nargin == 1 && isa(varargin{1}, 'fec.Convolutional.PunctureOptions'))
                self.set(varargin{1}.get());
            elseif (nargin == 1 && isstruct(varargin{1}))
                self.set(varargin{1});
            else
                if (~isempty(varargin))
                    self.mask = varargin{1};
                end
                if (~isempty(varargin(2:end)))
                    self.set(varargin{2:end});
                end
            end
        end
    end
end