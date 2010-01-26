
TCBUILD_TMP="/mnt/hda2/TMP"
SRC_BASE="${TCBUILD_TMP}/src"
BIN_BASE="${TCBUILD_TMP}/bin"
DOWNLOADS_DIR="${TCBUILD_DIR}/src"
CONFIGURE_OPTIONS="--prefix=/usr/local --disable-nls"
CONFIGURE_SYSTEM=""
COMPILE__SYSTEM=""
INSTALL__SYSTEM=""
DESKTOP_ICON=""
SRC_FILE=""
SRC_DIR=""

export CFLAGS="-march=i486 -mtune=i686 -Os -pipe"
export CXXFLAGS="${CFLAGS} -fno-exceptions -fno-rtti"
export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"

tcbuild_info()
{
  ext="$1"
  package="$2"
  comments="${COMMENTS}"
  if [ -n "$3" ]; then
    comments="${comments}
$3"
  fi
#  size="`wc -c "${BIN_BASE}/${PACKAGE}/${package}.${ext}"|awk '{print $1}'`"
#  if [ "${size}" -gt 1024 ]; then
#    size="`expr ${size} / 1024`K"
#  fi
  size=`du -h ${BIN_BASE}/${PACKAGE}/${package}.${ext} | cut -f 1`

  case "${ext}" in
  tcz*)
    if [ -z "`grep -v local "${BIN_BASE}/${PACKAGE}/${package}.${ext}.list`" ]; then
      comments="${comments}
This extension is PPI compatible."
    fi
  ;;
  esac
  (cat <<EOF
Title:          ${package}.${ext}
Description:    ${DESCRIPTION}
Version:        ${VERSION}
Author:         ${AUTHOR}
Original-site:  ${SITE}
Size:           ${size}
Copying-policy: ${COPYING}
Extension_by:   ${MAINTAINER}
EOF
  heading="Comments:	"
  echo "${comments}" | fold -s -w 64 | while read line;do
    echo "${heading}${line}"
    heading="		"
  done
  lines="`echo "${CHANGELOG}"|wc -l`"
  if [ "${lines}" -eq 1 ]; then
    CHANGELOG="----
${CHANGELOG}"
    lines=2
  fi
  linenr=0
  echo "${CHANGELOG}" | fold -s -w 64 | while read line;do
    linenr="`expr ${linenr} + 1`"
    if [ "${linenr}" -eq 1 ]; then
      echo "Change-log:	${line}"
    elif [ "${linenr}" -eq "${lines}" ]; then
      echo "Current:	${line}"
    else
      echo "		${line}"
    fi
  done
)
}

tcbuild_clean()
{
  echo "CLEANING"
  sudo rm -fr ${SRC_BASE}/${PACKAGE}
  sudo rm -fr ${BIN_BASE}/${PACKAGE}
  echo "CLEANING DONE"
}


tcbuild_download()
{
  echo "DOWNLOADING"
  mkdir -p "${DOWNLOADS_DIR}"
  cd "${DOWNLOADS_DIR}"
  echo wget $DOWNLOAD_PATH/$SRC_FILE
  [ -f "${SRC_FILE}" ] || wget "${DOWNLOAD_PATH}/${SRC_FILE}"
  echo "DOWNLOADING DONE"
}

tcbuild_unpack()
{
  echo "UNPACKING"
  if [ -z "$SRC_FILE" ]; then
    [ -f $DOWNLOADS_DIR/$PACKAGE-$VERSION.tar.gz ] && SRC_FILE="$PACKAGE-$VERSION.tar.gz"
    [ -f $DOWNLOADS_DIR/$PACKAGE-$VERSION.tgz ] && SRC_FILE="$PACKAGE-$VERSION.tgz"
    [ -f $DOWNLOADS_DIR/$PACKAGE-$VERSION.tar.bz2 ] && SRC_FILE="$PACKAGE-$VERSION.tar.bz2"
    [ -f $DOWNLOADS_DIR/$PACKAGE-$VERSION.tbz2 ] && SRC_FILE="$PACKAGE-$VERSION.tbz2"
    if [ -z "$SRC_FILE" ]; then
      echo Could not find out what Source file system to use.
      return 1
    fi
  fi


  mkdir -p "${SRC_BASE}/${PACKAGE}"
  cd "${SRC_BASE}/${PACKAGE}"
  case ${SRC_FILE} in
  *.tar.gz|*.tgz)
    tar zxvf "${DOWNLOADS_DIR}/${SRC_FILE}"
  ;;
  *.tar.bz2|*.tbz2)
    tar jxvf "${DOWNLOADS_DIR}/${SRC_FILE}"
  ;;
  *.zip)
    unzip "${DOWNLOADS_DIR}/${SRC_FILE}"
  ;;
  esac

  echo "UNPACKING DONE"
}

tcbuild_configure()
{
  echo "CONFIGURING"
  [ -z "$CONFIG_DIR" ] && CONFIG_DIR="${PACKAGE}-${VERSION}"
  

  if [ -z "$CONFIGURE_SYSTEM" ]; then
    cd "${SRC_BASE}/${PACKAGE}/${CONFIG_DIR}"
    [ -f configure ] && CONFIGURE_SYSTEM="configure"
    [ -f waf ] && CONFIGURE_SYSTEM="waf"
    [ -f SConstruct ] && CONFIGURE_SYSTEM="scons"
##    [ -f "????" ] && CONFIGURE_SYSTEM="cmake"
    if [ -z "$CONFIGURE_SYSTEM" ]; then
      echo Could not find out what Configure system to use.
      return 1
    fi
  fi

  case ${CONFIGURE_SYSTEM} in
     configure)
       cd "${SRC_BASE}/${PACKAGE}/${CONFIG_DIR}"
       ./configure ${CONFIGURE_OPTIONS} ;;
     waf)
       cd "${SRC_BASE}/${PACKAGE}/${CONFIG_DIR}"
       ./waf ${CONFIGURE_OPTIONS} configure;;
     none)
       echo Configure disabled ;;
     scons)
       cd "${SRC_BASE}/${PACKAGE}/${CONFIG_DIR}"
       echo Configure using scons takes part during compile phase ;;
     *)
       echo Unsupported Configure system: $CONFIGURE_SYSTEM
       return 1
       ;;
  esac

  echo "CONFIGURING DONE"
}

tcbuild_compile()
{
  echo "COMPILING"
  echo PKG_CONFIG_PATH $PKG_CONFIG_PATH
  [ -z "$SRC_DIR" ] && SRC_DIR="${PACKAGE}-${VERSION}"

  JOBS="`grep ^processor /proc/cpuinfo | wc -l`"

  if [ -z "$COMPILE_SYSTEM" ]; then
    cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
    [ -f setup.py ] && COMPILE_SYSTEM="python"
    [ -f Makefile  ] && COMPILE_SYSTEM="make"
    [ -f waf ] && COMPILE_SYSTEM="waf"
    [ -f SConstruct ] && COMPILE_SYSTEM="scons"
    if [ -z "$COMPILE_SYSTEM" ]; then
      echo Could not find out what Compile system to use.
      return 1
    fi
  fi

  case ${COMPILE_SYSTEM} in
     make)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       make ;;
     waf)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       ./waf ;;
     python)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       python setup.py build ;;
     none)
       echo Compile disabled ;;
     scons)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       scons ;;
     *)
       echo Unsupported Compile system: $COMPILE_SYSTEM
       return 1
       ;;
  esac

  echo "COMPILING DONE"
}

tcbuild_install()
{
  echo "INSTALLING"

  [ -z "$SRC_DIR" ] && SRC_DIR="${PACKAGE}-${VERSION}"

  root="${BIN_BASE}/${PACKAGE}/root"
  
  if [ -z "$INSTALL_SYSTEM" ]; then
    cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
    [ -f setup.py ] && INSTALL_SYSTEM="python"
    [ -f Makefile  ] && INSTALL_SYSTEM="make"
    [ -f waf ] && INSTALL_SYSTEM="waf"
    [ -f SConstruct ] && INSTALL_SYSTEM="scons"
    if [ -z "$INSTALL_SYSTEM" ]; then
      echo Could not find out what Install system to use.
      return 1
    fi
  fi

  case ${INSTALL_SYSTEM} in
     make)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       make install DESTDIR="$root" ;;
     waf)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       ./waf --destdir="$root" install ;;
     python)
       cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
       python setup.py install --prefix=/usr/local --root=$root ;;
     none)
       echo Install disabled ;;
#     scons)
#       scons ;;
     *)
       echo Unsupported Install system: $INSTALL_SYSTEM
       return 1
       ;;
  esac

  echo "INSTALLING DONE"
}

tcbuild_desktop()
{
  [ -z "$DESKTOP_ICON" ] && return 0
  [ -z "$SRC_DIR" ] && SRC_DIR="${PACKAGE}-${VERSION}"

  cd "${SRC_BASE}/${PACKAGE}/${SRC_DIR}"
  root="${BIN_BASE}/${PACKAGE}/root"

  mkdir -p "${root}/usr/local/tce.icons"
  mkdir -p "${root}/usr/local/tce.menu"
  mkdir -p "${root}/usr/local/tce.installed"
  cp -f $DESKTOP_ICON "${root}/usr/local/tce.icons/${PACKAGE}.png"
  
  cat <<EOF >"${root}/usr/local/tce.icons/${PACKAGE}"
i: /usr/local/tce.icons/${PACKAGE}.png
t: ${PACKAGE}
c: /usr/local/bin/$PACKAGE
EOF

  cat <<EOF >"${root}/usr/local/tce.menu/${PACKAGE}"
<JWM>
<Program label="${PACKAGE}">/usr/local/bin/$PACKAGE</Program>
</JWM>
EOF
}
 
tcbuild_postinstall()
{
  echo "POSTINSTALLING"

  cd "${BIN_BASE}/${PACKAGE}/root"
  find . -name man  -a -type d | xargs rm -rf
  find . -name doc  -a -type d | xargs rm -rf
  find . -name info -a -type d | xargs rm -rf

  find . | xargs file | grep "executable" | grep ELF | grep "not stripped" | cut -f 1 -d : | xargs strip --strip-unneeded 2> /dev/null
  find . | xargs file | grep "shared object" | grep ELF | grep "not stripped" | cut -f 1 -d : | xargs strip -g 2> /dev/null

  echo "POSTINSTALLING DONE"
}

tcbuild_ls()
{
  cd "${BIN_BASE}/${PACKAGE}"
  files=""
  for dir in *; do
    if [ -d "${dir}" ]; then
      echo -e "\n${dir}:"
      ( cd ${dir} ; find * ! -type d | xargs ls -l )
    else
      files="${files} ${dir}"
    fi
  done
  if [ -n "${files}" ]; then
    echo -e "\nfiles:"
  fi
  ls -l ${files}
}

tcbuild_packages()
{
  tcbuild_package
}

tcbuild_package()
{
  ext="tcz"
  package="$1"
  root="$2"
  comment="$3"
  [ -n "${root}" ]    || root='root'
  [ -n "${package}" ] || package="${PACKAGE}"
  cd "${BIN_BASE}/${PACKAGE}/${root}"
  files="${BIN_BASE}/${PACKAGE}/${package}.${ext}.list"
  find ! -type d > ${files}
  rm -f "${BIN_BASE}/${PACKAGE}/${package}.${ext}"
  mksquashfs . "${BIN_BASE}/${PACKAGE}/${package}.${ext}" -noappend
  md5sum "${BIN_BASE}/${PACKAGE}/${package}.${ext}" |
    sed 's@/.*/@@' > "${BIN_BASE}/${PACKAGE}/${package}.${ext}.md5.txt"
  tcbuild_info "${ext}" "${package}" "${comment}" | 
    tee "${BIN_BASE}/${PACKAGE}/${package}.${ext}.info"
  rm -fr ${BIN_BASE}/${PACKAGE}/${package}.${ext}.dep
  for d in $DEPS; do
      echo $d | sed "s/?/z/" >> ${BIN_BASE}/${PACKAGE}/${package}.${ext}.dep
  done
  ${TCBUILD_DIR}/bin/lsdeps ${BIN_BASE}/${PACKAGE}/${package}.${ext} >> ${BIN_BASE}/${PACKAGE}/${package}.${ext}.dep
  cat ${BIN_BASE}/${PACKAGE}/${package}.${ext}.dep | sort -u > ${BIN_BASE}/${PACKAGE}/${package}.${ext}.dep
}


tcbuild_initramfs()
{
  ext="cpio.gz"
  package="$1"
  root="$2"
  comment="$3"
  [ -n "${root}" ]    || root='root'
  [ -n "${package}" ] || package="${PACKAGE}"
  cd "${BIN_BASE}/${PACKAGE}/${root}"
  find | sudo cpio -o -H newc | gzip -2 > ${BIN_BASE}/${PACKAGE}/${package}.${ext}
}


tcbuild_export()
{
  REPODIR=/mnt/hda2/REPO
  [ -d ${REPODIR}/tcz ]  || mkdir -p  $f ${REPODIR}/tcz 
  [ -d ${REPODIR}/export ] || mkdir -p $f ${REPODIR}/export	
  cd ${BIN_BASE}/${PACKAGE}
  rm -f ${REPODIR}/export/${PACKAGE}.tar*
  tar cf ${REPODIR}/export/${PACKAGE}.tar ${PACKAGE}.tcz*
  gzip ${REPODIR}/export/${PACKAGE}.tar
  for f in *.tcz* ; do
    echo "Exporting $f"
    cp $f ${REPODIR}/tcz
  done
  
  [ -f tcz.${PACKAGE}.dep ] && cp tcz.${PACKAGE}.dep ${REPODIR}/tcz/${PACKAGE}.dep

  cd ${REPODIR}/tcz
  sudo rm -f info.lst
  for i in `ls -1 *.info|sort -f`; do
   sudo  basename "$i" .info >> info.lst
  done
}

