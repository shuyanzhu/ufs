add_executable(ufs main.c ufs.c ufs_struct.c)
add_library(UFS STATIC ufs.c ufs_struct.c)
