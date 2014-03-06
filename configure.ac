#
# Copyright (C) 1999-2014 Thorsten Kukuk <kukuk@linux-nis.org>
#
dnl Process this file with autoconf to produce a configure script.
AC_INIT([ypserv],[2.32.1],[kukuk@linux-nis.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([ypserv/ypserv.c])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT(/usr)
AH_BOTTOM([#include <conf_post.h>])

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

dnl Some hacks...
test "${prefix}" = "NONE" && prefix="/usr"
if test ${prefix} = '/usr'
then
dnl If we use /usr as prefix, use /etc for ypserv.conf
	if test ${sysconfdir} = '${prefix}/etc'
	then
	   sysconfdir="/etc"
	fi
dnl If we use /usr as prefix, use /usr/share/man for manual pages
	if test ${mandir} = '${prefix}/man'
	then
	   mandir='${prefix}/share/man'
	fi
fi
dnl Change libexec to lib/yp
if test ${libexecdir} = '${exec_prefix}/libexec'
then
  libexecdir='${exec_prefix}/lib/yp'
  if test "${exec_prefix}" = "NONE"
  then
    YPBINDIR=/usr/lib/yp
  else
    YPBINDIR=${exec_prefix}/lib/yp
  fi
else
  YPBINDIR=${libexecdir}
fi
AC_SUBST(YPBINDIR)

dnl Location of the NIS maps.  default: /var/yp
AC_ARG_ENABLE([ypmapdir],
       AS_HELP_STRING([--enable-ypmapdir=path],
		[Location of NIS maps (default=/var/yp)]),
              YPMAPDIR=$enableval, YPMAPDIR=/var/yp)
AC_SUBST(YPMAPDIR)


if test ${sbindir} = '${exec_prefix}/sbin'
then
  if test "${exec_prefix}" = "NONE"
  then
    SBINDIR=/usr/sbin
  else
    SBINDIR=${exec_prefix}/sbin
  fi
else
  SBINDIR=${sbindir}
fi
AC_SUBST(SBINDIR)

dnl Checks for programs.
AC_PROG_CC
AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS

if eval "test x$GCC = xyes"; then
  for flag in \
      -pedantic \
      -W \
      -Wall \
      -Wbad-function-cast \
      -Wcast-align \
      -Wcast-qual \
      -Wmissing-declarations \
      -Wmissing-prototypes \
      -Wpointer-arith \
      -Wreturn-type \
      -Wstrict-prototypes
  do
    JAPHAR_GREP_CFLAGS($flag, [ CFLAGS="$CFLAGS $flag" ])
  done
fi

AC_ARG_ENABLE(pie,AS_HELP_STRING([--disable-pie],
			[Disable position-independent executeables (PIE)]),
	USE_PIE=$enableval, USE_PIE=yes)

AC_CACHE_CHECK(for -fpie, libc_cv_fpie, [dnl
  cat > conftest.c <<EOF
int foo;
main () { return 0;}
EOF
  if test "$USE_PIE" = "yes" &&
	AC_TRY_COMMAND([${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS -pie -fpie
                              -o conftest conftest.c 1>&AS_MESSAGE_LOG_FD])
  then
    libc_cv_fpie=yes
    PIE_CFLAGS="-fpie"
    PIE_LDFLAGS="-pie"
  else
    libc_cv_fpie=no
    PIE_CFLAGS=""
    PIE_LDFLAGS=""
  fi
  rm -f conftest*])
AC_SUBST(libc_cv_fpie)
AC_SUBST(PIE_CFLAGS)
AC_SUBST(PIE_LDFLAGS)

AM_WITH_DMALLOC
AM_WITH_EFENCE

dnl Check for big or little endian system
AC_C_BIGENDIAN
dnl Check for 64bit systems
AC_CHECK_SIZEOF(long)

dnl Search a awk program
AC_PATH_PROGS(AWK, mawk gawk nawk awk, ERROR)
if test $AWK = ERROR
then
   echo " "
   echo "ERROR: You must have at last one of the following programs:"
   echo "       mawk, gawk, nawk or awk !"
   echo " "
   exit
fi

dnl Search the bash
AC_PATH_PROGS(BASH, bash /sbin/sh, /bin/sh)

dnl Search make
AC_PATH_PROGS(MAKE, gmake make, "", $PATH:/usr/ccs/bin:/usr/local/bin)
if test x$MAKE = x
then
   echo " "
   echo "Warning: No make on this system?"
   echo " "
   exit
fi
AC_SUBST(MAKE)

dnl Check for aliases path
AC_PATH_PROG(ALIASES, aliases, /etc/mail/aliases, /etc/mail:/etc)



dnl Should we use a gethostbyname after gethostname to use FQDN ? default yes
AC_ARG_ENABLE(fqdn, AS_HELP_STRING([--disable-fqdn],
			[Always use FQDN (default=yes)]),
              USE_PARAM_FQDN=$enableval, USE_PARAM_FQDN=yes)
if test "$USE_PARAM_FQDN" = "no"; then
  USE_FQDN=0
else
  USE_FQDN=1
fi
AC_SUBST(USE_FQDN)

dnl Should we allow root to change a password with yppasswd ? default: yes
AC_ARG_ENABLE([checkroot],AS_HELP_STRING([--disable-checkroot],
	[Disable check for root password (default=yes)]),
              CHECKROOT=$enableval, CHECKROOT=yes)
AC_SUBST(CHECKROOT)

# Check for --with-dbmliborder
dbmliborder="gdbm ndbm qdbm tokyocabinet"
AC_MSG_CHECKING(for --with-dbmliborder)
AC_ARG_WITH(dbmliborder,
            AS_HELP_STRING([--with-dbmliborder=db1:db2:...], [order to check db backends for dbm. Valid value is a colon separated string with the backend names `ndbm', `gdbm', `qdbm' and `tokyocabinet'.]),
[
if test x$with_dbmliborder = xyes
then
  AC_MSG_ERROR([proper usage is --with-dbmliborder=db1:db2:...])
else
  dbmliborder=`echo $with_dbmliborder | sed 's/:/ /g'`
  for db in $dbmliborder; do
    if test x$db != xndbm && test x$db != xgdbm && test x$db != xqdbm && test x$db != xtokyocabinet
    then
      AC_MSG_ERROR([proper usage is --with-dbmliborder=db1:db2:...])
    fi
  done
fi])
AC_MSG_RESULT($with_dbmliborder)

# find which dbm we have
for db in $dbmliborder; do
  if test x$db = xqdbm
  then
    AC_CHECK_LIB(qdbm,gdbm_open,LIBDBM="$( pkg-config --libs qdbm )" ; CFLAGS="$( pkg-config --cflags qdbm ) $CFLAGS",LIBDBM="")
    if test x"" != x"${LIBDBM}"
    then
      AC_DEFINE(HAVE_LIBQDBM, 1, [Use qdbm library as database])
      libdb_parameter=yes
      break
    fi
  elif test x$db = xgdbm
  then
    AC_CHECK_LIB(gdbm,gdbm_open,LIBDBM="-lgdbm",LIBDBM="")
    if test x"" != x"${LIBDBM}"
    then
      AC_DEFINE(HAVE_LIBGDBM, 1, [Use gdbm library as database])
      libdb_parameter=yes
      break
    fi
  elif test x$db = xtokyocabinet
  then
    AC_CHECK_LIB(tokyocabinet,tcbdbopen,LIBDBM="-ltokyocabinet",LIBDBM="")
    if test x"" != x"${LIBDBM}"
    then
      AC_DEFINE(HAVE_LIBTC, 1, [Use tokyocabinet library as database])
      libdb_parameter=yes
      break
    fi
  elif test x$db = xndbm
  then
    AC_CHECK_FUNCS(dbm_open)
    if test x$ac_cv_func_dbm_open = xyes
    then
      AC_DEFINE(HAVE_NDBM, 1, [Use ndbm library as database])
      libdb_parameter=yes
      break
    fi
  fi
done
AC_SUBST(LIBDBM)

if test x$libdb_parameter != xyes
then
  echo "

 You need the GNU GDBM, QDBM, Tokyo Cabinet or the Solaris NDBM functions for this package !"
  echo ""
  echo ""
  exit
fi

AC_CHECK_LIB(xcrypt,crypt,LIBCRYPT="-lxcrypt",LIBCRYPT="")
if test -z "$LIBCRYPT" ; then
  AC_CHECK_LIB(crypt,crypt,LIBCRYPT="-lcrypt",LIBCRYPT="")
  AC_CHECK_HEADERS(crypt.h)
else
  AC_CHECK_HEADERS(xcrypt.h crypt.h)
fi
AC_SUBST(LIBCRYPT)

USE_SLP=0
AC_CHECK_LIB(slp,SLPReg,LIBSLP="-lslp",LIBSLP="")
AC_SUBST(LIBSLP)
if test -n "$LIBSLP" ; then
	AC_CHECK_HEADERS(slp.h)
        if test "$ac_cv_header_slp_h" = yes; then
	  USE_SLP=1
	fi
fi
AC_SUBST(USE_SLP)

AC_CHECK_FUNCS(getrpcport)
if test "$ac_cv_func_getrpcport" = no; then
	AC_CHECK_LIB([rpcsvc], [getrpcport],
	             [ac_cv_func_getrpcport=yes; LIBS="-lrpcsvc $LIBS"])
fi

USE_SD_NOTIFY=0
AC_SUBST(USE_SD_NOTIFY)
AC_CHECK_LIB(systemd-daemon,sd_notify,LIBSYSTEMD_DAEMON="-lsystemd-daemon",
	LIBSYSTEMD_DAEMON="")
if test -z "$LIBSYSTEMD_DAEMON" ; then
	AC_CHECK_LIB(systemd,sd_notify,LIBSYSTEMD_DAEMON="-lsystemd",
	LIBSYSTEMD_DAEMON="")
fi
if test -n "$LIBSYSTEMD_DAEMON" ; then
	AC_CHECK_HEADERS(systemd/sd-daemon.h)
	if test "$ac_cv_header_systemd_sd_daemon_h" = yes; then
          USE_SD_NOTIFY=1
        fi
fi
AC_SUBST(USE_SD_NOTIFY)
AC_SUBST(LIBSYSTEMD_DAEMON)

AC_CHECK_LIB(nsl,gethostbyname)
AC_CHECK_LIB(socket,socket)
AC_CHECK_LIB(resolv, res_gethostbyname, RESOLV="-lresolv", RESOLV="")
if test x$RESOLV != x
then
	AC_DEFINE(HAVE_RES_GETHOSTBYNAME, 1, [Define if you have the res_gethostbyname function.])
else
	AC_CHECK_LIB(resolv+, gethostbyname, RESOLV="-lresolv+", RESOLV="")
fi
AC_SUBST(RESOLV)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h paths.h sys/file.h sys/time.h syslog.h unistd.h)
AC_CHECK_HEADERS(getopt.h shadow.h rpc/clnt_soc.h rpc/svc_soc.h)
AC_CHECK_HEADERS(netinet/in.h alloca.h)
AC_CHECK_TYPE(in_addr_t, ,[AC_DEFINE(in_addr_t,int,[Define to int if you don't have in_addr_t])],[#include <netinet/in.h>])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_C___ATTRIBUTE__
TYPE_SOCKLEN_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl Checks if resultproc_t has been defined
AC_MSG_CHECKING(for resultproc_t in rpc/clnt.h)
AC_EGREP_HEADER(resultproc_t,rpc/clnt.h,\
	AC_DEFINE(HAVE_RESULTPROC_T, 1, [Define if the systems has the resultproc_t type.]) AC_MSG_RESULT(yes),\
	AC_MSG_RESULT(no))
AC_MSG_CHECKING(for resultproc_t in rpc/pmap_clnt.h)
AC_EGREP_HEADER(resultproc_t,rpc/pmap_clnt.h,\
	AC_DEFINE(HAVE_RESULTPROC_T) AC_MSG_RESULT(yes),\
	AC_MSG_RESULT(no))

dnl Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(setenv putenv)
AC_CHECK_FUNCS(fgetpwent putpwent fgetgrent getspnam)
AC_CHECK_FUNCS(inet_aton inet_pton vsyslog lckpwdf)
AC_CHECK_FUNCS(strdup strndup strerror strstr strsep stpcpy)
AC_CHECK_FUNCS(snprintf asprintf)
AC_CHECK_FUNCS(gethostname gettimeofday select socket uname getopt_long)
AC_CHECK_FUNCS(_rpc_dtablesize getdtablesize)
AC_CHECK_FUNCS(svc_getcaller svc_getrpccaller getline getdelim)
AC_CHECK_FUNCS(xdr_domainname xdr_ypxfrstat xdr_ypresp_xfr xdr_ypreq_xfr)
AC_CHECK_FUNCS(xdr_ypmap_parms xdr_ypreq_nokey xdr_ypresp_master xdr_mapname)
AC_CHECK_FUNCS(xdr_peername xdr_ypstat xdr_ypbind_binding xdr_yppushresp_xfr)
AC_CHECK_FUNCS(xdr_yppush_status)

dnl
dnl Check for xsltproc
dnl
enable_man=yes
AC_PATH_PROG([XSLTPROC], [xsltproc])
if test -z "$XSLTPROC"; then
     enable_man=no
fi
AC_PATH_PROG([XMLLINT], [xmllint],[/bin/true])
dnl check for DocBook DTD and stylesheets in the local catalog.
JH_CHECK_XML_CATALOG([-//OASIS//DTD DocBook XML V4.3//EN],
                [DocBook XML DTD V4.3], [], enable_man=no)
JH_CHECK_XML_CATALOG([http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl],
                [DocBook XSL Stylesheets], [], enable_man=no)
AM_CONDITIONAL(ENABLE_REGENERATE_MAN, test x$enable_man != xno)

AC_CONFIG_FILES([Makefile
	lib/Makefile etc/Makefile ypserv/Makefile
	ypxfr/Makefile yppush/Makefile makedbm/Makefile mknetid/Makefile
	revnetgroup/Makefile rpc.yppasswdd/Makefile rpc.ypxfrd/Makefile
	yphelper/Makefile scripts/Makefile scripts/ypxfr_1perhour
	scripts/ypxfr_1perday scripts/ypxfr_2perday scripts/pwupdate
	scripts/create_printcap scripts/match_printcap
	scripts/ypinit scripts/ypMakefile])
AC_OUTPUT

echo "
Configuration:

  Source code location:   ${srcdir}
  Compiler:               ${CC}
  Compiler flags:         ${CFLAGS}
  Preprocessor:		  ${CPP}
  Preprocessor flags:     ${CPPFLAGS}
  Libraries:		  ${LIBS} ${LIBDBM} ${LIBCRYPT} ${LIBSYSTEMD_DAEMON}
  Awk:                    ${AWK}
  Shell:		  ${BASH}
  NIS map dir:            ${YPMAPDIR}
  Install path prefix:    ${prefix}"
if test ${sysconfdir} != '${prefix}/etc'
then
  echo "  Install ypserv.conf in:  ${sysconfdir}"
else
  echo "  Install ypserv.conf in:  ${prefix}/etc"
fi
  echo "  Allow root password:     ${CHECKROOT}"
  echo "  Use FQDN as master name: ${USE_FQDN}"
  echo "  Notify systemd:	   ${USE_SD_NOTIFY}"
echo ""