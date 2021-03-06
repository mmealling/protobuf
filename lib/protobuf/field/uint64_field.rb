require 'protobuf/field/varint_field'

module Protobuf
  module Field
    class Uint64Field < VarintField
      def self.max; UINT64_MAX; end
      def self.min; 0; end
      def max; UINT64_MAX; end
      def min; 0; end
    end
  end
end
