require 'protobuf/field/float_field'

module Protobuf
  module Field
    class DoubleField < FloatField
      def wire_type
        WireType::FIXED64
      end

      def decode(bytes)
        bytes.unpack('E').first
      end

      def encode(value)
        [value].pack('E')
      end
    end
  end
end
