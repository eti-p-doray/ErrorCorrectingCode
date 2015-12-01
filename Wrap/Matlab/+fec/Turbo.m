
%>  This class represents a turbo encode / decoder.
%>  It offers methods to encode and to decode data given a Turbo::Structure.
%>
%>  The structure of the parity bits generated by a Turbo object is as follow
%>
%>    | syst | systTail | convOutput1 | tailOutpu1 | convOutput2 | tailOutpu2 | ... |
%>
%>  where syst are the systematic bits, systTail are the tail bit
%>  added to the msg for termination of the constituents 1, 2, ..., respectively,
%>  convOutputX and tailOutputX are the output parity of the  msg and the tail
%>  generated by the constituent convolutional code X.
%>
%>  The structure of the extrinsic information
%>
%>    | msg1 | systTail1 | msg2 | systTail2 | ... |
%>
%>  where msgX are the extrinsic msg L-values generated by the constituent code X
%>  and systTailX are the tail bit added to the msg
%>  for termination of the constituent X.
classdef Turbo < fec.Codec
    properties (Dependent = true, Hidden)
        %>  Turbo::EncoderOptions
        encoderOptions
        %>  Turbo::DecoderOptions
        decoderOptions
    end
    properties (Dependent = true)
        %>  Number of iterations in decoder.
        iterations;
        %>  DecoderAlgorithm type used in decoder.
        algorithm;
        %>  Turbo::Scheduling type used in decoder.
        scheduling;
    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Turbo();
            self.reload(s);
        end
    end

    methods
        %>  Turbo constructor.
        %>  `codec = fec.Turbo(trellis, interleavers, Name, Value)` creates a Turbo object from a Trellis and interleaver indices. Optionally, additional properties can be specified using Name (string inside single quotes) followed by its Value. See Turbo::EncoderOptions and Turbo::DecoderOptions for a list of properties.
        %>
        %> `codec = fec.Turbo(encoderOptions, decoderOptions)` creates a Turbo object from the Turbo::EncoderOptions and the Turbo::DecoderOptions structures, cell array or object containing encoder and decoder properties which describes the codec.
        %>
        %>  See example.
        %>  @snippet Turbo.m  Creating a simple Turbo Codec
        function self = Turbo(varargin)
            if (nargin > 0)
              self.structure = fec.Turbo.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Turbo_constructor), self.structure.getEncoderOptions, self.structure.getDecoderOptions);
            end
        end
        %> Creates a permutation of parity bits from Turbo::PunctureOptions structure, object or NAME, Value list. The permutation can then transform (Permutation::permute() and then Permutation::dePermute()) the parity bits generated by the same Turbo Codec.
        %> Note that the this function will not make the objet to apply the puncture pattern. You will have to use the created permutation to Permutation::permute() after the encode() method and Permutation::dePermute() before the decode() method. If you want to create a Turbo Codec with a single puncturing permutation, see PuncturedTurbo.
        %>
        %>  @param  punctureOptions structure, object or Name, Value list containing the permutation properties.
        %>  @return Generated permutation that will apply the specify punctureOptions
        function perms = puncturing(self, varargin)
            options = fec.Turbo.PunctureOptions(varargin{:});
            perms = fec.Permutation(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Turbo_puncturing), self, options.get()), self.paritySize);
        end
        %>  Access the Turbo::iterations property.
        function val = get.iterations(self)
            val = self.decoderOptions.iterations;
        end
        %>  Access the Turbo::algorithm property.
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        %>  Access the Turbo::scheduling property.
        function val = get.scheduling(self)
            val = fec.Turbo.Scheduling(self.decoderOptions.scheduling).char;
        end
        %>  Modify the Turbo::iterations property.
        function set.iterations(self, val)
            self.setDecoderOptions('iterations', val);
        end
        %>  Modify the Turbo::algorithm property.
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
        end
        %>  Access the Turbo::scheduling property.
        function set.scheduling(self, val)
            self.setDecoderOptions('scheduling', val);
        end
        %>  Access the Turbo::decoderOptions property.
        function val = get.decoderOptions(self)
            val = self.structure.decoderOptions;
        end
        %>  Access the Turbo::encoderOptions property.
        function val = get.encoderOptions(self)
            val = self.structure.encoderOptions;
        end
        %>  Modify the Turbo::decoderOptions property.
        function set.decoderOptions(self,val)
            self.structure.decoderOptions = fec.Turbo.DecoderOptions(val)
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.Turbo_set_decoderOptions), self, self.structure.decoderOptions.get());
        end
        %>  Modify the Turbo::decoderOptions property.
        %>  @param  Structure, cell, object or Name, Value list
        function setDecoderOptions(self,varargin)
            decoderOptions = self.decoderOptions;
            decoderOptions.set(varargin{:});
            self.decoderOptions = decoderOptions;
        end
    end

end