AC_DEFUN([PSPIN_CHECK],
    [AC_ARG_WITH(pspin, AC_HELP_STRING([--with-pspin], [PsPIN repo directory]))
    pspin_found=no

    if test x"${with_pspin}" != x; then
        CPPFLAGS="$CPPFLAGS -I${with_pspin}/hw/verilator_models/include/ -I${with_pspin}/sw/runtime/include/"
    fi

    ]
)

