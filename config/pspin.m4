AC_DEFUN([PSPIN_CHECK],
    [AC_ARG_WITH(pspin, AC_HELP_STRING([--with-pspin], [PsPIN repo directory]))
    pspin_found=no

    if test x"${with_pspin}" != x; then
        CPPFLAGS="$CPPFLAGS -I ${with_pspin}/hw/verilator_model/include  -I${with_pspin}/sw/runtime/include"
        LDFLAGS="$LDFLAGS -L${with_pspin}/hw/verilator_model/lib -lpspin"
    fi

    AC_CHECK_HEADER(pspinsim.h, [pspin_found=yes], [AC_MSG_ERROR([Can't find PsPIN!])])

    ]
)

