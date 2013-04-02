module Protobuf
  module Rpc
    module Cacheable

      def self.included(other)
        other.class_eval do
          extend Protobuf::Rpc::ServiceFilters::ClassMethods
          include Protobuf::Rpc::ServiceFilters::InstanceMethods
        end
      end

      module ClassMethods

        def cache(method_key, options = {})
          cache_options[method_key] = CacheOption.new(self.class, method_key, options)
        end

        def cache_options
          @_cache_options ||= {}
        end

        def cache_engine
          Protobuf.rpc_cache_engine
        end

      end

      module InstanceMethods

        def cache_key(method_key)
          cache_option(method_key).try(:key)
        end

        def cache_option(method_key)
          cache_options[method_key]
        end

        def cache_options
          self.class.cache_options
        end

        def cacheable?(method_key)
          self.class.cached_methods.include?(method_key)
        end

        def cache_engine
          self.class.cache_engine
        end

        def readthrough_or_yield(method_key, request, response, &miss)
          if key = cache_key(method_key, request)
            cache_engine.read(key, &miss)
          else
            yield
          end
        end

      end

    end
  end
end

