tcbuild_install()
{
  cd ${SRC_BASE}/${PACKAGE}/${SRC_DIR}
  make INSTALL_MOD_PATH="${BIN_BASE}/${PACKAGE}/root" modules_install firmware_install
  cp arch/x86/boot/bzImage ${BIN_BASE}/${PACKAGE}/$PACKAGE-$VERSION
  cd ${BIN_BASE}/${PACKAGE}/root
  for F in $(find  -name *.ko); do
    gzip -9 $F
  done
}

tcbuild_postinstall()
{
  set -e
  root="${BIN_BASE}/${PACKAGE}/root"
  kname=$(ls $root/lib/modules)

  rootfs="${BIN_BASE}/${PACKAGE}/rootfs"
  rm -rf "${rootfs}"
  mkdir -p "${rootfs}"

  for F in $(cat $MODULES_FILE); do
     filep="lib/modules/$kname/kernel/$F"
     source="$root/$filep"
     target="$rootfs/$filep"
     if [ -f $source ]; then
        [ -d ${target%/*} ] || mkdir -p ${target%/*}
        mv $source $target
     else
       echo "No file $source"
     fi
  done

  ln -s  /usr/local/lib/modules/$kname/kernel $rootfs/lib/modules/$kname/kernel.tclocal
  depmod -b $rootfs $kname


  sound="${BIN_BASE}/${PACKAGE}/sound"
  rm -rf "${sound}"
  mkdir -p "${sound}"
  filep="lib/modules/$kname/kernel/sound"
  source="$root/$filep"
  target="$sound/usr/local/$filep"
  if [ -d $source ]; then
     mkdir -p $target
     mv $source/* $target
  fi

  rest="${BIN_BASE}/${PACKAGE}/rest"
  rm -rf "${rest}"
  mkdir -p "${rest}"
  filep="lib/modules/$kname/kernel"
  source="$root/$filep"
  target="$rest/usr/local/$filep"
  if [ -d $source ]; then
     mkdir -p $target
     mv $source/* $target
  fi

  firmware="${BIN_BASE}/${PACKAGE}/firmware"
  rm -rf ${firmware}
  mkdir -p ${firmware}/lib
  mv ${root}/lib/firmware ${firmware}/lib
#  This firmware is now available in the iwlwifi extension package
#  cp ${TCBUILD_DIR}/import/ipw2200-*.fw ${firmware}/lib/firmware


}

tcbuild_packages()
{

  tcbuild_package  "sound-${PACKAGE}-${VERSION}" "sound" \
  "Sound modules."
  
  tcbuild_package  "rest-${PACKAGE}-${VERSION}" "rest" \
  "Rest of the built modules."
  
  tcbuild_package  "firmware-${PACKAGE}-${VERSION}" "firmware" \
  "Firmware that comes with the kernel."

  rootfs="${BIN_BASE}/${PACKAGE}/rootfs"
  mkdir -p ${rootfs}/opt/tce/
  cp ${BIN_BASE}/${PACKAGE}/sound-${PACKAGE}-${VERSION}.tcz ${rootfs}/opt/tce/
  cp ${BIN_BASE}/${PACKAGE}/rest-${PACKAGE}-${VERSION}.tcz ${rootfs}/opt/tce/
  cp ${BIN_BASE}/${PACKAGE}/firmware-${PACKAGE}-${VERSION}.tcz ${rootfs}/opt/tce/

  tcbuild_initramfs  "kmod-${PACKAGE}-${VERSION}" "rootfs" \
  "Kernel modules."


  root="${BIN_BASE}/${PACKAGE}/root"
  kname=$(ls $root/lib/modules)
  boot="${BIN_BASE}/${PACKAGE}/boot"
  rm -rf ${boot}
  mkdir -p ${boot}/boot/$kname
  cp ${BIN_BASE}/${PACKAGE}/${PACKAGE}-${VERSION} ${boot}/boot/$kname/bzImage
  cp ${BIN_BASE}/${PACKAGE}/kmod-${PACKAGE}-${VERSION}.cpio.gz ${boot}/boot/$kname/kmod.gz
  

  tar cf ${BIN_BASE}/${PACKAGE}/kernel-${PACKAGE}-${VERSION}.tar -C $boot .


}

tcbuild_export()
{
  cp ${BIN_BASE}/${PACKAGE}/config-${PACKAGE}-${VERSION} ${TCBUILD_DIR}/export
  cp ${BIN_BASE}/${PACKAGE}/kernel-${PACKAGE}-${VERSION}.tar ${TCBUILD_DIR}/export
}



 
