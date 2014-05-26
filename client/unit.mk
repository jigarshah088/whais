#Unit name.
UNIT:=client

UNIT_EXES:=wcmd
UNIT_SHLS:=
UNIT_LIBS:=wslconnector

wslconnector_SRC:=src/connector.c src/client_connection.c
wslconnector_DEF:=
wslconnector_LIB:=utils/wutils custom/wcustom
wslconnector_SHL:=

wcmd_SRC:=wcmd/wcmd.cpp wcmd/wcmd_optglbs.cpp wcmd/wcmd_cmdsmgr.cpp\
		 wcmd/wcmd_tabcomds.cpp wcmd/wcmd_onlinecmds.cpp wcmd/wcmd_execcmd.cpp\
		 wcmd/wcmd_valparser.cpp wcmd/wcmd_dbcheck.cpp
		 
wcmd_DEF:=USE_DBS_SHL
wcmd_LIB:=client/wslconnector utils/wslutils custom/wslcustom 
wcmd_SHL:=dbs/wpastra custom/wcommon
wcmd_INC:=


ifeq ($(BUILD_TESTS),yes)
wslconnector_SRC+=test/test_client_common.cpp
-include ./$(UNIT)/test/test.mk
endif

$(foreach exe, $(UNIT_EXES), $(eval $(call add_output_executable,$(exe),$(UNIT))))
$(foreach shl, $(UNIT_SHLS), $(eval $(call add_output_shared_lib,$(shl),$(UNIT),$($(shl)_MAJ),$($(shl)_MIN))))
$(foreach lib, $(UNIT_LIBS), $(eval $(call add_output_library,$(lib),$(UNIT))))

