# Board configs by warits, Mon Nov 29, 2010
#
# $(MKCONFIG) $@ config_file descriptions
# $(MKCONFIG): Script to do configuratinos, this field should never be changed.
# $@: This field should never be changed.
# config_file: The coresponding configuration file, which is ported by developer.
# descriptions: The description string of board, which should be surround be quotes.
#
# So the squence to add a board is:
# Apply a board hardware ID from Raymond Wang
# Apply configuration file if needed(e.g. a totally new board)
# Add the information below.
#
# Steps to compile:
# make clean
# make mrproper
# make list
# make 1.0.0.1   #(e.g.)
#

# developing boards
1.0.0.1 : unconfig
	@$(MKCONFIG) $@ imapx820 "iMAPx820" 

