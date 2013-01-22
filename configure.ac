AC_PREREQ([2.64])

m4_define([json_tools_major_version], [0])
m4_define([json_tools_minor_version], [1])
m4_define([json_tools_micro_version], [0])
m4_define([json_tools_version],
          [json_tools_major_version.json_tools_minor_version.json_tools_micro_version])

AC_INIT([json_tools],
        [json_tools_version],
        [https://json_tools.org],
        [json_tools],
        [http://json_tools.org])

AC_SUBST([JSON_TOOLS_VERSION_MAJOR], [json_tools_major_version])
AC_SUBST([JSON_TOOLS_VERSION_MINOR], [json_tools_minor_version])
AC_SUBST([JSON_TOOLS_VERSION_MICRO], [json_tools_micro_version])
AC_SUBST([JSON_TOOLS_VERSION], [json_tools_version])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_cxx_compile_stdcxx_11.m4])

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])

#`AM_SILENT_RULES([yes])

CXXFLAGS="-Wall -Wextra -Wno-unused-parameter -g -Wstrict-prototypes -Wmissing-prototypes -fvisibility=hidden"

AC_LANG([C++])
AC_PROG_CXX

AX_CXX_COMPILE_STDCXX_11([ext])

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT

if test "x$GCC" = "xyes"; then
	GCC_CFLAGS="-Wall -Wextra -Wno-unused-parameter -g -Wstrict-prototypes -Wmissing-prototypes -fvisibility=hidden"
fi

AC_SUBST(GCC_CFLAGS)


AC_CONFIG_FILES([Makefile
		 src/Makefile
		 src/json_tools-version.h
		 tests/Makefile])
AC_OUTPUT
