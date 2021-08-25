AC_DEFUN([PSPIN_HANDLERS_CHECK],
    [AC_ARG_WITH(pspin-handlers, AC_HELP_STRING([--with-pspin-handlers], [PsPIN handlers directory (contains headers in ./include)]))
    pspin_handlers_found=no

    if test x"${with_pspin_handlers}" != x; then
        CPPFLAGS="$CPPFLAGS -I${with_pspin_handlers}/include -DSPIN_SLM_PATH=${with_pspin_handlers}/build/slm_files/"
    fi

    AC_CHECK_HEADER(pspin_sst.h, [pspin_handlers_found=yes], [AC_MSG_ERROR([Can't find PsPIN handler headers (pspin_sst.h)!])])

    ]
)

