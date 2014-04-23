################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../common.o \
../content.o \
../message.o \
../router.o 

CPP_SRCS += \
../common.cpp \
../content.cpp \
../host.cpp \
../message.cpp \
../prt.cpp \
../router.cpp \
../routercontroller.cpp \
../rt.cpp \
../util.cpp 

OBJS += \
./common.o \
./content.o \
./host.o \
./message.o \
./prt.o \
./router.o \
./routercontroller.o \
./rt.o \
./util.o 

CPP_DEPS += \
./common.d \
./content.d \
./host.d \
./message.d \
./prt.d \
./router.d \
./routercontroller.d \
./rt.d \
./util.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


