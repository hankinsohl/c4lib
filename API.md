# c4lib API

## Header files

The include directory contains several header files which collectively constitute the c4lib API.
The following sections briefly describe each header file.

### c4lib.hpp

c4lib.hpp contains the main functions used to interface with the library. Functions to read
saves and info files and to write saves, info files and translations are provided. Each
function is documented in the header using Doxygen-style comments.

### exceptions.hpp

c4lib throws a variety of exceptions specific to the library, and these exceptions are listed
in exceptions.hpp. In addition to the listed exceptions, c4lib may throw exceptions generated
by the C++ standard library.

If an exception is thrown, the library is no longer usable and the calling application should
exit.

If an exception occurs in the read_save method, c4lib generates a crash-dump file containing
the info-representation of the property tree generated up to the point where the exception
was thrown. The crash-dump is written to the debug output directory if specified or to the
current directory otherwise. The name of the crash-dump file is the same as the name of the
save with the suffix ".crash-dump.info" added. Use the crash-dump and the exception message to
diagnose the cause of failure. If you've modified BTS.schema, it's likely that the schema isn't
correct. It's also possible that there's a bug in c4lib. File an issue on Github if this is
the case.

### logger.hpp

c4lib uses Logger to log diagnostic messages. By default, the logger isn't enabled. Use
one of the Logger::start methods to enable logging.

By way of example, the following code, taken from src/main.cpp, shows how to start the logger:

```
// Process log option
if (options.contains(exeopt::log) && options[exeopt::log] == "1") {
    c4lib::Logger::start(log_filename, c4lib::Logger::Severity::info);
}
```

### node-attributes.hpp

c4lib stores a representation of the save it is working on using a Boost Property Tree. Roughly
speaking, each data member in a Beyond the Sword save is stored in a property tree node. c4lib
generates a child node for each data-member node called "\_\_Attributes__". Various attributes
of the data-member node are stored as child nodes of \_\_Attributes__.

For example, the GameVersion data member node, its \_\_Attributes__ child, and various attributes
looks like this (info-format representation):

```
GameVersion
{
    __Attributes__ *
    {
        __Name__ GameVersion
        __Type__ uint_type
        __Typename__ uint32
        __Size__ 4
        __Data__ 302
        __FormattedData__ 302
    }
}
```

node-attributes.hpp is used to help access these nodes using the Boost Property Tree API.
For example, the following code, taken from lib/ptree/translation-node-writer.cpp,
illustrates use of node-attributes.hpp to access the origin node (c4lib writes an origin
node at the start of each property tree to document the origin of the save):

```
void Translation_node_writer::print_origin_info_(const bpt::ptree& root) const
{
    // Print origin info in the following format:
    // Savegame: "data\\Brennus BC-4000.CivBeyondSwordSave"
    // Schema: "..\\..\\..\\doc\\BTS.schema"
    // Date: 12-09-2024 16:59:28 UTC
    // c4lib version: 01.00.00
    std::ostream& out{*m_out};
    
        const boost::optional<const bpt::ptree&> origin_node{root.get_child_optional(nn_origin)};
        if (!origin_node) {
            throw Ptree_error{std::format(fmt::node_not_found, nn_origin)};
        }
    
        out << text_savegame << ": " << origin_node->get<std::string>(nn_savegame) << '\n';
        out << text_schema << ": " << origin_node->get<std::string>(nn_schema) << '\n';
        out << text_date << ": " << origin_node->get<std::string>(nn_date) << '\n';
        out << text_c4lib_version << ": " << origin_node->get<std::string>(nn_c4lib_version) << '\n';
        out << '\n';
}
```

To access the property tree nodes, generate an info file representation of the property tree
you're working with using the write_info API method. Examine the info file in a text editor
alongside BTS.schema, to identify nodes of interest. Refer to the
[Boost Property Tree API](https://www.boost.org/doc/libs/1_87_0/doc/html/property_tree.html) for
documentation about using Boost ptree.

### node-type.hpp

node-type.hpp contains the Node_type enumeration. Node_type is used in the \_\_Type__ child of
the \_\_Attributes__ node for a BTS data member. See [node-attributes.hpp](#node-attributeshpp).

## Library files

The following versions of c4lib are provided:

* libc4-clang.a - Use when compiling with Clang. Supports both Debug and Release builds.
* libc4-gnu.a - Use when compiling with GCC. Supports both Debug and Release builds.
* c4.lib - Use when compiling with MSVC. Supports Release builds.
* c4d.lib - Use when compiling with MSVC. Supports Debug builds.

In theory the libraries should be interchangeable; in practice, some ABI incompatibilities
still exist between MSVC and other Windows compilers, and it's likely that differences in
the implementation of hardening makes Clang incompatible with GCC.

## Hardening

c4lib is built with various security hardening options enabled. To take full advantage
of hardening, you'll want to compile and link your executable using these same options.

Hardening options used are as follows:

For Clang and GCC:

```
-D_FORTIFY_SOURCE=3 -fstack-protector-all -mshstk -fcf-protection=full
```

For MSVC:

```
/GS /guard:cf /sdl
```

## Sample code

[c4recover](https://github.com/hankinsohl/c4recover) is a command line program built using c4lib. In
the c4recover sources, the remove_lma
function, which resides in util.cpp, uses the Boost Property Tree API to change several string
variables.

Although this sample is very small, it should at least be enough to get you started.

## Tips

* Consider using debug options in config.xml: DEBUG_WRITE_BINARIES, DEBUG_WRITE_IMPORTS.
* If your program crashes while reading a save, examine the crash dump file.
* If your program crashes while writing, use boost::property_tree::write_info
  to examine the changes you've made.
* The c4lib libraries contain full symbol information. Download the sources to step into c4lib
  code to facilitate debugging.
* Arrays and other compound variables within a BTS save are often preceded by a size field. When
  changing the number of items in a compound variable, you'll need to update the corresponding
  size field in cases where a size field is used.