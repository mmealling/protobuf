require 'set'

module Protobuf
  module Rpc
    class CacheOption


      attr_reader :fields, :ttl, :required, :if_callable, :unless_callable

      # Valid options
      #   :on => []
      #     An array of fields to use as part of the cache key of a request.
      #     Fields must be defined by the request type of the owning service.
      #   :ttl => 60
      #     A time-to-live policy for any response that will be cached under
      #     this option.
      #   :require => []
      #     An array of request field names that must contain a value
      #     in order for a request to be cacheable. Default is none.
      #   :if => lambda { |request| true }
      #     A callable that returns a boolean. True indicates that the request
      #     can be cached. False indicates to skip caching.
      #   :unless => lambda { |request| false }
      #     A callable that returns a boolean. False indicates that the request
      #     can be cached. True indicates to skip caching.
      def initialize(service, method_key, options = {})
        @service = service
        @method_key = method_key
        @options = options
        @fields = OrderedSet.new
        @require = Set.new
        @ttl = 60
        @if_callable = @unless_callable = nil

        extract_options!
        validate_fields!
      end

      # Indicate if the given request is considered cacheable based
      # upon the option configuration. Ensures required fields are present
      # and callables return appropriately.
      #
      def cacheable?(request)
        required_fields_present?(request) \
          && if_callable_succeeded?(request) \
          && unless_callable_failed?(request)
      end

      # Return a cache key for the given request.
      #
      # Example
      # Given the following cache definition
      #
      #   class UserService < Service
      #     cache :find, :on => [ :id, :name ]
      #     def find
      #       #...
      #     end
      #   end
      #
      # For a request like:
      #
      #   { :id => 123, :name => 'jeff' }
      #
      # The resulting request_key would be:
      #
      #   rpc.user_service.find.id:123.name:jeff
      #
      def request_key(request)
        data = {}
        @fields.each do |field|
          data[field] = request.__send__(field) if request.respond_to?(field)
        end

        return key_prefix + data.map { |k, v| "#{k}:#{v}" }.join('.')
      end

      private

      def extract_options!
        @fields          += @options.delete(:on) if @options.key?(:on)
        @required        += @options.delete(:require) if @options.key?(:require)
        @ttl             = @options.delete(:ttl) if @options.key?(:ttl)
        @if_callable     = @options.delete(:if) if @options[:if].respond_to?(:callable)
        @unless_callable = @options.delete(:unless) if @options[:unless].respond_to?(:callable)
      end

      # Call the if_callable if it is set. If it is not set
      # return a default true for default state.
      #
      def if_callable_succeeded?(request)
        if @if_callable
          !! @if_callable.call(request)
        else
          true
        end
      end

      # Return the default key prefix.
      #
      def key_prefix
        @key_prefix ||= [ "rpc",
                          @service.name,
                          @method_key ].join('.')
      end

      # Check if the given request has all of the fields
      # present and set.
      #
      def required_fields_present?(request)
        @require.all? do |field|
          request.respond_to_has_and_present?(field)
        end
      end

      # Call the unless_callable if it is set and negate it's return value.
      # If it is not set return a true for default state.
      #
      def unless_callable_failed?(request)
        if @unless_callable
          ! @unless_callable.call(request)
        else
          true
        end
      end

    end
  end
end

