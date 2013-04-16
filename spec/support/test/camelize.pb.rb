##
# This file is auto-generated. DO NOT EDIT!
#
require 'protobuf/message'

module test

  ##
  # Message Classes
  #
  class simple_mail < ::Protobuf::Message; end

  class mail_list < ::Protobuf::Message; end


  ##
  # Message Fields
  #
  class simple_mail
    optional ::Protobuf::Field::StringField, :body, 1
  end

  class mail_list
    repeated ::test::simple_mail, :simple_mails, 1
  end

end
