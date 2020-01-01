.PHONY: printvars
printvars:
	$(foreach var,$(.VARIABLES),$(info $(var) = $($(var))))

