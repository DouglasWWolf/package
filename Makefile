#-----------------------------------------------------------------------------
# These are the variables that are specific to your program
#-----------------------------------------------------------------------------
EXE          = package
#-----------------------------------------------------------------------------


#-----------------------------------------------------------------------------
# Find out what kind of hardware we're compiling on
#-----------------------------------------------------------------------------
PLATFORM := $(shell uname)

#-----------------------------------------------------------------------------
# For x86, declare whether to emit 32-bit or 64-bit code
#-----------------------------------------------------------------------------
X86_TYPE = 64

#-----------------------------------------------------------------------------
# Declare where the object files get created
#-----------------------------------------------------------------------------
ARM_OBJ_DIR = obj_arm
X86_OBJ_DIR = obj_x86

#-----------------------------------------------------------------------------
# Tell 'make' what suffixes will appear in make rules
#-----------------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .cpp

#-----------------------------------------------------------------------------
# Declare the compile-time flags that are common between all platforms
#-----------------------------------------------------------------------------
CXXFLAGS =	\
-O2 -g -Wall \
-Wno-write-strings \
-Wno-sign-compare \
-Wno-unused-result \
-Wno-strict-aliasing \
-std=c++11 \
-fpermissive \
-fcommon \
-I$(CPPUTILS_DIR) \
-DLINUX 

#-----------------------------------------------------------------------------
# Special compile time flags for ARM targets
#-----------------------------------------------------------------------------
ARMFLAGS = -march=armv7-a -mfpu=neon -mfloat-abi=hard

#-----------------------------------------------------------------------------
# If there was no goal on the command line, the default goal depends
# on what platform we're running on
#-----------------------------------------------------------------------------
ifeq ($(.DEFAULT_GOAL),)
    ifeq ($(PLATFORM), Linux)
        .DEFAULT_GOAL := all
    endif

    ifeq ($(PLATFORM), windows32)
        .DEFAULT_GOAL := all 
    endif
endif


#-----------------------------------------------------------------------------
# Define the name of the compiler and what "build all" means for our platform
#-----------------------------------------------------------------------------
ifeq ($(PLATFORM), Linux)
    ALL    = clean x86 arm
    ARMCXX = arm-linux-gnueabihf-g++-7
endif

ifeq ($(PLATFORM), windows32)
    ALL    = clean x86 arm
    ARMCXX = arm-none-linux-gnueabi-g++
endif


#-----------------------------------------------------------------------------
# We're going to compile every .cpp file in this folder
#-----------------------------------------------------------------------------
SRC_FILES := $(shell ls *.cpp)

#-----------------------------------------------------------------------------
# Create the base-names of the object files
#-----------------------------------------------------------------------------
OBJ_FILES = $(SRC_FILES:.cpp=.o)

#-----------------------------------------------------------------------------
# We are going to keep x86 and ARM object files in separate sub-directories
#-----------------------------------------------------------------------------
X86_OBJS = $(addprefix $(X86_OBJ_DIR)/,$(OBJ_FILES))
ARM_OBJS = $(addprefix $(ARM_OBJ_DIR)/,$(OBJ_FILES))


#-----------------------------------------------------------------------------
# This rules tells how to compile an X86 .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR)/%.o : %.cpp
	$(CXX) -m$(X86_TYPE) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# This rules tells how to compile an ARM .o object file from a .cpp source
#-----------------------------------------------------------------------------
$(ARM_OBJ_DIR)/%.o : %.cpp
	$(ARMCXX) $(CPPFLAGS) $(CXXFLAGS) $(ARMFLAGS) -c $< -o $@

#-----------------------------------------------------------------------------
# This rule builds the x86 executable from the object files
#-----------------------------------------------------------------------------
$(EXE).x86 : $(X86_OBJS)
	$(CXX) -m$(X86_TYPE) -pthread -o $@ $(X86_OBJS)
	strip $(EXE).x86

#-----------------------------------------------------------------------------
# This rule builds the ARM executable from the object files
#-----------------------------------------------------------------------------
$(EXE).arm : $(ARM_OBJS)
	$(ARMCXX)  -pthread $(ARMFLAGS) -o $@ $(ARM_OBJS) 
	arm-linux-gnueabihf-strip $(EXE).arm

 .PHONY : clean x86 arm

#-----------------------------------------------------------------------------
# This target builds all executables supported by this platform
#-----------------------------------------------------------------------------
all:	$(ALL)

#-----------------------------------------------------------------------------
# This target builds just the ARM executable
#-----------------------------------------------------------------------------
arm:	mkdirs $(EXE).arm  

#-----------------------------------------------------------------------------
# This target builds just the x86 executable
#-----------------------------------------------------------------------------
x86:	mkdirs $(EXE).x86

#-----------------------------------------------------------------------------
# This target configures the object file directories
#-----------------------------------------------------------------------------
$(X86_OBJ_DIR):
	@mkdir $(X86_OBJ_DIR)
$(ARM_OBJ_DIR):
	@mkdir $(ARM_OBJ_DIR)
	

#-----------------------------------------------------------------------------
# This target configured the object file directories
#-----------------------------------------------------------------------------
mkdirs:	$(X86_OBJ_DIR) $(ARM_OBJ_DIR)
	@chmod 777 $(X86_OBJ_DIR)
	@chmod 777 $(ARM_OBJ_DIR)
	@touch     $(X86_OBJ_DIR)/CACHEDIR.TAG
	@touch     $(ARM_OBJ_DIR)/CACHEDIR.TAG

#-----------------------------------------------------------------------------
# This target removes all files that are created at build time
#-----------------------------------------------------------------------------
clean:
	rm -rf Makefile.bak $(EXE).x86 $(EXE).arm
	rm -rf $(X86_OBJ_DIR) $(ARM_OBJ_DIR)


    
#-----------------------------------------------------------------------------
# This target appends/updates the dependencies list at the end of this file
#-----------------------------------------------------------------------------
depend:
	makedepend -p$(X86_OBJ_DIR)/ -Y *.cpp 2>/dev/null


# DO NOT DELETE

obj_x86/chcp.o: chcp.h globals.h memmap.h fpga_fifo.h heralder.h
obj_x86/chcp.o: chcp_structs.h
obj_x86/fpga_fifo.o: fpga_fifo.h memmap.h socsubsystem.h altera_peripherals.h
obj_x86/globals.o: globals.h memmap.h fpga_fifo.h heralder.h chcp_structs.h
obj_x86/globals.o: chcp.h socsubsystem.h altera_peripherals.h common.h
obj_x86/heralder.o: globals.h memmap.h fpga_fifo.h heralder.h chcp_structs.h
obj_x86/heralder.o: chcp.h common.h
obj_x86/main.o: globals.h memmap.h fpga_fifo.h heralder.h chcp_structs.h
obj_x86/main.o: chcp.h history.h common.h
obj_x86/memmap.o: memmap.h
