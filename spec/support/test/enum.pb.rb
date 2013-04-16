##
# This file is auto-generated. DO NOT EDIT!
#
require 'protobuf/message'

##
# Imports
#
require 'test/resource.pb'

module test
  ##
  # Enum Classes
  #
  class EnumTestType < ::Protobuf::Enum
    define :ONE, 1
    define :TWO, 2
  end


  ##
  # Message Classes
  #
  class EnumTestMessage < ::Protobuf::Message; end


  ##
  # Message Fields
  #
  class EnumTestMessage
    optional ::test::EnumTestType, :non_default_enum, 1
    optional ::test::EnumTestType, :default_enum, 2, :default => ::test::EnumTestType::ONE
    repeated ::test::EnumTestType, :repeated_enums, 3
  end

  ##
  # Extended Messages
  #
end
