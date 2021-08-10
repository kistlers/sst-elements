AC_DEFUN([SST_PSPIN_CHECK],
    [AC_ARG_WITH(pspin-handlers, AC_HELP_STRING([--with-pspin-handlers], [PsPIN handlers directory (contains headers in ./include)]))
    pspin_handlers_found=no

    if test x"${with_pspin_handlers}" != x; then
        CPPFLAGS="$CPPFLAGS -I ${with_pspin_handlers}/include"
    fi

    AC_CHECK_HEADER(pspin_sst.h, [pspin_handlers_found=yes], [AC_MSG_ERROR([Can't find PsPIN handler headers (pspin_sst.h)!])])

    ]
)

