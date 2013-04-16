##
# This file is auto-generated. DO NOT EDIT!
#
require 'protobuf/message'

module test
  module underscore_pkg

    ##
    # Message Classes
    #
    class Foo < ::Protobuf::Message; end

    class With_Underscore < ::Protobuf::Message; end

    class simple_mail < ::Protobuf::Message; end

    class simple_Mail < ::Protobuf::Message; end

    class Simple_Mail < ::Protobuf::Message; end

    class simpleMail < ::Protobuf::Message; end

    class SimpleMail < ::Protobuf::Message; end

    class SimpleMAIL < ::Protobuf::Message; end

    class SIMPLE_MAIL < ::Protobuf::Message; end

    class SIMPLEMAIL < ::Protobuf::Message; end


    ##
    # Message Fields
    #
    class Foo
      optional ::Protobuf::Field::StringField, :bar, 1
      optional ::test::underscore_pkg::With_Underscore, :with_underscore, 2
    end

    class With_Underscore
      optional ::Protobuf::Field::StringField, :foo, 1
    end

    class simple_mail
      optional ::test::underscore_pkg::simple_Mail, :foo, 1
      optional ::test::underscore_pkg::Simple_Mail, :bar, 2
      optional ::test::underscore_pkg::simpleMail, :baz, 3
      optional ::test::underscore_pkg::SimpleMail, :qux, 4
      optional ::test::underscore_pkg::SIMPLE_MAIL, :quux, 5
      optional ::test::underscore_pkg::SIMPLEMAIL, :corge, 6
    end

  end
end
