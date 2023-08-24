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

	BreaksAPU/APUSim/apu.cpp
	BreaksAPU/APUSim/clkgen.cpp
	BreaksAPU/APUSim/common.cpp
	BreaksAPU/APUSim/core.cpp
	BreaksAPU/APUSim/dac.cpp
	BreaksAPU/APUSim/debug.cpp
	BreaksAPU/APUSim/dma.cpp
	BreaksAPU/APUSim/dpcm.cpp
	BreaksAPU/APUSim/env.cpp
	BreaksAPU/APUSim/length.cpp
	BreaksAPU/APUSim/noise.cpp
	BreaksAPU/APUSim/pads.cpp
	BreaksAPU/APUSim/regs.cpp
	BreaksAPU/APUSim/square.cpp
	BreaksAPU/APUSim/triangle.cpp

	BreaksPPU/PPUSim/bgcol.cpp
	BreaksPPU/PPUSim/cram.cpp
	BreaksPPU/PPUSim/dataread.cpp
	BreaksPPU/PPUSim/debug.cpp
	BreaksPPU/PPUSim/fifo.cpp
	BreaksPPU/PPUSim/fsm.cpp
	BreaksPPU/PPUSim/hv.cpp
	BreaksPPU/PPUSim/hv_decoder.cpp
	BreaksPPU/PPUSim/mux.cpp
	BreaksPPU/PPUSim/oam.cpp
	BreaksPPU/PPUSim/par.cpp
	BreaksPPU/PPUSim/patgen.cpp
	BreaksPPU/PPUSim/pclk.cpp
	BreaksPPU/PPUSim/ppu.cpp
	BreaksPPU/PPUSim/regs.cpp
	BreaksPPU/PPUSim/scroll_regs.cpp
	BreaksPPU/PPUSim/sprite_eval.cpp
	BreaksPPU/PPUSim/video_out.cpp
	BreaksPPU/PPUSim/vram_ctrl.cpp

	Breaks6502/M6502Core/address_bus.cpp
	Breaks6502/M6502Core/alu.cpp
	Breaks6502/M6502Core/alu_control.cpp
	Breaks6502/M6502Core/branch_logic.cpp
	Breaks6502/M6502Core/bus_control.cpp
	Breaks6502/M6502Core/core.cpp
	Breaks6502/M6502Core/data_bus.cpp
	Breaks6502/M6502Core/debug.cpp
	Breaks6502/M6502Core/decoder.cpp
	Breaks6502/M6502Core/dispatch.cpp
	Breaks6502/M6502Core/extra_counter.cpp
	Breaks6502/M6502Core/flags.cpp
	Breaks6502/M6502Core/flags_control.cpp
	Breaks6502/M6502Core/interrupts.cpp
	Breaks6502/M6502Core/ir.cpp
	Breaks6502/M6502Core/pc.cpp
	Breaks6502/M6502Core/pc_control.cpp
	Breaks6502/M6502Core/predecode.cpp
	Breaks6502/M6502Core/random_logic.cpp
	Breaks6502/M6502Core/regs.cpp
	Breaks6502/M6502Core/regs_control.cpp

	Mappers/AbstractCartridge.cpp
	Mappers/CartridgeFactory.cpp
	Mappers/AOROM.cpp
	Mappers/NROM.cpp
	Mappers/UNROM.cpp
	Mappers/MMC1.cpp
	Mappers/MMC1_Based.cpp

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
