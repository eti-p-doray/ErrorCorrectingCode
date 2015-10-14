classdef WrapFcnId < uint32
  enumeration
    Codec_destroy(0)
    Codec_save(1)
    Codec_load(2)
    Codec_check(3)
    Codec_encode(4)
    Codec_decode(5)
    Codec_soDecode(6)
    Codec_get_msgSize(7)
    Codec_get_systSize(8)
    Codec_get_stateSize(9)
    Codec_get_paritySize(10)
    Codec_get_workGroupSize(11)
    Codec_set_workGroupSize(12)
    Turbo_constructor(13)
    Turbo_get_decoderOptions(14)
    Turbo_set_decoderOptions(15)
    Turbo_set_encoderOptions(16)
    Turbo_createPermutation(17)
    Turbo_Lte3Gpp_interleaver(18)
    PuncturedTurbo_constructor(19)
    PuncturedTurbo_set_punctureOptions(20)
    Ldpc_constructor(21)
    Ldpc_get_decoderOptions(22)
    Ldpc_set_decoderOptions(23)
    Ldpc_set_encoderOptions(24)
    Ldpc_createPermutation(25)
    Ldpc_DvbS2_matrix(26)
    PuncturedLdpc_constructor(27)
    PuncturedLdpc_set_punctureOptions(28)
    Convolutional_constructor(29)
    Convolutional_get_decoderOptions(30)
    Convolutional_set_decoderOptions(31)
    Convolutional_set_encoderOptions(32)
    Convolutional_createPermutation(33)
    PuncturedConvolutional_constructor(34)
    PuncturedConvolutional_set_punctureOptions(35)
    Trellis_constructor(36)
  end
end