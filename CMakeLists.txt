cmake_minimum_required (VERSION 3.0)
project (breaknes CXX)

add_definitions (-D_LINUX)

# Main application

set(CMAKE_BUILD_TYPE Release)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

add_executable (breaknes 
	Breaknes/BreaknesSDL/main.cpp 
	Breaknes/BreaknesSDL/VideoProcessing.cpp
	Breaknes/BreaknesSDL/SoundProcessing.cpp

	# BreaksCore

	Common/BaseLogicLib/BaseLogic.cpp

	Common/BaseBoardLib/Fake6502.cpp
	Common/BaseBoardLib/LS32.cpp
	Common/BaseBoardLib/LS139.cpp
	Common/BaseBoardLib/LS161.cpp
	Common/BaseBoardLib/LS368.cpp
	Common/BaseBoardLib/LS373.cpp
	Common/BaseBoardLib/RegDumpProcessor.cpp
	Common/BaseBoardLib/SRAM.cpp

	#Common/JsonLib/Json.cpp

	Chips/APUSim/apu.cpp
	Chips/APUSim/clkgen.cpp
	Chips/APUSim/common.cpp
	Chips/APUSim/core.cpp
	Chips/APUSim/dac.cpp
	Chips/APUSim/debug.cpp
	Chips/APUSim/dma.cpp
	Chips/APUSim/dpcm.cpp
	Chips/APUSim/env.cpp
	Chips/APUSim/length.cpp
	Chips/APUSim/noise.cpp
	Chips/APUSim/pads.cpp
	Chips/APUSim/regs.cpp
	Chips/APUSim/square.cpp
	Chips/APUSim/triangle.cpp

	Chips/PPUSim/bgcol.cpp
	Chips/PPUSim/cram.cpp
	Chips/PPUSim/dataread.cpp
	Chips/PPUSim/debug.cpp
	Chips/PPUSim/fifo.cpp
	Chips/PPUSim/fsm.cpp
	Chips/PPUSim/hv.cpp
	Chips/PPUSim/hv_decoder.cpp
	Chips/PPUSim/mux.cpp
	Chips/PPUSim/oam.cpp
	Chips/PPUSim/obj_eval.cpp
	Chips/PPUSim/pamux.cpp
	Chips/PPUSim/par.cpp
	Chips/PPUSim/pclk.cpp
	Chips/PPUSim/ppu.cpp
	Chips/PPUSim/regs.cpp
	Chips/PPUSim/scroll_regs.cpp
	Chips/PPUSim/tilecnt.cpp
	Chips/PPUSim/video_out.cpp
	Chips/PPUSim/vram_ctrl.cpp

	Chips/M6502Core/address_bus.cpp
	Chips/M6502Core/alu.cpp
	Chips/M6502Core/alu_control.cpp
	Chips/M6502Core/branch_logic.cpp
	Chips/M6502Core/bus_control.cpp
	Chips/M6502Core/core.cpp
	Chips/M6502Core/data_bus.cpp
	Chips/M6502Core/debug.cpp
	Chips/M6502Core/decoder.cpp
	Chips/M6502Core/dispatch.cpp
	Chips/M6502Core/extra_counter.cpp
	Chips/M6502Core/flags.cpp
	Chips/M6502Core/flags_control.cpp
	Chips/M6502Core/interrupts.cpp
	Chips/M6502Core/ir.cpp
	Chips/M6502Core/pc.cpp
	Chips/M6502Core/pc_control.cpp
	Chips/M6502Core/predecode.cpp
	Chips/M6502Core/random_logic.cpp
	Chips/M6502Core/regs.cpp
	Chips/M6502Core/regs_control.cpp

	Mappers/AbstractCartridge.cpp
	Mappers/CartridgeFactory.cpp
	Mappers/AOROM.cpp
	Mappers/NROM.cpp
	Mappers/UNROM.cpp
	Mappers/MMC1.cpp
	Mappers/MMC1_Based.cpp

	IO/IO.cpp
	IO/DendyController.cpp
	IO/FamiController.cpp
	IO/NESController.cpp
	IO/CD4021.cpp
	IO/UM6582.cpp

	Breaknes/BreaksCore/AbstractBoard.cpp
	Breaknes/BreaksCore/APUPlayerBoard.cpp
	Breaknes/BreaksCore/APUPlayerBoardDebug.cpp
	Breaknes/BreaksCore/BoardFactory.cpp
	Breaknes/BreaksCore/BogusBoard.cpp
	Breaknes/BreaksCore/BogusBoardDebug.cpp
	Breaknes/BreaksCore/CoreApi.cpp
	Breaknes/BreaksCore/DebugHub.cpp
	Breaknes/BreaksCore/FamicomBoard.cpp
	Breaknes/BreaksCore/NESBoard.cpp
	Breaknes/BreaksCore/NESBoardDebug.cpp
	Breaknes/BreaksCore/PPUPlayerBoard.cpp
	Breaknes/BreaksCore/PPUPlayerBoardDebug.cpp
	Breaknes/BreaksCore/SignalDefs.cpp
	Breaknes/BreaksCore/RegDumpEmitter.cpp
)

target_link_libraries (breaknes LINK_PUBLIC SDL2)
