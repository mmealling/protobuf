#include "RubyGenerator.h"

namespace google {
namespace protobuf  {
namespace compiler {
namespace ruby {

RubyGenerator::RubyGenerator() {}  // Constructor
RubyGenerator::~RubyGenerator() {} // Destructor

// Generate one or more ruby source files for the given proto file.
bool RubyGenerator::Generate(const FileDescriptor* file,
                              const string& parameter,
                              GeneratorContext* context,
                              string* error) const {
  file_ = file;
  context_ = context;

  filename = CreateRubyFileName(file_->name());
  ns_vector.clear();
  extended_messages.clear();
  SplitStringUsing(file_->package(), ".", &ns_vector);

  // Get a ZeroCopyOutputStream object of the data.
  scoped_ptr<io::ZeroCopyOutputStream> output(context_->Open(filename));

  // Check the data against Google's proto checking algorithm.
  GOOGLE_CHECK(output.get());

  // Get a printer.
  io::Printer printer(output.get(), '$');
  printer_ = &printer;
  indent_level = 0;

  try {
    PrintGeneratedFileComment();
    PrintGenericRequires();
    PrintImportRequires();

    PrintEnclosingNamespaceModules();

    PrintEnumsForFileDescriptor(file_, false);
    PrintNewLine();
    PrintMessagesForFileDescriptor(file_, false);
    PrintNewLine();

    PrintMessagesForFileDescriptor(file_, true);

    PrintDanglingExtendedMessages();

    PrintServices();

    PrintEnclosingNamespaceModuleEnds();
  }
  catch (int e) {
    if (failed_message.empty()) {
      *error = strings::Substitute("An unknown error occurred compiling $0", filename).c_str();
    }
    else {
      *error = failed_message.c_str();
    }
    return false;
  }
  return true;
} // end Generate()

//
///////////////////////////////////////////////// [ namespaces ] //////////////
//

void RubyGenerator::PrintEnclosingNamespaceModules() const {
  PrintNewLine();
  vector<string>::iterator iter;
  map<string,string> data;
  for (iter = ns_vector.begin(); iter < ns_vector.end(); iter++) {
    PrintModuleDeclaration(Constantize(*iter, false));
  }
}

void RubyGenerator::PrintEnclosingNamespaceModuleEnds() const {
  vector<string>::iterator iter;
  for (iter = ns_vector.begin(); iter < ns_vector.end(); iter++) {
    PrintBlockEnd();
  }
}


//
///////////////////////////////////////////////// [ messages ] ////////////////
//

// Print a comment and then iteratively PrintMessage for each message
// type defined by in this FileDescriptor scope.
//
void RubyGenerator::PrintMessagesForFileDescriptor(const FileDescriptor* descriptor, bool print_fields) const {
  if (descriptor->message_type_count() > 0) {
    if (print_fields) {
      PrintComment("Message Fields", true);
    }
    else {
      PrintComment("Message Classes", true);
      StoreExtensionFields(descriptor);
    }

    for (int i = 0; i < descriptor->message_type_count(); i++) {
      PrintMessage(descriptor->message_type(i), print_fields);
    }
  }
}

// Iterates the nested types of a message descriptor and calls PrintMessage for each.
//
void RubyGenerator::PrintMessagesForDescriptor(const Descriptor* descriptor, bool print_fields) const {
  for (int i = 0; i < descriptor->nested_type_count(); i++) {
    PrintMessage(descriptor->nested_type(i), print_fields);
  }
}

// Print out the given descriptor message as a Ruby class.
//
void RubyGenerator::PrintMessage(const Descriptor* descriptor, bool print_fields) const {
  switch (print_fields) {
    case false:

      if (DescriptorHasNestedTypes(descriptor)) {
        PrintClassDeclaration(descriptor->name(), CLASS_TYPE_MESSAGE);

        if (descriptor->enum_type_count() > 0) {
          PrintEnumsForDescriptor(descriptor, true);
        }

        if (descriptor->nested_type_count() > 0) {
          PrintMessagesForDescriptor(descriptor, false);
        }

        PrintBlockEnd();
      }
      else {
        PrintClassDeclaration(descriptor->name(), CLASS_TYPE_MESSAGE, true);
      }

      PrintNewLine();
      StoreExtensionFields(descriptor);

      break;

    case true:

      if (descriptor->field_count() > 0 || DescriptorHasExtensions(descriptor)) {
        PrintClassDeclaration(descriptor->name(), CLASS_TYPE_NONE);

        if (descriptor->nested_type_count() > 0) {
          PrintMessagesForDescriptor(descriptor, true);
        }

        // Print Fields
        if (descriptor->field_count() > 0) {
          for (int i = 0; i < descriptor->field_count(); i++) {
            PrintMessageField(descriptor->field(i));
          }
        }

        PrintExtensionRangesForDescriptor(descriptor);

        // Print Extension Fields
        if (DescriptorHasExtensions(descriptor)) {
          PrintMessageExtensionFields(descriptor->full_name());
        }

        PrintBlockEnd();
        PrintNewLine();
      }
      else if (descriptor->nested_type_count() > 0) {
        PrintClassDeclaration(descriptor->name(), CLASS_TYPE_NONE);

        if (descriptor->nested_type_count() > 0) {
          PrintMessagesForDescriptor(descriptor, true);
        }

        PrintBlockEnd();
        PrintNewLine();
      }

      break;
  }
}

void RubyGenerator::PrintExtensionRangesForDescriptor(const Descriptor* descriptor) const {
  if (descriptor->extension_range_count() > 0) {
    PrintNewLine();
    PrintComment("Extension Fields", false);

    for (int i = 0; i < descriptor->extension_range_count(); i++) {
      const Descriptor::ExtensionRange* range = descriptor->extension_range(i);
      printer_->Print("extensions $start$...$end$", "start", SimpleItoa(range->start), "end", SimpleItoa(range->end));
      ValidatePrinter("Failed printing extension ranges");
      PrintNewLine();
    }
  }
}

// Print the given FieldDescriptor to the Message DSL methods.
void RubyGenerator::PrintMessageField(const FieldDescriptor* descriptor) const {
  map<string,string> data;
  data["field_presence"] = "";
  data["field_name"] = descriptor->lowercase_name();
  data["tag_number"] = SimpleItoa(descriptor->number());
  data["data_type"] = "";
  data["default_opt"] = "";
  data["packed_opt"] = "";
  data["deprecated_opt"] = "";
  data["extension_opt"] = "";

  if (descriptor->is_required()) {
    data["field_presence"] = "required";
  }
  else if (descriptor->is_optional()) {
    data["field_presence"] = "optional";
  }
  else if (descriptor->is_repeated()) {
    data["field_presence"] = "repeated";
  }

  switch (descriptor->type()) {
    // Primitives
    case FieldDescriptor::TYPE_DOUBLE:   data["data_type"] = "::Protobuf::Field::DoubleField"; break;
    case FieldDescriptor::TYPE_FLOAT:    data["data_type"] = "::Protobuf::Field::FloatField"; break;
    case FieldDescriptor::TYPE_INT64:    data["data_type"] = "::Protobuf::Field::Int64Field"; break;
    case FieldDescriptor::TYPE_UINT64:   data["data_type"] = "::Protobuf::Field::Uint64Field"; break;
    case FieldDescriptor::TYPE_INT32:    data["data_type"] = "::Protobuf::Field::Int32Field"; break;
    case FieldDescriptor::TYPE_FIXED64:  data["data_type"] = "::Protobuf::Field::Fixed64Field"; break;
    case FieldDescriptor::TYPE_FIXED32:  data["data_type"] = "::Protobuf::Field::Fixed32Field"; break;
    case FieldDescriptor::TYPE_BOOL:     data["data_type"] = "::Protobuf::Field::BoolField"; break;
    case FieldDescriptor::TYPE_STRING:   data["data_type"] = "::Protobuf::Field::StringField"; break;
    case FieldDescriptor::TYPE_BYTES:    data["data_type"] = "::Protobuf::Field::BytesField"; break;
    case FieldDescriptor::TYPE_UINT32:   data["data_type"] = "::Protobuf::Field::Uint32Field"; break;
    case FieldDescriptor::TYPE_SFIXED32: data["data_type"] = "::Protobuf::Field::Sfixed32Field"; break;
    case FieldDescriptor::TYPE_SFIXED64: data["data_type"] = "::Protobuf::Field::Sfixed64Field"; break;
    case FieldDescriptor::TYPE_SINT32:   data["data_type"] = "::Protobuf::Field::Sint32Field"; break;
    case FieldDescriptor::TYPE_SINT64:   data["data_type"] = "::Protobuf::Field::Sint64Field"; break;

    // Enums
    case FieldDescriptor::TYPE_ENUM:
      data["data_type"] = Constantize(descriptor->enum_type()->full_name());
      break;

    // Messages
    case FieldDescriptor::TYPE_GROUP:
    case FieldDescriptor::TYPE_MESSAGE:
    default:
      data["data_type"] = Constantize(descriptor->message_type()->full_name());
      break;
  }

  if (descriptor->has_default_value()) {
    string value;
    switch(descriptor->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32:  value = SimpleItoa(descriptor->default_value_int32()); break;
      case FieldDescriptor::CPPTYPE_INT64:  value = SimpleItoa(descriptor->default_value_int64()); break;
      case FieldDescriptor::CPPTYPE_UINT32: value = SimpleItoa(descriptor->default_value_uint32()); break;
      case FieldDescriptor::CPPTYPE_UINT64: value = SimpleItoa(descriptor->default_value_uint64()); break;
      case FieldDescriptor::CPPTYPE_DOUBLE: value = SimpleDtoa(descriptor->default_value_double()); break;
      case FieldDescriptor::CPPTYPE_FLOAT:  value = SimpleFtoa(descriptor->default_value_float()); break;
      case FieldDescriptor::CPPTYPE_BOOL:   value = descriptor->default_value_bool() ? "true" : "false"; break;
      case FieldDescriptor::CPPTYPE_ENUM:   value = FullEnumNamespace(descriptor->default_value_enum()); break;
      case FieldDescriptor::CPPTYPE_STRING: value = "\"" + descriptor->default_value_string() + "\""; break;
      default: break;
    }
    data["default_opt"] = strings::Substitute(", :default => $0", value);
  }

  if (descriptor->is_packable() && descriptor->options().has_packed()) {
    string packed_bool = descriptor->options().packed() ? "true" : "false";
    data["packed_opt"] = strings::Substitute(", :packed => $0", packed_bool);
  }

  if (descriptor->options().has_deprecated()) {
    string deprecated_bool = descriptor->options().deprecated() ? "true" : "false";
    data["deprecated_opt"] = strings::Substitute(", :deprecated => $0", deprecated_bool);
  }

  if (descriptor->is_extension()) {
    data["extension_opt"] = ", :extension => true";
  }

  printer_->Print(data,
    "$field_presence$ "
    "$data_type$, "
    ":$field_name$, "
    "$tag_number$"
    "$default_opt$"
    "$packed_opt$"
    "$deprecated_opt$"
    "$extension_opt$");
  ValidatePrinter("Failed printing message field");
  PrintNewLine();
}

// Print out each extension field previously mapped to the full name of
// the descriptor message.
//
// After printign the fields, erase the fields from the map so that we know
// which fields are dangling and to print wrapped in a re-opened class block.
//
void RubyGenerator::PrintMessageExtensionFields(const string full_name) const {
  vector<const FieldDescriptor*> message_extensions = extended_messages[full_name];
  vector<const FieldDescriptor*>::iterator it;
  for (it = message_extensions.begin(); it != message_extensions.end(); it++) {
    PrintMessageField(*it);
  }
  extended_messages.erase(full_name);
}

//
///////////////////////////////////////////////// [ enums ] ///////////////////
//

void RubyGenerator::PrintEnumsForDescriptor(const Descriptor* descriptor, bool print_values) const {
  for (int i = 0; i < descriptor->enum_type_count(); i++) {
    PrintEnum(descriptor->enum_type(i));
  }
}

void RubyGenerator::PrintEnumsForFileDescriptor(const FileDescriptor* descriptor, bool print_values) const {
  if (descriptor->enum_type_count() > 0) {
    if (print_values) {
      PrintComment("Enum Values", true);
    }
    else {
      PrintComment("Enum Classes", true);
    }

    for (int i = 0; i < descriptor->enum_type_count(); i++) {
      PrintEnum(descriptor->enum_type(i));
    }
  }
}

// Print the given enum descriptor as a Ruby class.
void RubyGenerator::PrintEnum(const EnumDescriptor* descriptor) const {
  PrintClassDeclaration(descriptor->name(), CLASS_TYPE_ENUM);

  for (int i = 0; i < descriptor->value_count(); i++) {
    PrintEnumValue(descriptor->value(i));
  }

  PrintBlockEnd();
  PrintNewLine();
}

// Print the given enum value to the Enum class DSL methods.
void RubyGenerator::PrintEnumValue(const EnumValueDescriptor* descriptor) const {
  string number = ConvertIntToString(descriptor->number()).c_str();
  printer_->Print("define :$name$, $number$", "name", descriptor->name().c_str(), "number", number);
  ValidatePrinter("Failed printing enum value");
  PrintNewLine();
}

//
///////////////////////////////////////////////// [ services ] ////////////////
//

void RubyGenerator::PrintServices() const {
  if (file_->service_count() > 0) {
    PrintComment("Services", true);
    for (int i = 0; i < file_->service_count(); i++) {
      PrintService(file_->service(i));
    }
  }
}

// Print the given service as a Ruby class.
void RubyGenerator::PrintService(const ServiceDescriptor* descriptor) const {
  PrintClassDeclaration(descriptor->name(), CLASS_TYPE_SERVICE);

  for (int i = 0; i < descriptor->method_count(); i++) {
    PrintServiceMethod(descriptor->method(i));
  }

  PrintBlockEnd();
}

// Print the rpc DSL declaration to the Ruby service class.
void RubyGenerator::PrintServiceMethod(const MethodDescriptor* descriptor) const {
  map<string,string> data;
  data["method_name"] = descriptor->name();
  data["request_klass"] = Constantize(descriptor->input_type()->full_name());
  data["response_klass"] = Constantize(descriptor->output_type()->full_name());
  printer_->Print(data, "rpc :$method_name$, $request_klass$, $response_klass$");
  ValidatePrinter("Failed printing rpc method");
  PrintNewLine();
}


//
///////////////////////////////////////////////// [ general ] ////////////////
//

void RubyGenerator::PrintDanglingExtendedMessages() const {
  if (extended_messages.size() > 0) {
    PrintComment("Extended Messages", true);

    tr1::unordered_map<string, vector<const FieldDescriptor*> >::iterator it;
    for (it = extended_messages.begin(); it != extended_messages.end(); it++) {
      string extended_message = it->first;
      PrintClassDeclaration(Constantize(extended_message), CLASS_TYPE_NONE);
      PrintMessageExtensionFields(extended_message);
      PrintBlockEnd();
    }
  }
}

// Explicitly check for the key with `count` so that we don't create
// empty vectors for classes simply using bracket access.
//
bool RubyGenerator::DescriptorHasExtensions(const Descriptor* descriptor) const {
  const string full_name = descriptor->full_name();
  if (extended_messages.count(full_name) > 0) {
    return (extended_messages[full_name].size());
  }
  else {
    return 0;
  }
}

// Prints a require with the given ruby library.
void RubyGenerator::PrintRequire(string lib_name) const {
  printer_->Print("require '$lib$'\n", "lib", lib_name.c_str());
  ValidatePrinter("Failed printing require");
}

// Print a comment indicating that the file was generated.
void RubyGenerator::PrintGeneratedFileComment() const {
  PrintComment("This file is auto-generated. DO NOT EDIT!", true);
}

// Print out message requires as they pertain to the ruby library.
void RubyGenerator::PrintGenericRequires() const {
  if (file_->message_type_count() > 0) {
    PrintRequire("protobuf/message");
  }
  if (file_->service_count() > 0) {
    PrintRequire("protobuf/rpc/service");
  }
}

void RubyGenerator::PrintImportRequires() const {
  if (file_->dependency_count() > 0) {
    PrintNewLine();
    PrintComment("Imports", true);
    for (int i = 0; i < file_->dependency_count(); i++) {
      PrintRequire(CreateRubyFileName(file_->dependency(i)->name(), true));
    }
  }
}

// Print a one-line comment.
void RubyGenerator::PrintComment(string comment) const {
  PrintComment(comment, false);
}

// Print a header or one-line comment (as indicated by the as_header bool).
void RubyGenerator::PrintComment(string comment, bool as_header) const {
  string format = "# $comment$";
  if (as_header) {
    format = "##\n# $comment$\n#";
  }
  printer_->Print(format.c_str(), "comment", comment.c_str());
  ValidatePrinter("Failed printing comment");
  PrintNewLine();
}

void RubyGenerator::PrintNewLine() const {
  PrintNewLine(1);
}

void RubyGenerator::PrintNewLine(int num_newlines) const {
  for (int i = indent_level; i > 0; i--)
    printer_->Outdent();

  for (int i = 0; i < num_newlines; i++) {
    printer_->Print("\n");
    ValidatePrinter("Failed printing newline");
  }

  for (int i = 0; i < indent_level; i++)
    printer_->Indent();
}

void RubyGenerator::Indent() const {
  printer_->Indent();
  indent_level++;
}

void RubyGenerator::Outdent() const {
  if (indent_level > 0) {
    printer_->Outdent();
    indent_level--;
  }
}

void RubyGenerator::PrintClassDeclaration(string class_name, RubyClassType class_type) const {
  PrintClassDeclaration(class_name, class_type, false);
}

void RubyGenerator::PrintClassDeclaration(string class_name, RubyClassType class_type, bool empty_body) const {
  PrintBlockDeclaration(RUBY_CLASS, class_type, class_name, empty_body);
}

void RubyGenerator::PrintModuleDeclaration(string module_name) const {
  PrintModuleDeclaration(module_name, false);
}

void RubyGenerator::PrintModuleDeclaration(string module_name, bool empty_body) const {
  PrintBlockDeclaration(RUBY_MODULE, CLASS_TYPE_NONE, module_name, empty_body);
}

void RubyGenerator::PrintBlockDeclaration(RubyBlockType block_type, RubyClassType class_type, string block_name, bool empty_body) const {
  string format = "$block_type$ $block_name$";
  map<string,string> data;
  data["block_name"] = block_name;

  switch (block_type) {
    case RUBY_CLASS:  data["block_type"] = "class"; break;
    case RUBY_MODULE: data["block_type"] = "module"; break;
  }

  if (class_type != CLASS_TYPE_NONE) {
    format += " < $parent_type$";
    switch (class_type) {
      case CLASS_TYPE_MESSAGE: data["parent_type"] = "::Protobuf::Message"; break;
      case CLASS_TYPE_ENUM:    data["parent_type"] = "::Protobuf::Enum"; break;
      case CLASS_TYPE_SERVICE: data["parent_type"] = "::Protobuf::Rpc::Service"; break;
      case CLASS_TYPE_NONE: break;
    }
  }

  if (empty_body) {
    format += "; end";
  }

  printer_->Print(data, format.c_str());
  ValidatePrinter("Failed printing block declaration");
  PrintNewLine();

  if (! empty_body) {
    Indent();
  }
}


void RubyGenerator::PrintBlockEnd() const {
  Outdent();
  printer_->Print("end");
  ValidatePrinter("Failed printing block end");
  PrintNewLine();
}

// We need to store any extension fields defined in the scope of this
// descriptor message by the field's containing type.
void RubyGenerator::StoreExtensionFields(const FileDescriptor* descriptor) const {
  for (int i = 0; i < descriptor->extension_count(); i++) {
    const FieldDescriptor* extension_field = descriptor->extension(i);
    const Descriptor* containing = extension_field->containing_type();
    extended_messages[containing->full_name()].push_back(extension_field);
  }
}

// Same as above, only accept the Descriptor type instead of FileDescriptor.
void RubyGenerator::StoreExtensionFields(const Descriptor* descriptor) const {
  for (int i = 0; i < descriptor->extension_count(); i++) {
    const FieldDescriptor* extension_field = descriptor->extension(i);
    const Descriptor* containing = extension_field->containing_type();
    extended_messages[containing->full_name()].push_back(extension_field);
  }
}

void RubyGenerator::ValidatePrinter(string fail_message) const {
  if ((*printer_).failed()) {
    failed_message = fail_message;
    throw 1;
  }
}

} // namespace ruby
} // namespace compiler
} // namespace protobuf
} // namespace google
