.PHONY: clean All

All:
	@echo "----------Building project:[ mc - Debug_Win32 ]----------"
	@cd "mc" && "$(MAKE)" -f  "mc.mk"
clean:
	@echo "----------Cleaning project:[ mc - Debug_Win32 ]----------"
	@cd "mc" && "$(MAKE)" -f  "mc.mk" clean
