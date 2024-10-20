set(this 
    liblinux
)

set(Src 
    startup.s
    syscall.s
    console_io.c
)

add_library(${this} ${Core} ${Src})
target_compile_options(${this} PRIVATE -fno-stack-protector)