#ifndef GOOGLE_PROTOBUF_COMPILER_RUBY_GENERATOR_H
#define GOOGLE_PROTOBUF_COMPILER_RUBY_GENERATOR_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <tr1/unordered_map>

#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/substitute.h>
#include <google/protobuf/stubs/strutil.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace ruby {

class LIBPROTOC_EXPORT RubyGenerator : public CodeGenerator {
	public:
		RubyGenerator();
		~RubyGenerator();

    typedef enum {
      RUBY_CLASS,
      RUBY_MODULE
    } RubyBlockType;

    typedef enum {
      CLASS_TYPE_MESSAGE,
      CLASS_TYPE_ENUM,
      CLASS_TYPE_SERVICE,
      CLASS_TYPE_NONE
    } RubyClassType;

		// implemented Generate method from parent
		bool Generate(const FileDescriptor* file,
			const string& parameter,
			GeneratorContext* context,
			string* error) const;

	private:
		mutable GeneratorContext* context_;
		mutable io::Printer* printer_;
		mutable const FileDescriptor* file_;
		mutable string filename;
		mutable vector<string> ns_vector;
    mutable tr1::unordered_map<string, vector<const FieldDescriptor*> > extended_messages;
    mutable int indent_level;
    mutable string failed_message;

		GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RubyGenerator);

    bool DescriptorHasExtensions(const Descriptor* descriptor) const;
		void PrintDanglingExtendedMessages() const;
		void PrintExtensionRangesForDescriptor(const Descriptor* descriptor) const;

		void PrintEnclosingNamespaceModules() const;
		void PrintEnclosingNamespaceModuleEnds() const;

		void PrintMessagesForDescriptor(const Descriptor* descriptor, bool print_fields) const;
		void PrintMessagesForFileDescriptor(const FileDescriptor* descriptor, bool print_fields) const;
		void PrintMessage(const Descriptor* descriptor, bool print_fields) const;
		void PrintMessageField(const FieldDescriptor* descriptor) const;
		void PrintMessageExtensionFields(const string full_name) const;

		void PrintEnumsForDescriptor(const Descriptor* descriptor, bool print_values) const;
		void PrintEnumsForFileDescriptor(const FileDescriptor* descriptor, bool print_values) const;
		void PrintEnum(const EnumDescriptor* descriptor) const;
		void PrintEnumValue(const EnumValueDescriptor* descriptor) const;

		void PrintServices() const;
		void PrintService(const ServiceDescriptor* descriptor) const;
		void PrintServiceMethod(const MethodDescriptor* descriptor) const;

		void PrintGeneratedFileComment() const;
		void PrintGenericRequires() const;
		void PrintImportRequires() const;
		void PrintComment(string comment) const;
		void PrintComment(string comment, bool as_header) const;
		void PrintRequire(string lib_name) const;
		void PrintNewLine() const;
		void PrintNewLine(int num_newlines) const;
		void Indent() const;
		void Outdent() const;

    void PrintClassDeclaration(string class_name, RubyClassType class_type) const;
    void PrintClassDeclaration(string class_name, RubyClassType class_type, bool empty_body) const;
    void PrintModuleDeclaration(string module_name) const;
    void PrintModuleDeclaration(string module_name, bool empty_body) const;
    void PrintBlockDeclaration(RubyBlockType block_type, RubyClassType class_type, string block_name, bool empty_body) const;
    void PrintBlockEnd() const;

    void StoreExtensionFields(const FileDescriptor* descriptor) const;
    void StoreExtensionFields(const Descriptor* descriptor) const;

    void ValidatePrinter(string fail_message) const;

		// Take the proto file name, strip ".proto"
		// from the end and add ".pb.rb"
		static string CreateRubyFileName(const string proto_filename) {
			return CreateRubyFileName(proto_filename, false);
		}

		static string CreateRubyFileName(const string proto_filename, bool for_require) {
			string replacement = for_require ? ".pb" : ".pb.rb";
			return StringReplace(proto_filename, ".proto", replacement, false);
		}

		static string ConvertIntToString(int number) {
			stringstream stream;
			stream << number;
			return stream.str();
		}

		static string ConvertDoubleToString(double number) {
			stringstream stream;
			stream << number;
			return stream.str();
		}

		static string ConvertFloatToString(float number) {
			stringstream stream;
			stream << number;
			return stream.str();
		}

		static string Constantize(string full_path) {
			return Constantize(full_path, true);
		}

		static string Constantize(string full_path, bool is_top_level) {
			stringstream constantized;

			if (is_top_level) {
				constantized << "::";
			}

			string::iterator i;
			bool segment_end = true;
			for (i = full_path.begin(); i < full_path.end(); i++) {
				char c = *i;
				if (c == 46) { // period char
					constantized << ':' << ':';
					segment_end = true;
					continue;
				}
				else if (c == 95) { // underscore char
					segment_end = true;
				}
				constantized << c;
			}

			return constantized.str();
		}

    static string FullEnumNamespace(const EnumValueDescriptor* descriptor) {
      string parent_enum_type = Constantize(descriptor->type()->full_name());
      string enum_name = descriptor->name();

      return strings::Substitute("$0::$1", parent_enum_type, enum_name);
    }

    static bool DescriptorHasNestedTypes(const Descriptor* descriptor) {
      return (descriptor->enum_type_count() > 0 || descriptor->nested_type_count() > 0);
    }

}; // class RubyGenerator

} // namespace ruby
} // namespace compiler
} // namespace protobuf
} // namespace google

#ifdef __cplusplus
extern "C" {
#endif

int _rprotoc_extern(int argc, char* argv[]) {
  google::protobuf::compiler::CommandLineInterface cli;

  google::protobuf::compiler::ruby::RubyGenerator ruby_generator;
  cli.RegisterGenerator("--ruby_out", &ruby_generator,
    "Generate Ruby-compatible protobuf message and service classes.");

  return cli.Run(argc, argv);
}

/*

Use for testing:
*/

int main(int argc, char* argv[]) {
  return _rprotoc_extern(argc, argv);
}

/*
*/

#ifdef __cplusplus
}
#endif

#endif // GOOGLE_PROTOBUF_COMPILER_RUBY_GENERATOR_H
