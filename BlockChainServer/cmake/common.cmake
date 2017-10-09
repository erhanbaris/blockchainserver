if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
set(BLOCKCHAIN_PLATFORM mac)
elseif(CMAKE_SYSTEM_NAME STREQUAL DragonFly)
set(BLOCKCHAIN_PLATFORM dragonflybsd)
elseif(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
set(BLOCKCHAIN_PLATFORM freebsd)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
set(BLOCKCHAIN_PLATFORM linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL NetBSD)
set(BLOCKCHAIN_PLATFORM netbsd)
elseif(CMAKE_SYSTEM_NAME STREQUAL OpenBSD)
set(BLOCKCHAIN_PLATFORM openbsd)
elseif(CMAKE_SYSTEM_NAME STREQUAL Windows)
set(BLOCKCHAIN_PLATFORM win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL SunOS)
set(BLOCKCHAIN_PLATFORM sunos)
endif()


# CMAKE_SYSTEM_PROCESSOR is always i386 on MacOS.  Since we only support
# 64 bits builds anyway, default to x64.
if(CMAKE_SYSTEM_NAME STREQUAL Darwin OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64 OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL IA64 OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL amd64 OR
   CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
  set(BLOCKCHAIN_ARCH x64)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL i386 OR
       CMAKE_SYSTEM_PROCESSOR STREQUAL i686 OR
       CMAKE_SYSTEM_PROCESSOR STREQUAL x86)
  set(BLOCKCHAIN_ARCH ia32)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL arm)
  set(BLOCKCHAIN_ARCH arm)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL ppc64)
  set(BLOCKCHAIN_ARCH ppc64)
endif()

set(${BLOCKCHAIN_PLATFORM} ON)
set(${BLOCKCHAIN_ARCH} ON)