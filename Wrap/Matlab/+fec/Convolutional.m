
%>  This class represents a convolutional encode / decoder.
%>  It offers methods encode and to decode data given a Structure.
%>
%>  The structure of the parity bits generated by a ConvolutionalCodec object is as follows
%>
%>    | parity1 | parity2 | ... | tail1 | tail2 ... |
%>
%>  where parityX is the output symbol sequence of the branch used at stage X
%>  in the trellis, tailX is are the output symbol sequence of the tail branch
%>  at stage X.
%>
%>  The structure of the extrinsic information
%>
%>    | msg | systTail |
%>
%>  where msg are the extrinsic msg L-values generated by the map decoder
%>  and systTail are the tail bit added to the message for trellis termination.
classdef Convolutional < fec.Codec
    properties (Dependent = true, Hidden)
        %>  Convolutional::EncoderOptions
        encoderOptions
        %>  Convolutional::DecoderOptions
        decoderOptions
    end
    properties (Dependent = true)
        %>  DecoderAlgorithm type used in decoder.
        algorithm;
    end
    
    methods (Static)
        function self = loadobj(s)
            self = fec.Convolutional();
            self.reload(s);
        end
    end

    methods
        %>  Convolutional constructor.
        %>  `codec = fec.Convolutional(trellis, lenght, Name, Value)` creates a Convolutional object from a Trellis and lenght. Optionally, additional properties can be specified using Name (string inside single quotes) followed by its Value. See Convolutional::EncoderOptions and Convolutional::DecoderOptions for a list of properties.
        %>
        %> `codec = fec.Convolutional(encoderOptions, decoderOptions)` creates a Convolutional object from the Convolutional::EncoderOptions and the Convolutional::DecoderOptions structures, cell array or object containing encoder and decoder properties which describes the codec.
        %>
        %>  See example.
        %>  @snippet Convolutional.m  Creating a simple Convolutional Codec
        function self = Convolutional(varargin)
            if (nargin > 0)
              self.structure = fec.Convolutional.Structure(varargin{:});
              self.mexHandle_ = fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_constructor), self.structure.getEncoderOptions, self.structure.getDecoderOptions);
            end
        end
        %> Creates a permutation of parity bits from Convolutional::PunctureOptions structure, object or NAME, Value list. The permutation can then transform (Permutation::permute() and then Permutation::dePermute()) the parity bits generated by the same Convolutional Codec.
        %> Note that the this function will not make the objet to apply the puncture pattern. You will have to use the created permutation to Permutation::permute() after the encode() method and Permutation::dePermute() before the decode() method. If you want to create a Convolutional Codec with a single puncturing permutation, see PuncturedConvolutional.
        %>
        %>  @param  punctureOptions structure, object or Name, Value list containing the permutation properties.
        %>  @return Generated permutation that will apply the specify punctureOptions
        function perms = puncturing(self, varargin)
            options = fec.Convolutional.PunctureOptions(varargin{:});
            perms = fec.Permutation(fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_puncturing), self, options.get()), self.paritySize);
        end
        %>  Access the Turbo::algorithm property.
        function val = get.algorithm(self)
            val = fec.Codec.DecoderAlgorithm(self.decoderOptions.algorithm).char;
        end
        %>  Modify the Turbo::algorithm property.
        function set.algorithm(self, val)
            self.setDecoderOptions('algorithm', val);
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
            self.structure.decoderOptions = fec.Convolutional.DecoderOptions(val);
            fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_set_decoderOptions), self, decoderOptions.get());
        end
        %function set.encoderOptions(self,val)
        %    self.structure.encoderOptions = fec.Convolutional.EncoderOptions(val);
        %    fec.bin.wrap(uint32(fec.detail.WrapFcnId.Convolutional_set_encoderOptions), self, encoderOptions.get());
        %end
        %>  Modify the Turbo::decoderOptions property.
        %>  @param  Structure, cell, object or Name, Value list
        function setDecoderOptions(self,varargin)
            decoderOptions = self.decoderOptions;
            decoderOptions.set(varargin{:});
            self.decoderOptions = decoderOptions;
        end
        %function setEncoderOptions(self,varargin)
        %    encoderOptions = self.encoderOptions;
        %    encoderOptions.set(varargin{:});
        %    self.encoderOptions = encoderOptions;
        %end
    end

end